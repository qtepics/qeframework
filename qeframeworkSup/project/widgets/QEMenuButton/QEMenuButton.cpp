/*  QEMenuButton.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2015-2022 Australian Synchrotron
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
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include "QEMenuButton.h"
#include <QDebug>
#include <QECommon.h>
#include <QEStringFormatting.h>
#include <QEMenuButtonData.h>
#include <QEMenuButtonModel.h>

#define DEBUG  qDebug () << "QEMenuButton" << __LINE__ << __FUNCTION__ << "  "

#define ACTION_DATA  "QE_MENU_BUTTON_ACTION_DATA"

//------------------------------------------------------------------------------
//
QEMenuButton::QEMenuButton (QWidget* parent) : QEAbstractWidget (parent)
{
   // Create internal widget.
   //
   this->button = new QPushButton (this);

   // Copy actual widget size policy to the containing widget, then ensure
   // internal widget will expand to fill container widget.
   //
   this->setSizePolicy (this->button->sizePolicy ());
   this->button->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);

   this->layout = new QHBoxLayout (this);
   this->layout->setMargin (0);    // extact fit.
   this->layout->addWidget (this->button);

   // Set default property values
   //
   this->setAllowDrop (false);
   this->setVariableAsToolTip (false);
   this->setDisplayAlarmStateOption (DISPLAY_ALARM_STATE_NEVER);

   this->setButtonTextProperty ("MenuButton");
   this->button->setFont (this->font ());

   // Null menu specification.
   //
   this->theMenuString = QEMenuButtonModel::emptyXml ();

   // There are no variables per se, but we do use subsitutions.
   //
   // Set up the number of variables managed by the variable name manager
   // NOTE: there are no PVs associated with this widget, but it uses the same
   // substitution mechanism as other QE widgets.
   // This is used for the menu button text, menu items, PV names and PV values.
   //
   this->variableNameManagerInitialise (0);

   this->buttonMainMenu = new QMenu (this);

   this->button->setMenu (this->buttonMainMenu);

   // The connection applies to all added the sub-menus and actions.
   //
   QObject::connect (this->buttonMainMenu, SIGNAL (triggered     (QAction *)),
                     this,                 SLOT   (menuTriggered (QAction *)));


   // If a profile is define by whatever contains the menu button, then use it.
   // Note: cribbed from QEGenericButton - keep in sync.
   //
   if (this->isProfileDefined ()) {
      // Setup a signal to launch a new gui
      // The signal will be used by whatever the button is in.
      //
      QObject* launcher = this->getGuiLaunchConsumer ();
      if (launcher) {
         QObject::connect (this,   SIGNAL (newGui        (const QEActionRequests&)),
                           launcher, SLOT (requestAction (const QEActionRequests&)));
      }
   } else {
      // A profile is not already defined, create one. This is the case if
      // this class is used by an application that does not set up a profile,
      // such as 'designer'.
      //
      // Set up the button's own gui form launcher.
      //
      QObject::connect( this, SIGNAL (newGui        (const QEActionRequests&)),
                        this, SLOT   (requestAction (const QEActionRequests&)));
   }

   // Some events must be applied to the internal widget
   //
   this->installEventFilter (this);
}

//------------------------------------------------------------------------------
//
QEMenuButton::~QEMenuButton ()
{
   // place holder
}

//------------------------------------------------------------------------------
//
QSize QEMenuButton::sizeHint () const
{
   return QSize (110, 25);
}

//------------------------------------------------------------------------------
//
bool QEMenuButton::eventFilter(QObject* watched, QEvent* event)
{
   const QEvent::Type type = event->type ();
   bool result = false;

   switch (type) {
      case QEvent::FontChange:
         if (watched == this) {
            // Font must be mapped to the internal button
            //
            if (this->button) {
               this->button->setFont (this->font ());
            }
         }
         break;

      default:
         result = false;
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QEMenuButton::menuTriggered (QAction* action)
{
   // Retrieve the specific menu button data.
   // This is a decalred meta data type, so can be held in/extraction from a variant.
   //
   QEMenuButtonData buttonData = action->data ().value<QEMenuButtonData> ();

   // If there is a command to run, run it, with substitutions applied to the
   // command and arguments. Note the applicationLauncher itself handles the
   // substitutions to both program name and the arguments.
   //
   if (!buttonData.programName.isEmpty ()) {

      applicationLauncher programLauncher;

      programLauncher.setProgram (buttonData.programName);
      programLauncher.setArguments (buttonData.programArguments);
      programLauncher.setProgramStartupOption (buttonData.programStartupOption);

      // Note the launch function expects (2nd) this to have a programCompletedSlot slot.
      //
      programLauncher.launch ((VariableNameManager*) this, this);
   }

   // If a new GUI is required, start it (cribbed from QEGenericButton - keep in sync)
   //
   if (!buttonData.uiFilename.isEmpty()) {

      // Publish/release and set up profile as necessary.
      //
      ProfilePublisher publisher (this, buttonData.prioritySubstitutions);

      // Start the GUI
      //
      QString substitutedUiFilename = this->substituteThis (buttonData.uiFilename);

      emit this->newGui (QEActionRequests (substitutedUiFilename,
                                           buttonData.customisationName,
                                           buttonData.creationOption));
   }

   // If variable defined, then write to it.
   //
   if (!buttonData.variable.isEmpty()) {

      QString pvName = this->substituteThis (buttonData.variable);
      qcaobject::QCaObject* qca = new qcaobject::QCaObject (pvName, this, 0);

      // Store the required action data as a dynamic qca property.
      //
      qca->setProperty (ACTION_DATA, action->data ());

      // We are writing, only need wait for a successful connection.
      // No subscribe/siggle shot read needed.
      //
      QObject::connect (qca,  SIGNAL (connectionChanged (QCaConnectionInfo&, const unsigned int&)),
                        this, SLOT   (connectionChanged (QCaConnectionInfo&, const unsigned int&)));

      // Need to explicity subscribe, singleShotRead or connectChannel
      // As we are write only, connectChannel will do.
      //
      qca->connectChannel ();
   }
}

//------------------------------------------------------------------------------
//
void QEMenuButton::writeToVariable (qcaobject::QCaObject* qca)
{
   if (!qca) return;

   QVariant actionData = qca->property (ACTION_DATA);
   QEMenuButtonData buttonData = actionData.value<QEMenuButtonData> ();

   QString variableValue = this->substituteThis (buttonData.variableValue);

   bool okay = false;
   QVariant pvData;

   switch (buttonData.format) {
      case QEStringFormatting::FORMAT_FLOATING:
         pvData = QVariant (double (variableValue.toDouble (&okay)));
         break;

      case QEStringFormatting::FORMAT_INTEGER:
         pvData = QVariant (int (variableValue.toInt (&okay)));
         break;

      case QEStringFormatting::FORMAT_UNSIGNEDINTEGER:
         pvData = QVariant (qlonglong (variableValue.toLongLong (&okay)));
         break;

      case QEStringFormatting::FORMAT_DEFAULT:
         /// TODO - fix this option, but go with string for now.

      case QEStringFormatting::FORMAT_STRING:
         pvData = QVariant (variableValue);
         okay = true;
         break;

      case QEStringFormatting::FORMAT_TIME:
      case QEStringFormatting::FORMAT_LOCAL_ENUMERATE:
         DEBUG << "unhandled format value" << buttonData.format;
         okay = false;
         break;

      case QEStringFormatting::FORMAT_NT_TABLE:
      case QEStringFormatting::FORMAT_NT_IMAGE:
      case QEStringFormatting::FORMAT_OPAQUE:
         DEBUG << "unexpected format value" << buttonData.format;
         okay = false;
         break;

      default:
         DEBUG << "erroneous format value" << buttonData.format;
         okay = false;
         break;
   }

   // Was the conversion successful.
   //
   if (okay) {
      qca->writeData (pvData);
   } else {
      DEBUG << "conversion of " << buttonData.variableValue <<  "to"
            << buttonData.format <<  " failed.";
   }

   // The object will be deleted when control returns to the event loop.
   // Or can we just delete it now??
   //
   qca->deleteLater();
}

//------------------------------------------------------------------------------
//
void QEMenuButton::connectionChanged (QCaConnectionInfo& connectionInfo, const unsigned int&)
{
   qcaobject::QCaObject* qca = dynamic_cast <qcaobject::QCaObject*> (QObject::sender());
   if (!qca) return;

   if (connectionInfo.isChannelConnected()) {
      this->writeToVariable (qca);
   }
}

//------------------------------------------------------------------------------
//
void QEMenuButton::programCompletedSlot ()
{
//   DEBUG;
}

//------------------------------------------------------------------------------
//
void QEMenuButton::requestAction (const QEActionRequests& request)
{
   // Use QEWidget to handle this.
   //
   this->startGui (request);
}

//------------------------------------------------------------------------------
//
void QEMenuButton::setSubstitutionsProperty (const QString& substitutions)
{
   // Set the substitutions
   //
   this->setVariableNameSubstitutions (substitutions);

   // Update the button
   if (this->button) {
      this->button->setText (this->substituteThis (this->buttonText));
   }
}

//------------------------------------------------------------------------------
//
QString QEMenuButton::getSubstitutionsProperty () const
{
   return this->getVariableNameSubstitutions ();
}

//------------------------------------------------------------------------------
//
void QEMenuButton::setButtonTextProperty (const QString& buttonTextIn)
{
   this->buttonText = buttonTextIn;   // save ubn substituted button text
   if (this->button) {
      this->button->setText (this->substituteThis (this->buttonText));
   }
}

//------------------------------------------------------------------------------
//
QString QEMenuButton::getButtonTextProperty() const
{
   return this->buttonText;
}

//------------------------------------------------------------------------------
//
void QEMenuButton::setIcon (const QIcon& icon)
{
   if (this->button) {
      this->button->setIcon (icon);
   }
}

//------------------------------------------------------------------------------
//
QIcon QEMenuButton::getIcon () const
{
   return this->button->icon ();
}

//------------------------------------------------------------------------------
//
void QEMenuButton::setIconSize (const QSize& size)
{
   if (this->button) {
      this->button->setIconSize (size);
   }
}

//------------------------------------------------------------------------------
//
QSize QEMenuButton::getIconSize () const
{
   return this->button->iconSize ();
}

//------------------------------------------------------------------------------
//
void QEMenuButton::setMenuString (const QString& menuStringIn)
{
   if (menuStringIn.trimmed().isEmpty()) {
      this->resetMenuString ();
   } else if (this->theMenuString != menuStringIn) {
      this->theMenuString = menuStringIn;
   }

   QEMenuButtonModel model (this);
   bool status = model.parseXml (this->getMenuString ());
   if (status) {
      model.constructMenu (this->buttonMainMenu);
   }
   // model deconstructs here
}

//------------------------------------------------------------------------------
//
QString QEMenuButton::getMenuString () const
{
   return this->theMenuString;
}

//------------------------------------------------------------------------------
//
void QEMenuButton::resetMenuString ()
{
   this->theMenuString = QEMenuButtonModel::emptyXml ();
}

// end
