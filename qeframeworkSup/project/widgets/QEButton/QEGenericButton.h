/*  QEGenericButton.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2024 Australian Synchrotron
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

#ifndef QE_GENERIC_BUTTON_H
#define QE_GENERIC_BUTTON_H

#include <QAbstractButton>
#include <QEEnums.h>
#include <QEWidget.h>
#include <QEForm.h>
#include <QEString.h>
#include <QEInteger.h>
#include <QEStringFormatting.h>
#include <managePixmaps.h>
#include <QESingleVariableMethods.h>
#include <QEStringFormattingMethods.h>
#include <QEIntegerFormatting.h>
#include <applicationLauncher.h>

/// Class common to all QE buttons
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEGenericButton :
        public QEWidget,
        public managePixmaps,
        public QESingleVariableMethods,
        public QEStringFormattingMethods
{
public:
    explicit QEGenericButton( QAbstractButton *owner );
    virtual ~QEGenericButton(){}

    enum VariableAllocation {
       VAR_PRIMARY = 0,     // Primary Control PV
       VAR_READBACK,        // Alternative readback PV - QEPushButton only
       NUMBER_OF_VARIABLES  // Maximum number of variables - must be last.
    };

    // subscribe
    void setSubscribe( bool subscribe );
    bool getSubscribe();

    // Properties

    // Update option (icon, text, or both)
    void setUpdateOption( QE::UpdateOptions updateOptionIn );
    QE::UpdateOptions getUpdateOption();

    // text alignment
    void setTextAlignment( Qt::Alignment alignment );
    Qt::Alignment getTextAlignment();

    // password
    void setPassword( QString password );
    QString getPassword();

    // confirm
    void setConfirmAction( bool confirmRequiredIn );
    bool getConfirmAction();

    // confirm text
    void setConfirmText( QString confirmTextIn );
    QString getConfirmText();

    // write on press
    void setWriteOnPress( bool writeOnPress );
    bool getWriteOnPress();

    // write on release
    void setWriteOnRelease( bool writeOnRelease );
    bool getWriteOnRelease();

    // write on click
    void setWriteOnClick( bool writeOnClick );
    bool getWriteOnClick();


    // press value
    void setPressText( QString pressText );
    QString getPressText();

    // release value
    void setReleaseText( QString releaseTextIn );
    QString getReleaseText();

    // click value
    void setClickText( QString clickTextIn );
    QString getClickText();

    // click checked value
    void setClickCheckedText( QString clickCheckedTextIn );
    QString getClickCheckedText();

    // 'Command button' Property convenience functions

    // Program String
    void setProgram( QString program );
    QString getProgram();

    // Arguments String
    void setArguments( QStringList arguments );
    QStringList getArguments();

    // Qt Designer Properties program startup options
    void setProgramStartupOption( QE::ProgramStartupOptions programStartupOptionIn );
    QE::ProgramStartupOptions getProgramStartupOption();

    // Property convenience functions

    // GUI name
    void setGuiName( QString guiName );
    QString getGuiName();

    // Priority macro substitutions
    void setPrioritySubstitutions( QString prioritySubstitutionsIn );
    QString getPrioritySubstitutions();

    // Window customisation name
    void setCustomisationName( QString customisationNameIn );
    QString getCustomisationName();

    // Qt Designer Properties Creation options
    void setCreationOption( QE::CreationOptions creationOption );
    QE::CreationOptions getCreationOption();

    // Label text (prior to substitution)
    void setLabelTextProperty( QString labelTextIn );
    QString getLabelTextProperty();

    // disabledRecordPolicy
    void setDisabledRecordPolicy( const QE::DisabledRecordPolicy disabledRecordPolicy );
    QE::DisabledRecordPolicy getDisabledRecordPolicy() const;

    // Write the click value now irrespective of whether writeOnClick property true or false.
    // This allows programatic "clicks" even when property is false.
    //
    void writeClickedNow (const bool checked = false);

protected:
    void useGenericNewVariableName( const QString& variableName, const QString& variableNameSubstitutions, const unsigned int variableIndex );
    void connectionChanged( QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex );
    void setGenericButtonText( const QString& text, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& variableIndex );
    void userPressed();
    void userReleased();
    void userClicked( bool checked );

    // write the click/clockChecked/press/release text value (of the associated button object) into the PV immediately.
    // performs no other action such as run command, open ui file.
    void processWriteNow( const bool checked );

    virtual QE::UpdateOptions getDefaultUpdateOption() = 0;

    applicationLauncher programLauncher;

private:
    Qt::Alignment textAlignment;

    QString password; // Text to be entered in a dialog before any action

    bool writeOnPress;
    bool writeOnRelease;
    bool writeOnClick;
    bool confirmRequired;     // Request confirmation before acting on a button event
    QE::DisabledRecordPolicy disabledRecordPolicy;    //
    QString confirmText;      // Text presented when confirming action
    QString releaseText;      // Text to write on a button release
    QString pressText;        // Text to write on a button press
    QString clickText;        // Text to write on a button click
    QString clickCheckedText; // Text to write on a button click when the button moves to the checked position

//    QString program;        // Program to run
//    QStringList arguments;  // Program arguments
//    programStartupOptions programStartupOption; // Startup option (in a terminal, log output, or just start it and forget it)

    QString guiName;      // GUI file name to launch
    QE::CreationOptions creationOption;  // Indicate how the new gui should be created ( examples: in a new window, or a top dock)
    QString prioritySubstitutions;  // Macro substitutions that take precedence over existing substitutions when creating new guis
    QString customisationName;      // Name of set of Window customisations such as additional menu items or tool bar push buttons

    bool localEnabled;

    QE::UpdateOptions updateOption;

    QString labelText;                                                 // Fixed label text to which substitutions will be applied

    bool getIsConnected(){ return isConnected; }
    bool confirmAction();

    void processRecordDisableState();

private:
    // Drag and Drop (See specific QE button widgets for button type specific drag and drop)
    // Use default getDrop/setDrop

    // Copy paste (See specific QE button widgets for button type specific copy and paste)
    QString copyVariable();
    virtual QVariant copyData() = 0;
    void paste( QVariant s );

protected:
    void setup();
    void establishConnection( unsigned int variableIndex );
    void calcStyleOption();                                             // Calculate style based on the widget usage and set a dynamic propert for style options.

    QESingleVariableMethods* altReadback;

private:
    void dataSetup();
//    void commandSetup();
    void guiSetup();
    qcaobject::QCaObject* createQcaItem( unsigned int variableIndex  );

    bool isConnected;
    QEIntegerFormatting integerFormatting;

    // !! Any of these that are accessing the QWidget don't have to call back up to the specific push button
    virtual void setButtonState( bool checked ) = 0;
    virtual void setButtonText( QString text ) = 0;
    virtual QString getButtonText() = 0;
    virtual void setButtonIcon( QIcon& icon ) = 0;

    virtual void emitNewGui( const QEActionRequests& request ) = 0;

    virtual void connectButtonDataChange( qcaobject::QCaObject* qca );

    virtual QAbstractButton* getButtonQObject() = 0;
    bool checkPassword();

    virtual void emitPressed( int pressValue ) = 0;
    virtual void emitReleased( int releaseValue ) = 0;
    virtual void emitClicked( int clickValue ) = 0;
};

#endif // QE_GENERIC_BUTTON_H
