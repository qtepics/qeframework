/*  QEForm.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2022 Australian Synchrotron
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

#ifndef QE_FORM_H
#define QE_FORM_H

#include <QScrollArea>
#include <QEAbstractWidget.h>
#include <QWidget>
#include <QString>
#include <QLabel>
#include <QSize>
#include <QEFileMonitor.h>
#include <QEFrameworkLibraryGlobal.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEActionRequests.h>
#include <QEFormMapper.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEForm :
      public QEAbstractWidget,
      public QEMapable
{
   Q_OBJECT
public:

   explicit QEForm( QWidget* parent = 0 );
   explicit QEForm( const QString& uifileNameIn, QWidget* parent = 0 );
   virtual ~QEForm();
   QSize sizeHint () const;

   void    setQEGuiTitle( const QString titleIn ); /// Set the title to be used as the window or form title. (note, also set when reading a .ui file)
   QString getQEGuiTitle() const;                  /// Get the title to be used as the window or form title.
   QString getFullFileName() const;                /// Get the standard, absolute UI file name
   QString getUiFileName() const;                  /// Get the fully substituted file name (Not the uiFile property)

   void setFileMonitoringIsEnabled( bool fileMonitoringIsEnabled ); /// Set flag indicating if form should take account of file monitoring.
   bool getFileMonitoringIsEnabled() const;                         /// Get flag indicating if form should take account of file monitoring.

   // Property convenience functions

   void setHandleGuiLaunchRequests( bool handleGuiLaunchRequests ); /// Set flag indicating form should handle gui form launch requests
   bool getHandleGuiLaunchRequests() const;                         /// Get flag indicating form should handle gui form launch requests

   void setResizeContents( bool resizeContentsIn );                 /// Set flag indicating form should resize contents to match form size (otherwise resize form to match contents)
   bool getResizeContents() const;                                  /// Get flag indicating form should resize contents to match form size (otherwise resize form to match contents)

   QString getContainedFrameworkVersion() const;                    /// Get the version of the first QE widget (if any) of QE widgets by QUILoader
   QString getUniqueIdentifier() const;                             /// Get a unique identifier string for this form. This identifier should be persistant across application runs as it is based on the QEForm's position in the widget hierarchy. The same widget will generate the same identifier when opened within the same GUI.
   void setUniqueIdentifier( QString name );                        /// Set a unique identifier string for this form. This identifier should be persistant across application runs as it is based on the QEForm's position in the widget hierarchy. The same widget will generate the same identifier when opened within the same GUI.

   void clearUiFileNames ();   // clears uiFileName/fullUiFileName

   int getDisconnectedCount() const;                                /// Return the count of disconnected variables
   int getConnectedCount() const;                                   /// Return the count of connected variables

   QWidget* getChild( QString name ) const;                         /// Find a widget within the ui loaded by the QEForm. Returns NULL if no UI is loaded yet or if the named widget can't be found.

public slots:
   bool readUiFile();                                               /// Read a .ui file and present it within this QEForm
   void requestAction( const QEActionRequests& request );           /// Slot for launching a new gui. Used by QE buttons and QEForm as the default action for launching a gui.

   // Slot function equivilents of setUiFileNameProperty and setVariableNameSubstitutionsProperty
   //
   void setUiFileName( const QString& uiFileName );
   void setUiFileSubstitutions( const QString& uiFileNameSubstitutions );

private slots:
   void fileChanged ( const QString & path );
   // Note, in QEForm, the standard variable name mechanism is used for the UI file name
   void useNewVariableNameProperty( QString variableNameIn,
                                    QString variableNameSubstitutionsIn,
                                    unsigned int variableIndex );
   void reloadLater();           // Slot for delaying form loading until after existing events have been processed

protected:
   void establishConnection( unsigned int variableIndex );

   QString uiFileName; // As specified on creation
   QString fullUiFileName; // Full standard path
   // no implementation - void setVariableNameSubstitutions( QString variableNameSubstitutionsIn );
   bool handleGuiLaunchRequests;
   bool resizeContents;

private:
   static void setEmbeddedFileMonitoringIsEnabled( QWidget* widget, bool fileMonitoringIsEnabled );

   void commonInit( const bool alertIfUINoFoundIn, const bool loadManuallyIn );

   QString title;                // GUI title (to be used by an application when presenting the GUI, such as in the title bar)
   QWidget* ui;
   bool alertIfUINoFound;        // True if the UI file could not be read. No alert is required, for example, when a partial UI file name is being typed in Designer
   bool fileMonitoringIsEnabled; // Only when true  does form honor any fileChanged signals from fileMon.
   QEFileMonitor fileMon;
   QString savedCurrentPath;

   void newMessage( QString msg, message_types type );
   void resizeEvent ( QResizeEvent * event );
   unsigned int childMessageFormId;

   QString containedFrameworkVersion;
   int* disconnectedCountRef;              // Pointer into plugin library (loaded by UI loader) to disconnection count
   int* connectedCountRef;                 // Pointer into plugin library (loaded by UI loader) to connection count


   void saveConfiguration( PersistanceManager* pm );
   void restoreConfiguration( PersistanceManager* pm, restorePhases restorePhase );
   void reloadFile();
   void resetCurrentPath();                // Reset the current directory.

   void setupWindowTitle( QString filename );    // Setup the title used to the name of the top level widget title, if it has one, or to the file name

   // Should be set up by an application using the persistance manager and
   // creating more than one top level QEForm. Not required if only one
   // QEForm is used.
   //
   QString uniqueIdentifier;


   void paintEvent(QPaintEvent *event);                        // When displaying a blank form, the form area is marked out on every update
   QLabel* placeholderLabel;                                   // Used to display a message when the QEForm could not be loaded with a .ui file
   void displayPlaceholder( const QString& message );          // Display a message when the QEForm could not be loaded with a .ui file
   void clearPlaceholder();                                    // Clear a message when the QEForm could not be loaded with a .ui file

   bool loadManually;                                          // Set true when QEForm will be manually loaded by calling QEForm::readUiFile()

signals:
   void formLoaded( bool fileLoaded );                         // The form has finished loading a .ui file. fileLoaded is true if reading the .ui file was successfull. This signal is required since the loading completes in an event.

public:
   // Note, a property macro in the form 'Q_PROPERTY(QString uiFileName READ ...' doesn't work.
   // A property name ending with 'Name' results in some sort of string a variable being
   // displayed, but will only accept alphanumeric and won't generate callbacks on change.
   /// File name of the .ui file being presented within the QEForm widget.
   ///
   Q_PROPERTY(QString uiFile READ getUiFileNameProperty WRITE setUiFileNameProperty)

   // Note, standard variable name and macros mechanism is used by QEForm for
   // UI file name and marcos
   void    setUiFileNameProperty( QString uiFileName );
   QString getUiFileNameProperty() const;

   /// Macro substitutions to be applied to this widget, and all QE widgets that are opened when the .ui file is presented.
   /// Note, despite the name, the macro substitutions are general macro substitutions, and do not just apply to a variable name (in fact a
   /// QEForm widget does not even have a variable name property.
   Q_PROPERTY(QString variableSubstitutions READ getVariableNameSubstitutionsProperty
                                            WRITE setVariableNameSubstitutionsProperty)

   void    setVariableNameSubstitutionsProperty( QString variableNameSubstitutions );
   QString getVariableNameSubstitutionsProperty() const;

   /// If true, the QEForm widget publishes its own slot for launching new GUIs so all QE widgets within it will use
   /// the QEForm's mechanism for launching new GUIs, rather than any mechanism the application may provide (through the ContainerProfile mechanism)
   Q_PROPERTY(bool handleGuiLaunchRequests READ getHandleGuiLaunchRequests WRITE setHandleGuiLaunchRequests)

   /// If set, the QEForm will resize the top level widget of the .ui file it opens (and set other size and border
   /// related properties) to match itself. This is useful if the QEForm is used as a sub form within a main form
   /// (possible another QEForm) and you want to control the size of the QEForm being used as a sub form.
   /// If clear, the QEForm will resize itself (and set other size and border related properties) to match the top
   /// level widget of the .ui file it opens. This is useful if the QEForm is used as a sub form within a main form
   /// (possible another QEForm) and you want to the main form to resize to match the size of the QEForm being used
   /// as a sub form, or you want the sub form border decorations (such as frame shape and shadow) to be displayed.
   Q_PROPERTY(bool resizeContents READ getResizeContents WRITE setResizeContents)

   /// Widgets or applications that use messages from the framework have the option of filtering on this ID
   /// Messages that the QEForm widget catches with its message filters will be regenerated using this ID
   ///
   Q_PROPERTY(unsigned int messageSourceId READ getMessageSourceId WRITE setMessageSourceId )

   Q_ENUMS(MessageFilterOptions)

   /// Message filter that attempts to match messages sent through the QE message logging system based on
   /// the automatically generated message form ID.
   /// This filter will match form ID of the message to the form ID of this QEform as follows:
   ///
   /// Any - A message will always be accepted.
   /// Match - A message will be accepted if it comes from a QE widget within this form.
   /// None	- The message will not be matched based on the form the message comes from. (It may still be accepted based on the message source ID.)
   /// Matched messages will be resend with the messageSourceId of this QEForm
   Q_PROPERTY(MessageFilterOptions messageFormFilter READ getMessageFormFilter
                                                     WRITE setMessageFormFilter )

   //!!!??? is this a valid property. Resending messages based on the source ID is unnessesary as they will be sent on with the same source ID?
   /// Message filter that attempts to match messages sent through the QE message logging system based on
   /// the messageSourceId of the widget that generatedd the messge.
   /// This filter will match message message source ID of the message to the message source ID of this QEform as follows:
   ///
   /// Any - A message will always be accepted.
   /// Match - A message will be accepted if the message source ID matches this QEForm.
   /// None	- The message will not be matched based of message source ID (It may still be accepted based on the message form ID.)
   /// Matched messages will be resend with the messageSourceId of this QEForm.
   Q_PROPERTY(MessageFilterOptions messageSourceFilter READ getMessageSourceFilter
                                                       WRITE setMessageSourceFilter )

   enum MessageFilterOptions {
      /* Any = UserMessage::MESSAGE_FILTER_ANY, This would allow QEForm widgets to get in a message resend loop */
      Match = UserMessage::MESSAGE_FILTER_MATCH,
      None  = UserMessage::MESSAGE_FILTER_NONE
   };

   MessageFilterOptions getMessageFormFilter() const;
   void setMessageFormFilter( MessageFilterOptions messageFormFilter );

   MessageFilterOptions getMessageSourceFilter() const;
   void setMessageSourceFilter( MessageFilterOptions messageSourceFilter );

   /// variableAsToolTip is added as a non-designable property here only to hide the implementation present in QEAbstractWidget
   ///
   Q_PROPERTY(bool variableAsToolTip READ getVariableAsToolTip WRITE setVariableAsToolTip DESIGNABLE false)
   /// allowDrop is added as a non-designable property here only to hide the implementation present in QEAbstractWidget
   ///
   Q_PROPERTY(bool allowDrop READ getAllowDrop WRITE setAllowDrop  DESIGNABLE false)
   /// displayAlarmStateOption is added as a non-designable property here only to hide the implementation present in QEAbstractWidget
   ///
   Q_PROPERTY(DisplayAlarmStateOptions displayAlarmStateOption READ getDisplayAlarmStateOptionProperty WRITE setDisplayAlarmStateOptionProperty DESIGNABLE false)

private:
   // Note, this is only used to manage the macro substitutions that will be
   // passed down to the form's QE widgets. The form has no variable name.
   //
   QCaVariableNamePropertyManager variableNamePropertyManager;
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEForm::MessageFilterOptions)
#endif

#endif // QE_FORM_H
