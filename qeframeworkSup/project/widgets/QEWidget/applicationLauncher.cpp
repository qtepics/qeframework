/*  applicationLauncher.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The EPICS QT Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with the EPICS QT Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2012 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
 This class manages applications launched from the QEImage widget to present a
 snapshot of the current image in another application.
 */

#include <iostream>
#include <QTemporaryFile>
#include <QMessageBox>
#include "applicationLauncher.h"

#define FILE_KEYWORD "<FILENAME>"


//==============================================================================
// processManager
//==============================================================================
//
processManager::processManager( bool logOutput, bool useStandardIo, QTemporaryFile* tempFileIn )
{
    tempFile = tempFileIn;

    // Catch when the process can be deleted
    QObject::connect( this, SIGNAL( finished(int, QProcess::ExitStatus) ),
                      this, SLOT( doFinished(int, QProcess::ExitStatus) ) );

    // Catch output if required.
    // Note: we do not expect both logOutput and useStandardIo to be true
    if( logOutput )
    {
        QObject::connect( this, SIGNAL( readyReadStandardOutput() ),
                          this, SLOT( doRead() ) );
        QObject::connect( this, SIGNAL( readyReadStandardError() ),
                          this, SLOT( doRead() ) );
    }

    if( useStandardIo )
    {
        QObject::connect( this, SIGNAL( readyReadStandardOutput() ),
                          this, SLOT(    doReadToStandardOutput() ) );
        QObject::connect( this, SIGNAL( readyReadStandardError() ),
                          this, SLOT(    doReadToStandardError() ) );
    }

}

processManager::~processManager()
{
    // qDebug() << "processManager destructor called";
    if( tempFile )
    {
        delete tempFile;
    }
}

void processManager::doRead()
{
    message.sendMessage( readAll() );
}

void processManager::doReadToStandardOutput()
{
    std::cout << readAllStandardOutput().data();
}

void processManager::doReadToStandardError()
{
    std::cerr << readAllStandardError().data();
}

void processManager::doFinished( int /*exitCode*/, QProcess::ExitStatus /*exitStatus*/ )
{
    emit processCompleted();
    deleteLater();
}


//==============================================================================
// applicationLauncher
//==============================================================================
//
applicationLauncher::applicationLauncher()
{
    programStartupOption = PSO_NONE;
}

// Desctuction
applicationLauncher::~applicationLauncher()
{
}

void applicationLauncher::launchImage( VariableNameManager* variableNameManager, QImage image )
{
    // Do nothing if no program to run
    if( program.isEmpty() )
    {
        return;
    }

    // Create a temporary file containing the image
    QTemporaryFile* tempFile = new QTemporaryFile;
    tempFile->open();
    if( !image.save( tempFile, "TIFF") )
    {
        QMessageBox msgBox;
        msgBox.setText("Can't start application. There is no image available.");
        msgBox.exec();
        return;
    }

    // Launch the program
    launchCommon( variableNameManager, tempFile );
}

void applicationLauncher::launch( VariableNameManager* variableNameManager, QObject* receiver )
{
    launchCommon( variableNameManager, NULL, receiver );
}

void applicationLauncher::launchCommon( VariableNameManager* variableNameManager, QTemporaryFile* tempFile, QObject* receiver )
{
    // Do nothing if no program to run
    if( program.isEmpty() )
    {
        return;
    }

    // Create a new process to run the program
    // (It will be up to the processManager to delete the temporary file if present)
    processManager* process = new processManager( programStartupOption == PSO_LOGOUTPUT,
                                                  programStartupOption == PSO_STDOUTPUT,
                                                  tempFile );

    // Connect to caller if a recipient has been provided
    if( receiver )
    {
        QObject::connect( process, SIGNAL( processCompleted() ), receiver, SLOT( programCompletedSlot() ) );
    }

    QStringList substitutedArguments;
    QString substitutedProgram;

    // Apply substitutions if available
    if( variableNameManager )
    {
        // Apply substitutions to the arguments
        substitutedArguments = arguments;
        for( int i = 0; i < substitutedArguments.size(); i++ )
        {
            substitutedArguments[i] = variableNameManager->substituteThis( substitutedArguments[i] );
        }

        // Apply substitutions to the program name
        substitutedProgram = variableNameManager->substituteThis( program );
    }

    // Use without substitutions
    else
    {
        substitutedArguments = arguments;
        substitutedProgram = program;
    }

    // Build up a single string with the command and arguments and run the program
    bool foundFileKeyword = false;
    for( int i = 0; i < substitutedArguments.size(); i++)
    {
        // Get the next argument
        QString arg = substitutedArguments[i];

        // If the argument contains the filename keyword, and a temorary filename is
        // available, insert the filename into the argument
        if( !foundFileKeyword && tempFile && !tempFile->fileName().isEmpty() && arg.contains( FILE_KEYWORD ) )
        {
            arg.replace( FILE_KEYWORD, tempFile->fileName() );
            foundFileKeyword = true;
        }

        // Add the argument to the command line
        substitutedProgram.append( " " );
        substitutedProgram.append( arg );
    }

    // If the filename of the temporary file needs to be added, and has not been added
    // already (to replace a filename keyword in an existing argument), then add the
    // filename as a new argument
    if( !foundFileKeyword && tempFile && !tempFile->fileName().isEmpty() )
    {
        substitutedProgram.append( " " );
        substitutedProgram.append( tempFile->fileName() );
    }

    // Add apropriate terminal command if starting up within a terminal
    if( programStartupOption == PSO_TERMINAL )
    {
#ifdef WIN32
        substitutedProgram.prepend( "cmd.exe /C start " );
#else
        substitutedProgram.prepend( "xterm -hold -e " );// use $TERM ??
#endif
    }

    // Run the program
    message.sendMessage( QString( "Launching: " ).append( substitutedProgram ), "Application launcher" );
    process->start( substitutedProgram );

    // Alternate (and cleaner) way to run the program without building a string containing the program and arguments.
    // (This didn't seem to work when starting EDM with the '-one' switch, perhaps due to the
    //  way EDM checks all arguments are identical when the '-one' switch is present?)
    //process->start( substituteThis( program ), substitutedArguments );
}

// end
