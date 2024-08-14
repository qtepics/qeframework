/*  QEFormStateChange.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2019-2022 Australian Synchrotron
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
 *    andrews@ansto.gov.au
 */

#include <QEFormStateChange.h>
#include <QDebug>
#include <QPainter>
#include <QTimer>
#include <QEString.h>

#define DEBUG qDebug () << "QEFormStateChange" << __LINE__ << __FUNCTION__ << "  "

//==============================================================================
// QEFormStateChange::Actions
//==============================================================================
//
class QEFormStateChange::Actions {
public:
   explicit Actions (const VariableIndicies index, const QEFormStateChange* owner);
   ~Actions ();

   void doAction ();

   const VariableIndicies index;
   const QEFormStateChange* owner;

   QEStringFormatting formatting;
   QCaVariableNamePropertyManager vnpm;
   applicationLauncher launcher;
   QString text;
};

//------------------------------------------------------------------------------
//
QEFormStateChange::Actions::Actions (const VariableIndicies indexIn,
                                     const QEFormStateChange* ownerIn) :
   index (indexIn),
   owner (ownerIn)
{
   this->text = "1";
   this->vnpm.setVariableIndex (this->index);
   QObject::connect (&this->vnpm, SIGNAL (newVariableNameProperty (QString, QString, unsigned int)),
                     this->owner, SLOT   (newVariableNameProperty (QString, QString, unsigned int)));

}

//------------------------------------------------------------------------------
//
QEFormStateChange::Actions::~Actions () { }

//------------------------------------------------------------------------------
//
void QEFormStateChange::Actions::doAction ()
{
   // Do not apply when in designer.
   //
   if (QEWidget::inDesigner ()) return;

   QEString* qca = dynamic_cast <QEString*> (this->owner->getQcaItem (this->index));
   if (qca && qca->getChannelIsConnected ()) {
      qca->writeStringElement (this->text);   // performs required formatting
   }

   // If there is a command to run, then run it, with substitutions applied to
   // the command and arguments.
   //
   const VariableNameManager* vm;
   vm = static_cast<const VariableNameManager*> (this->owner);
   this->launcher.launch (vm, NULL);
}

//==============================================================================
// QEFormStateChange
//==============================================================================
//
// Constructor with no initialisation
//
QEFormStateChange::QEFormStateChange (QWidget *parent) :
   QWidget (parent),
   QEWidget (this),
   QEStringFormattingMethods ()
{
   this->setup ();
}

//------------------------------------------------------------------------------
// Constructor with known variable
//
QEFormStateChange::QEFormStateChange (const QString& openVariableName,
                                      QWidget *parent) :
   QWidget (parent),
   QEWidget (this),
   QEStringFormattingMethods ()
{
   this->setup ();
   this->setVariableName (openVariableName, viOpen);
   this->activate ();
}

//------------------------------------------------------------------------------
// Constructor with known variables
//
QEFormStateChange::QEFormStateChange (const QString& openVariableName,
                                      const QString& closeVariableName,
                                      QWidget* parent):
   QWidget (parent),
   QEWidget (this),
   QEStringFormattingMethods ()
{
   this->setup ();
   this->setVariableName (openVariableName,  viOpen);
   this->setVariableName (closeVariableName, viClose);
   this->activate ();
}

//------------------------------------------------------------------------------
//
QEFormStateChange::~QEFormStateChange ()
{
   this->windowClosed ();
}

//------------------------------------------------------------------------------
// Setup common to all constructors
//
void QEFormStateChange::setup ()
{
   this->actionList [viOpen]  = new Actions (viOpen, this);
   this->actionList [viClose] = new Actions (viClose, this);

   this->setMinimumSize (8,8);
   this->setMaximumSize (24,24);

   // Set up data
   //
   this->setNumVariables (NUMBER_OF_VARIABLES);

   // Set up default properties
   //
   this->setAllowDrop (false);
   this->setRunVisible (false);

   // Allow time for the openVariable to connect.
   // 100 mS is a bit arbitary.
   // Maybe we can be a bit smarter.
   //
   QTimer::singleShot (100, this, SLOT (windowOpened ()));
}

//------------------------------------------------------------------------------
//
QSize QEFormStateChange::sizeHint ()
{
   return QSize (12, 12);
}

//------------------------------------------------------------------------------
//
void QEFormStateChange::setOpenVariableNameProperty (const QString pvName)
{
   this->actionList [viOpen]->vnpm.setVariableNameProperty (pvName);
}

//------------------------------------------------------------------------------
//
QString QEFormStateChange::getOpenVariableNameProperty () const
{
   return this->actionList [viOpen]->vnpm.getVariableNameProperty ();
}

//------------------------------------------------------------------------------
//
void QEFormStateChange::setCloseVariableNameProperty (const QString pvName)
{
   this->actionList [viClose]->vnpm.setVariableNameProperty (pvName);
}

//------------------------------------------------------------------------------
//
QString QEFormStateChange::getCloseVariableNameProperty () const
{
   return this->actionList [viClose]->vnpm.getVariableNameProperty ();
}

//------------------------------------------------------------------------------
//
void QEFormStateChange::setVariableNameSubstitutionsProperty (const QString value)
{
   // Use same default subsitutions for all PVs used by this widget.
   //
   for (int j = 0; j < NUMBER_OF_VARIABLES; j++) {
      this->actionList [j]->vnpm.setSubstitutionsProperty (value);
   }
}

//------------------------------------------------------------------------------
//
QString QEFormStateChange::getVariableNameSubstitutionsProperty () const
{
   // Either one of the name managers can provide the subsitutions.
   //
   return this->actionList [0]->vnpm.getSubstitutionsProperty ();
}

//------------------------------------------------------------------------------
//
void QEFormStateChange::setOpenText (const QString value)
{
   this->actionList [viOpen]->text = value;
}

//------------------------------------------------------------------------------
//
QString QEFormStateChange::getOpenText () const
{
   return this->actionList [viOpen]->text;
}

//------------------------------------------------------------------------------
//
void QEFormStateChange::setOpenProgram (const QString progarm)
{
   this->actionList [viOpen]->launcher.setProgram (progarm);
}

//------------------------------------------------------------------------------
//
QString QEFormStateChange::getOpenProgram () const
{
   return this->actionList [viOpen]->launcher.getProgram ();
}

//------------------------------------------------------------------------------
//
void QEFormStateChange::setOpenArguments (const QStringList arguments)
{
   this->actionList [viOpen]->launcher.setArguments (arguments);
}

//------------------------------------------------------------------------------
//
QStringList QEFormStateChange::getOpenArguments () const
{
   return this->actionList [viOpen]->launcher.getArguments ();
}

//------------------------------------------------------------------------------
//
void QEFormStateChange::setCloseText (const QString value)
{
   this->actionList [viClose]->text = value;
}

//------------------------------------------------------------------------------
//
QString QEFormStateChange::getCloseText () const
{
   return this->actionList [viClose]->text;
}

//------------------------------------------------------------------------------
//
void QEFormStateChange::setCloseProgram (const QString progarm)
{
   this->actionList [viClose]->launcher.setProgram (progarm);
}

//------------------------------------------------------------------------------
//
QString QEFormStateChange::getCloseProgram () const
{
   return this->actionList [viClose]->launcher.getProgram ();
}

//------------------------------------------------------------------------------
//
void QEFormStateChange::setCloseArguments (const QStringList arguments)
{
   this->actionList [viClose]->launcher.setArguments (arguments);
}

//------------------------------------------------------------------------------
//
QStringList QEFormStateChange::getCloseArguments () const
{
   return this->actionList [viClose]->launcher.getArguments ();
}

//------------------------------------------------------------------------------
//
void QEFormStateChange::setFormatProperty (const QE::Formats format)
{
   this->setFormat (format);
}

//------------------------------------------------------------------------------
//
QE::Formats QEFormStateChange::getFormatProperty () const
{
   return this->getFormat ();
}

//------------------------------------------------------------------------------
//
void QEFormStateChange::paintEvent (QPaintEvent* /* event */)
{
   QPainter painter (this);
   QPen pen;
   QBrush brush;

   pen.setWidth (1);
   pen.setStyle (Qt::SolidLine);
   pen.setColor (QColor ("#607080"));
   painter.setPen (pen);

   brush.setStyle (Qt::SolidPattern);
   brush.setColor (QColor ("#c0e0ff"));
   painter.setBrush (brush);

   painter.drawRect (0, 0, this->width () - 1, this->height () - 1);
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of
// QCaObject required. For a QEFormStateChange, a QCaObject that accepts strings
// is required.
//
qcaobject::QCaObject* QEFormStateChange::createQcaItem (unsigned int variableIndex)
{
   if (variableIndex >= NUMBER_OF_VARIABLES) return NULL;

   // Create all items as QEFloating
   QString pvName = this->getSubstitutedVariableName (variableIndex);
   return new QEString (pvName, this, &this->actionList[variableIndex]->formatting, variableIndex);
}

//------------------------------------------------------------------------------
// Start updating.
// Implementation of VariableNameManager's virtual funtion to establish a
// connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QEFormStateChange::establishConnection (unsigned int variableIndex)
{
   if (variableIndex >= NUMBER_OF_VARIABLES) return;

   // Create a connection. We don't need any connection/update signals.
   //
   this->createConnection (variableIndex, false);
}

//------------------------------------------------------------------------------
//
void QEFormStateChange::stringFormattingChange () { }

//------------------------------------------------------------------------------
//
void QEFormStateChange::windowOpened ()
{
   this->actionList [viOpen]->doAction ();
}

//------------------------------------------------------------------------------
//
void QEFormStateChange::windowClosed ()
{
   this->actionList [viClose]->doAction ();
}

//------------------------------------------------------------------------------
//
void QEFormStateChange::newVariableNameProperty (QString pvName,
                                                 QString subs,
                                                 unsigned int vi)
{
   if (vi >= NUMBER_OF_VARIABLES) return;
   this->setVariableNameAndSubstitutions (pvName, subs, vi);
}

// end
