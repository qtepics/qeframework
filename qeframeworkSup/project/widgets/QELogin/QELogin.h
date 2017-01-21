/*  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
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
 *    Ricardo Fernandes
 *  Contact details:
 *    ricardo.fernandes@synchrotron.org.au
 */

#ifndef QELOGIN_H
#define QELOGIN_H

#include <QFrame>
#include <QRadioButton>
#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QStack>

#include <QEWidget.h>

// Forward delarations
class QELogin;

// Class to manage login.
// Used in a dialog by the compact form of QELogin or directly by the non-compact QELogin

class loginWidget:public QFrame
{
    Q_OBJECT

    private:
        QELogin* owner;                             // QELogin widget using this login form

        QRadioButton* qRadioButtonUser;             // User level buttons
        QRadioButton* qRadioButtonScientist;
        QRadioButton* qRadioButtonEngineer;

        QLineEdit* qLineEditPassword;               // Password entry

    public:
        loginWidget(QELogin* ownerIn );             // Construction
        userLevelTypes::userLevels getUserType();   // Return the selected user type
        QString getPassword();                      // Return the entered password
        void    clearPassword();                    // Clear any entered password

    private slots:
        void radioButtonClicked();                  // A user has pressed a radion button
};

// ============================================================
//  QELoginDialog class used when QELogin is in the compact form to present a login interface

class QELoginDialog : public QDialog
{
    Q_OBJECT

    private:
        QELogin*    owner;                  // QELogin widget using this dialog

        loginWidget* loginForm;             // Component widgets and layout
        QPushButton* qPushButtonOk;
        QPushButton* qPushButtonCancel;

    public:
        QELoginDialog( QELogin* ownerIn );  // Construction

    private slots:
        void buttonOkClicked();             // OK clicked
        void buttonCancelClicked();         // Cancel clicked
};

// ============================================================
// QELogin class manages the current user type (USER, SCIENTIST, ENGINEER) for the QE framework and applications

class QEPLUGINLIBRARYSHARED_EXPORT QELogin:public QFrame, public QEWidget
{
    Q_OBJECT

    private:
        QPushButton* qPushButtonLogin;                     // Component widgets and layout
        QLabel*      qLabelUserType;
        loginWidget* loginForm;

        QDialog* parentDialog;                              // Dialog this widget is a part of (optional. If present, login will also send accept to dialog)

        QString userPassword;                               // User level password (local to this widget)
        QString scientistPassword;                          // Scientist level password (local to this widget)
        QString engineerPassword;                           // Engineer level password (local to this widget)

        bool compactStyle;                                  // True if displaying in compact mode (login info presented in a dialog
        bool statusOnly;                                    // True if displaying status only

        void setCurrentLevelText();                         // Set the user level information text
        void userLevelChanged( userLevelTypes::userLevels );// Virtual function implementation called when the user level changes

    public:
        QELogin(QWidget *pParent = 0);                      // Construction
        virtual ~QELogin(){}                                // Destruction

        bool login( userLevelTypes::userLevels level, QString password );

        QString getPriorityUserPassword();                  // Get the user password. (application wide if present, otherwise local to this widget)
        QString getPriorityScientistPassword();             // Get the scientist password. (application wide if present, otherwise local to this widget)
        QString getPriorityEngineerPassword();              // Get the engineer password. (application wide if present, otherwise local to this widget)

        void setUserPassword(QString pValue);               // User password property functions
        QString getUserPassword();

        void setScientistPassword(QString pValue);          // Scientist password property functions
        QString getScientistPassword();

        void setEngineerPassword(QString pValue);           // Engineer password property functions
        QString getEngineerPassword();

        void setCompactStyle(bool compactStyle );           // Compact style property functions
        bool getCompactStyle();

        void setStatusOnly( bool statusOnlyIn );            // Status only property function
        bool getStatusOnly();

        QString getUserTypeName( userLevelTypes::userLevels type ); // Get a string to name each user level type

        // Properties
        Q_PROPERTY( bool statusOnly READ getStatusOnly WRITE setStatusOnly )
        Q_PROPERTY( bool compactStyle READ getCompactStyle WRITE setCompactStyle )
        Q_PROPERTY(QString userPassword READ getUserPassword WRITE setUserPassword)
        Q_PROPERTY(QString scientistPassword READ getScientistPassword WRITE setScientistPassword)
        Q_PROPERTY(QString engineerPassword READ getEngineerPassword WRITE setEngineerPassword)

    private slots:
        void buttonLoginClicked();                          // Login clicked. Raise login dialog in compact mode, otherwise attempt login

    signals:
        void login();                                       // A successfull login has occured. (good for closing a dialog)
};

#endif // QELOGIN_H
