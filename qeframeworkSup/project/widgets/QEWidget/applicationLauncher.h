/*  applicationLauncher.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2012-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
#include <QEEnums.h>
#include <QEFrameworkLibraryGlobal.h>


// Class to manage a process started by a QE button/image
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT processManager : public QProcess
{
   Q_OBJECT

public:
   explicit processManager( bool logOutput, bool useStandardIo, QTemporaryFile* tempFileIn );
   virtual ~processManager();

signals:
   void processCompleted();

public slots:
   void doRead();
   void doReadToStandardOutput();
   void doReadToStandardError();
   void doFinished( int exitCode, QProcess::ExitStatus exitStatus );

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

   void setProgramStartupOption( const QE::ProgramStartupOptions programStartupOptionIn );
   QE::ProgramStartupOptions getProgramStartupOption() const;

private:
   QString program;        // Program to run
   QStringList arguments;  // Program arguments
   QE::ProgramStartupOptions programStartupOption; // Startup option (in a terminal, log output,
                                                   // or just start it and forget it)
   UserMessage message;
};

#endif // QE_APPLICATION_LAUNCHER_H
