/*  applicationLauncher.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2019 Australian Synchrotron
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

#ifndef QE_APPLICATION_LAUNCHER_H
#define QE_APPLICATION_LAUNCHER_H

#include <QImage>
#include <QString>
#include <QProcess>
#include <QTemporaryFile>
#include <VariableNameManager.h>
#include <UserMessage.h>
#include <QEFrameworkLibraryGlobal.h>


// Class to manage a process started by a QE button/image
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT processManager : public QProcess
{
    Q_OBJECT

public:
    processManager( bool logOutput, bool useStandardIo, QTemporaryFile* tempFileIn );
    virtual ~processManager();

signals:
    void processCompleted();

public slots:
    void doRead();
    void doReadToStandardOutput();
    void doReadToStandardError();
    void doFinished( int /*exitCode*/, QProcess::ExitStatus /*exitStatus*/ );

private:
    UserMessage message;
    QTemporaryFile* tempFile;
    QObject* receiver;
};


class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT applicationLauncher
{
public:
    explicit applicationLauncher();
    ~applicationLauncher();

    void launchImage( const VariableNameManager* variableNameManager, QImage image );
    void launch( const VariableNameManager* variableNameManager, QObject* receiver );
    void launchCommon( const VariableNameManager* variableNameManager,
                       QTemporaryFile* tempFile = NULL,
                       QObject* receiver = NULL );

    // Program String
    void setProgram( const QString programIn );
    QString getProgram() const;

    // Arguments String
    void setArguments( const QStringList argumentsIn );
    QStringList getArguments() const;

    // Startup option
    enum programStartupOptions { PSO_NONE,         // Just run the program
                                 PSO_TERMINAL,     // Run the program in a termainal
                                 PSO_LOGOUTPUT,    // Run the program, and log the output in the QE message system
                                 PSO_STDOUTPUT };  // Run the program, and send output to standard output and standard error


    void setProgramStartupOption( const programStartupOptions programStartupOptionIn );
    programStartupOptions getProgramStartupOption() const;

private:
    QString program;        // Program to run
    QStringList arguments;  // Program arguments
    programStartupOptions programStartupOption; // Startup option (in a terminal, log output, or just start it and forget it)
    UserMessage message;
};

#endif // QE_APPLICATION_LAUNCHER_H
