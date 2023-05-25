/*  applicationLauncher.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2023 Australian Synchrotron
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
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
 This class manages applications launched from the QEImage widget to present a
 snapshot of the current image in another application.
 */

#include "applicationLauncher.h"
#include <iostream>
#include <QDebug>
#include <QTemporaryFile>
#include <QMessageBox>

#define DEBUG qDebug () << "applicationLauncher" << __LINE__ << __FUNCTION__ << "  "

#define FILE_KEYWORD "<FILENAME>"


//==============================================================================
// processManager
//==============================================================================
//
processManager::processManager( bool logOutput, bool useStandardIo, QTemporaryFile* tempFileIn )
{
   this->tempFile = tempFileIn;

   // Catch when the process can be deleted
   //
   QObject::connect( this, SIGNAL( finished(int, QProcess::ExitStatus) ),
                     this, SLOT( doFinished(int, QProcess::ExitStatus) ) );

   // Catch output if required.
   // Note: we do not expect both logOutput and useStandardIo to be true
   //
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

//------------------------------------------------------------------------------
//
processManager::~processManager()
{
   // qDebug() << "processManager destructor called";
   if( this->tempFile ) {
      delete this->tempFile;
   }
}

//------------------------------------------------------------------------------
//
void processManager::doRead()
{
   this->message.sendMessage( readAll() );
}

//------------------------------------------------------------------------------
//
void processManager::doReadToStandardOutput()
{
   std::cout << this->readAllStandardOutput().data();
}

//------------------------------------------------------------------------------
//
void processManager::doReadToStandardError()
{
   std::cerr << this->readAllStandardError().data();
}

//------------------------------------------------------------------------------
//
void processManager::doFinished( int /*exitCode*/, QProcess::ExitStatus /*exitStatus*/ )
{
   emit processCompleted();
   this->deleteLater();
}


//==============================================================================
// applicationLauncher
//==============================================================================
//
applicationLauncher::applicationLauncher()
{
   this->programStartupOption = PSO_NONE;
}

//------------------------------------------------------------------------------
// Destruction
applicationLauncher::~applicationLauncher(){ }

//------------------------------------------------------------------------------
//
void applicationLauncher::launchImage( const VariableNameManager* variableNameManager, QImage image )
{
   // Do nothing if no program to run
   //
   if( this->program.isEmpty() ) {
      return;
   }

   // Create a temporary file containing the image
   //
   QTemporaryFile* tempFile = new QTemporaryFile;
   tempFile->open();
   if( !image.save( tempFile, "TIFF") ) {
      QMessageBox msgBox;
      msgBox.setText("Can't start application. There is no image available.");
      msgBox.exec();
      return;
   }

   // Launch the program
   //
   this->launchCommon( variableNameManager, tempFile );
}

//------------------------------------------------------------------------------
//
void applicationLauncher::launch( const VariableNameManager* variableNameManager, QObject* receiver )
{
   this->launchCommon( variableNameManager, NULL, receiver );
}

//------------------------------------------------------------------------------
//
void applicationLauncher::launchCommon( const VariableNameManager* variableNameManager,
                                        QTemporaryFile* tempFile, QObject* receiver )
{
   // Do nothing if no program to run
   //
   if( this->program.isEmpty() ) {
      return;
   }

   // Create a new process to run the program
   // (It will be up to the processManager to delete the temporary file if present)
   //
   processManager* process = new processManager( programStartupOption == PSO_LOGOUTPUT,
                                                 programStartupOption == PSO_STDOUTPUT,
                                                 tempFile );

   // Connect to caller if a recipient has been provided
   if( receiver ) {
      QObject::connect( process, SIGNAL( processCompleted() ),
                        receiver, SLOT( programCompletedSlot() ) );
   }

   QString substitutedProgram;
   QStringList substitutedArguments;

   // Apply substitutions if available
   //
   if( variableNameManager ) {
      // Apply substitutions to the arguments
      //
      substitutedArguments = this->arguments;
      for( int i = 0; i < substitutedArguments.size(); i++ ) {
         substitutedArguments[i] = variableNameManager->substituteThis( substitutedArguments[i] );
      }

      // Apply substitutions to the program name
      substitutedProgram = variableNameManager->substituteThis( this->program );
   } else {
      // Use without substitutions
      //
      substitutedArguments = this->arguments;
      substitutedProgram = this->program;
   }

   // Apply file name keyword substitution - once only.
   //
   bool foundFileKeyword = false;
   for( int i = 0; i < substitutedArguments.size(); i++) {

      // Get the next argument
      //
      QString arg = substitutedArguments[i];

      // If the argument contains the filename keyword, and a temorary filename is
      // available, insert the filename into the argument
      //
      if( !foundFileKeyword && tempFile && !tempFile->fileName().isEmpty() && arg.contains( FILE_KEYWORD ) )
      {
         arg.replace( FILE_KEYWORD, tempFile->fileName() );
         substitutedArguments[i] = arg;
         foundFileKeyword = true;
      }
   }

   // If the filename of the temporary file needs to be added, and has not been added
   // already (to replace a filename keyword in an existing argument), then add the
   // filename as a new argument
   //
   if( !foundFileKeyword && tempFile && !tempFile->fileName().isEmpty() )
   {
      substitutedArguments.append( tempFile->fileName() );
   }

   // Add apropriate terminal command if starting up within a terminal
   //
   if( this->programStartupOption == PSO_TERMINAL ) {
#ifdef WIN32
      substitutedArguments.prepend( substitutedProgram );
      substitutedArguments.prepend( "start" );
      substitutedArguments.prepend( "/C" );
      substitutedProgram = "cmd.exe";
#else
      substitutedArguments.prepend( substitutedProgram );
      substitutedArguments.prepend( "-e" );
      substitutedArguments.prepend( "-hold" );
      substitutedProgram = "xterm"; // use $TERM
#endif
   }

   // Run the program
   //
   this->message.sendMessage( QString( "Launching: " ).append( substitutedProgram ), "Application launcher" );
   process->start( substitutedProgram, substitutedArguments );
}


//------------------------------------------------------------------------------
//
void applicationLauncher::setProgram( const QString programIn )
{
   this->program = programIn;
}

//------------------------------------------------------------------------------
//
QString applicationLauncher::getProgram() const
{
   return this->program;
}

//------------------------------------------------------------------------------
//
void applicationLauncher::setArguments( const QStringList argumentsIn )
{
   this->arguments = argumentsIn;
}

//------------------------------------------------------------------------------
//
QStringList applicationLauncher::getArguments() const
{
   return this->arguments;
}

//------------------------------------------------------------------------------
//
void applicationLauncher::setProgramStartupOption( const programStartupOptions programStartupOptionIn )
{
   this->programStartupOption = programStartupOptionIn;
}

//------------------------------------------------------------------------------
//
applicationLauncher::programStartupOptions applicationLauncher::getProgramStartupOption() const
{
   return this->programStartupOption;
}

// end
