/*  QELogin.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2012-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Ricardo Fernandes,
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

/*
  This class manages the current user type (USER, SCIENTIST, ENGINEER) for
  the QE framework and applications. It uses passwords defined by the application,
  or if absent by its own properties. It is tighly integrated with the base class
  QEWidget. Refer to QEWidget.cpp for details
 */

#include <QGridLayout>
#include <QMessageBox>
#include <QGroupBox>
#include <QELogin.h>

// =============================================================================
//  QELogin class.

QELogin::QELogin(QWidget *pParent):QFrame(pParent), QEWidget( this )
{
   // Presentation of the frame
   setFrameStyle( QFrame::StyledPanel );
   setFrameShadow( QFrame::Raised );

   // Create the widgets that make up a QELogin widget
   qLabelUserType = new QLabel(this);
   loginForm = new loginWidget( this );
   qPushButtonLogin = new QPushButton(this);

   // Arrange the widget
   QGridLayout* qGridLayout = new QGridLayout(this);
   qGridLayout->addWidget( qLabelUserType, 0, 0, 1, 2 );
   qGridLayout->addWidget( loginForm, 1, 0, 1, 2 );
   qGridLayout->addWidget( qPushButtonLogin, 2, 0);

   // Assume compact style
   setCompactStyle( true );
   setStatusOnly( false );

   qLabelUserType->setToolTip("Current user");

   // Set up login button
   qPushButtonLogin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
   qPushButtonLogin->setText("Login");
   qPushButtonLogin->setToolTip("Change current user");
   QObject::connect(qPushButtonLogin, SIGNAL(clicked()), this, SLOT(buttonLoginClicked()));

   // Set initial state
   setCurrentLevelText();
}

//------------------------------------------------------------------------------
// Virtual function implementation called when the user level changes
void QELogin::userLevelChanged( QE::UserLevels )
{
   setCurrentLevelText();
}

//------------------------------------------------------------------------------
// Set up current level text
void QELogin::setCurrentLevelText()
{
   qLabelUserType->setText(QString( "Current Level: " ).append( getUserTypeName( getUserLevel() ) ));
}

//------------------------------------------------------------------------------
// Compact mode property fuctions
// Set if the widget is small and pops up a dialog to allow login,
// or larger and displays the login all the time
void QELogin::setCompactStyle(bool compactStyleIn )
{
   compactStyle = compactStyleIn;
   loginForm->setHidden( compactStyle || statusOnly );
}

//------------------------------------------------------------------------------
//
bool QELogin::getCompactStyle()
{
   return compactStyle;
}

//------------------------------------------------------------------------------
// Status only property fuctions
void QELogin::setStatusOnly( bool statusOnlyIn )
{
   statusOnly = statusOnlyIn;
   loginForm->setHidden( compactStyle || statusOnly );
   qPushButtonLogin->setHidden( statusOnly );
}

//------------------------------------------------------------------------------
//
bool QELogin::getStatusOnly()
{
   return statusOnly;
}

//------------------------------------------------------------------------------
// User password property fuctions
void QELogin::setUserPassword(QString pValue)
{
   userPassword = pValue;
}

//------------------------------------------------------------------------------
//
QString QELogin::getUserPassword()
{
   return userPassword;
}

//------------------------------------------------------------------------------
// Scientist password property fuctions
void QELogin::setScientistPassword(QString pValue)
{
   scientistPassword = pValue;
}

//------------------------------------------------------------------------------
//
QString QELogin::getScientistPassword()
{
   return scientistPassword;
}

//------------------------------------------------------------------------------
// Engineer password property fuctions
void QELogin::setEngineerPassword(QString pValue)
{
   engineerPassword = pValue;
}

//------------------------------------------------------------------------------
//
QString QELogin::getEngineerPassword()
{
   return engineerPassword;
}

//------------------------------------------------------------------------------
// Return a name for each user type
QString QELogin::getUserTypeName(QE::UserLevels type)
{
   switch( type )
   {
      case QE::User:      return "User";
      case QE::Scientist: return "Scientist";
      case QE::Engineer:  return "Engineer";
      default:            return "Unknown";
   }
}

//------------------------------------------------------------------------------
// The login button has been clicked
void QELogin::buttonLoginClicked()
{
   // For compact style, present the login dialog
   if( compactStyle )
   {
      QELoginDialog qELoginDialog( this );
      qELoginDialog.exec();
   }

   // For non compact style, try to login with the selected user type and password
   else
   {
      login( loginForm->getUserType(), loginForm->getPassword() );
      loginForm->clearPassword();
   }
}

//------------------------------------------------------------------------------
// Try to login with a selected user type and password.
// This is called by the QEWidget directly, or from the QEWidget's login dialog
bool QELogin::login( QE::UserLevels level, QString password )
{
   // Get the required password
   QString requiredPassword;
   switch( level )
   {
      case QE::User:      requiredPassword = getPriorityUserPassword();      break;
      case QE::Scientist: requiredPassword = getPriorityScientistPassword(); break;
      case QE::Engineer:  requiredPassword = getPriorityEngineerPassword();  break;
   }

   // Note the current user level
   QE::UserLevels currentLevel = getUserLevel();

   // If the password is not required, or if the password is OK, or
   // if we are lowering the user level, change the user type
   if( requiredPassword.isEmpty() || password == requiredPassword || currentLevel >= level )
   {
      // Change user level if required
      if( level != currentLevel )
      {
         sendMessage("The user type was changed from '" +
                     getUserTypeName( currentLevel ) +
                     "' to '" + getUserTypeName( level ) + "'");
         setUserLevel( level);
         setCurrentLevelText();
      }

      // Signal a successfull login has occured
      // This is usefull to for closing a dialog this widget may be a part of
      emit login();

      // Indicate successfull login
      return true;
   }

   // bad password, tell the user
   QMessageBox::critical(this, "Error", "The password is invalid. Please try again!");
   return false;
}

//------------------------------------------------------------------------------
// Return the user level password from the profile if available,
// otherwise use the local user level password property
QString QELogin::getPriorityUserPassword()
{
   ContainerProfile profile;
   if( profile.areUserLevelPasswordsSet() )
   {
      return profile.getUserLevelPassword( QE::User );
   }
   else
   {
      return userPassword;
   }
}

//------------------------------------------------------------------------------
// Return the scientist level password from the profile if available,
// otherwise use the local scientist level password property
QString QELogin::getPriorityScientistPassword()
{
   ContainerProfile profile;
   if( profile.areUserLevelPasswordsSet() )
   {
      return profile.getUserLevelPassword( QE::Scientist );
   }
   else
   {
      return scientistPassword;
   }
}

//------------------------------------------------------------------------------
// Return the engineer level password from the profile if available,
// otherwise use the local engineer level password property
QString QELogin::getPriorityEngineerPassword()
{
   ContainerProfile profile;
   if( profile.areUserLevelPasswordsSet() )
   {
      return profile.getUserLevelPassword( QE::Engineer );
   }
   else
   {
      return engineerPassword;
   }
}

// =============================================================================
// loginWidget
// This widget is used in the dialog launched by the compact form of QELogin,
// and by the larger form of QELogin directly.

loginWidget::loginWidget( QELogin* ownerIn )
{
   // Note the QELogin widget
   owner = ownerIn;

   // Create the widgets making up the login form
   QGroupBox*   qGroupBox = new QGroupBox(this);
   qRadioButtonUser = new QRadioButton();
   qRadioButtonScientist = new QRadioButton(this);
   qRadioButtonEngineer = new QRadioButton(this);
   qLineEditPassword = new QLineEdit(this);

   // Set up the widgets
   qGroupBox->setTitle( "Login as:");

   qRadioButtonUser->setText(owner->getUserTypeName(QE::User));
   QObject::connect(qRadioButtonUser, SIGNAL(clicked()), this, SLOT(radioButtonClicked()));

   qRadioButtonScientist->setText(owner->getUserTypeName(QE::Scientist));
   QObject::connect(qRadioButtonScientist, SIGNAL(clicked()), this, SLOT(radioButtonClicked()));

   qRadioButtonEngineer->setText(owner->getUserTypeName(QE::Engineer));
   QObject::connect(qRadioButtonEngineer, SIGNAL(clicked()), this, SLOT(radioButtonClicked()));

   qLineEditPassword->setEchoMode(QLineEdit::Password);
   qLineEditPassword->setToolTip("Password for the selected type");

   // Layout the form
   QVBoxLayout* qVBoxLayout = new QVBoxLayout();
   qVBoxLayout->addWidget(qRadioButtonUser);
   qVBoxLayout->addWidget(qRadioButtonScientist);
   qVBoxLayout->addWidget(qRadioButtonEngineer);
   qGroupBox->setLayout(qVBoxLayout);

   QGridLayout* qGridLayout = new QGridLayout(this);
   qGridLayout->addWidget(qGroupBox, 0, 0);
   qGridLayout->addWidget(qLineEditPassword, 1, 0);

   // Select a radio button to match the current user level
   switch( owner->getUserLevel() )
   {
      case QE::User:        qRadioButtonUser->setChecked(true);      break;
      case QE::Scientist:   qRadioButtonScientist->setChecked(true); break;
      case QE::Engineer:    qRadioButtonEngineer->setChecked(true);  break;
   }

   // Enable or disable the password according to the user type
   radioButtonClicked();
}

//------------------------------------------------------------------------------
// A user type has been selected.
// Enable or disable the password according to the user type
void loginWidget::radioButtonClicked()
{
   bool passwordIsEmpty;
   QE::UserLevels targetLevel;

   // Note if the password for the target level is empty (and what the target level is)
   if (qRadioButtonUser->isChecked())
   {
      passwordIsEmpty = owner->getPriorityUserPassword().isEmpty();
      targetLevel = QE::User;
   }
   else if (qRadioButtonScientist->isChecked())
   {
      passwordIsEmpty = owner->getPriorityScientistPassword().isEmpty();
      targetLevel = QE::Scientist;
   }
   else
   {
      passwordIsEmpty = owner->getPriorityEngineerPassword().isEmpty();
      targetLevel = QE::Engineer;
   }

   // Enable password entry if a password is required for the selected user type
   // A password is only requried if a password is specified and the level is increasing
   QE::UserLevels currentLevel = owner->getUserLevel();
   qLineEditPassword->setEnabled( targetLevel > currentLevel && !passwordIsEmpty );
}

//------------------------------------------------------------------------------
// Return the user type selected by the radio button group
QE::UserLevels loginWidget::getUserType()
{
   if( qRadioButtonUser->isChecked() )
      return QE::User;

   if( qRadioButtonScientist->isChecked() )
      return QE::Scientist;

   if( qRadioButtonEngineer->isChecked() )
      return QE::Engineer;

   // Default
   return QE::User;
}

//------------------------------------------------------------------------------
// Return the password entered by the user
QString loginWidget::getPassword()
{
   return qLineEditPassword->text();
}

// Clear the password entered by the user
void loginWidget::clearPassword()
{
   qLineEditPassword->clear();
}

// =============================================================================
// QELoginDialog widget
// This widget is used when the QELogin widget is in compact form to present the login options.

QELoginDialog::QELoginDialog(QELogin* ownerIn)
{
   // Remove maximise and minimise buttons
   // !!! doesn't work on linux???
   setWindowFlags( Qt::Dialog );

   // Note the QELogin widget
   owner = ownerIn;

   // Create the widgets making up the login dialog
   loginForm = new loginWidget(owner);
   qPushButtonOk = new QPushButton(this);
   qPushButtonCancel = new QPushButton(this);

   // Set up the widgets
   setWindowTitle("Login");

   qPushButtonOk->setText("Ok");
   qPushButtonOk->setToolTip("Perform login");
   QObject::connect(qPushButtonOk, SIGNAL(clicked()), this, SLOT(buttonOkClicked()));

   qPushButtonCancel->setText("Cancel");
   qPushButtonCancel->setToolTip("Cancel login");
   QObject::connect(qPushButtonCancel, SIGNAL(clicked()), this, SLOT(buttonCancelClicked()));

   // Layout the form
   QGridLayout* qGridLayout = new QGridLayout(this);
   qGridLayout->addWidget( loginForm, 0, 0, 1, 2 );
   qGridLayout->addWidget( qPushButtonCancel, 1, 0);
   qGridLayout->addWidget( qPushButtonOk, 1, 1);
}

//------------------------------------------------------------------------------
// Dialog OK clicked
void QELoginDialog::buttonOkClicked()
{
   // Attempt to login and close the dialog if successfull
   if( owner->login( loginForm->getUserType(), loginForm->getPassword() ) )
   {
      close();
   }
}

//------------------------------------------------------------------------------
// Dialog cancel clicked
void QELoginDialog::buttonCancelClicked()
{
   close();
}

// end
