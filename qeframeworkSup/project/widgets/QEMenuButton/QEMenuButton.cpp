/*  QEMenuButton.cpp
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
 *  Copyright (c) 2015, 2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QDebug>

#include <QECommon.h>
#include <QEStringFormatting.h>
#include <QEMenuButtonData.h>
#include <QEMenuButtonModel.h>
#include <QEMenuButton.h>

#define DEBUG  qDebug () << "QEMenuButton:" << __FUNCTION__ << __LINE__

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

   setLabelTextProperty(getLabelTextProperty());
   this->button->setFont(font());
   //this->button->setText ("MenuButton");

   // Null menu specification.
   //
   this->theMenuString = QEMenuButtonModel::emptyXml ();

   // There are no variables per se, but we do use subsitutions.
   //
   this->setNumVariables (0);

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
   return QSize (110, 27);
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
   // command and arguments
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
      qcaobject::QCaObject* qca = new qcaobject::QCaObject (buttonData.variable, this, 0);
      qca->setProperty (ACTION_DATA, action->data ());

      // We are writing, only need wait for a successful connection.
      // No subscribe/siggle shot read needed.
      //
      QObject::connect (qca,  SIGNAL (connectionChanged (QCaConnectionInfo&, const unsigned int&)),
                        this, SLOT   (connectionChanged (QCaConnectionInfo&, const unsigned int&)));
   }
}

//------------------------------------------------------------------------------
//
void QEMenuButton::writeToVariable (qcaobject::QCaObject* qca)
{
   if (!qca) return;

   QVariant actionData = qca->property (ACTION_DATA);
   QEMenuButtonData buttonData = actionData.value<QEMenuButtonData> ();

   bool okay = false;
   QVariant pvData;

   switch (buttonData.format) {
      case QEStringFormatting::FORMAT_FLOATING:
         pvData = QVariant (double (buttonData.variableValue.toDouble (&okay)));
         break;

      case QEStringFormatting::FORMAT_INTEGER:
         pvData = QVariant (int (buttonData.variableValue.toInt (&okay)));
         break;

      case QEStringFormatting::FORMAT_UNSIGNEDINTEGER:
         pvData = QVariant (qlonglong (buttonData.variableValue.toLongLong (&okay)));
         break;

      case QEStringFormatting::FORMAT_DEFAULT:
      // TODO - fix this option, but go with string for now.

      case QEStringFormatting::FORMAT_STRING:
         pvData = QVariant (buttonData.variableValue);
         okay = true;
         break;

      case QEStringFormatting::FORMAT_TIME:
      case QEStringFormatting::FORMAT_LOCAL_ENUMERATE:
      default:
         DEBUG << "unexpected format value" << buttonData.format;
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

   // TODO: initate timed delete of the qca object - or can we just delete it now??
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
   DEBUG;
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
   this->setVariableNameAndSubstitutions ("", substitutions, 0);
}

//------------------------------------------------------------------------------
//
QString QEMenuButton::getSubstitutionsProperty () const
{
   return this->getVariableNameSubstitutions ();
}

//------------------------------------------------------------------------------
//
void QEMenuButton::setLabelTextProperty( QString labelTextIn )
{
   this->button->setText (labelTextIn);
}

//------------------------------------------------------------------------------
//
QString QEMenuButton::getLabelTextProperty()
{
    return this->button->text();
}

//------------------------------------------------------------------------------
//
void QEMenuButton::setFont(const QFont &f)
{
    button->setFont(f);
}

//------------------------------------------------------------------------------
//
const QFont& QEMenuButton::font() const
{
    return button->font();
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

   QEMenuButtonModel model (NULL);
   bool status = model.parseXml (this->getMenuString ());
   if (status) {
      model.constructMenu (this->buttonMainMenu);
   }
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
