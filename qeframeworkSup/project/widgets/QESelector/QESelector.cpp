/*  QESelector.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2021-2022 Australian Synchrotron
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

#include "QESelector.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>

#define DEBUG  qDebug () << "QESelector" << __LINE__ << __FUNCTION__ << " "

#define PV_VARIABLE_INDEX      0
#define EMPTY_TEXT             "None"

//------------------------------------------------------------------------------
//
QESelector::QESelector (QWidget* parent) :
   QEAbstractWidget (parent),
   QESingleVariableMethods ( this, PV_VARIABLE_INDEX )
{
   this->commonSetup();
}

//------------------------------------------------------------------------------
//
QESelector::QESelector (const QString& variableNameIn, QWidget* parent) :
   QEAbstractWidget (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX)
{
   this->setVariableName (variableNameIn, PV_VARIABLE_INDEX);
   this->commonSetup();
   this->activate();
}

//------------------------------------------------------------------------------
//
QESelector::~QESelector ()
{
   // place holder
}

//------------------------------------------------------------------------------
//
void QESelector::commonSetup()
{
   // Create internal widget.
   //
   this->internalWidget = new QComboBox (this);

   // Copy actual widget size policy to the containing widget, then ensure
   // internal widget will expand to fill container widget.
   //
   this->setSizePolicy (this->internalWidget->sizePolicy ());
   this->internalWidget->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);

   this->internalWidget->setMaxVisibleItems (40);

   this->layout = new QHBoxLayout (this);
   this->layout->setContentsMargins (0, 0, 0, 0);    // extact fit.
   this->layout->addWidget (this->internalWidget);

   // We use the activated signal (as opposed to currentIndexChanged) as it
   // is only emmited on User change.
   //
   QObject::connect (this->internalWidget, SIGNAL (activated (const QString &)),
                     this,                 SLOT   (selected  (const QString &)));

   // Set default property values
   //
   this->userInfoList.clear();
   this->fileInfoList.clear();
   this->infoSource = stringListSource;
   this->infoList = &this->userInfoList;
   this->delimiter = SpaceDelimiter;
   this->internalWidget->addItem (EMPTY_TEXT);
   this->subscribe = true;
   this->writeOnChange = true;
   this->isAllowFocusUpdate = false;

   // Set up data
   //
   this->setNumVariables (1);

   // Set variable index used to select write access cursor style.
   //
   this->setControlPV (PV_VARIABLE_INDEX);

   this->setMinimumSize (40, 16);

   // Set up default properties
   //
   this->setVariableAsToolTip (true);
   this->setAllowDrop (true);
   this->setDisplayAlarmStateOption (standardProperties::DISPLAY_ALARM_STATE_ALWAYS);

   // Use default context menu.
   // Note: Turn off the menu for the internal QComboBox widget.
   //
   this->internalWidget->setContextMenuPolicy (Qt::NoContextMenu);
   this->setupContextMenu ();

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   this->connectNewVariableNameProperty
         (SLOT (useNewVariableNameProperty (QString, QString, unsigned int)));

   // Some events must be applied to the internal widget
   //
   this->installEventFilter (this);
}

//------------------------------------------------------------------------------
//
void QESelector::updateDropDownList ()
{
   this->internalWidget->clear();
   this->internalWidget->addItem (EMPTY_TEXT);
   this->internalWidget->addItems (*this->infoList);
}

//------------------------------------------------------------------------------
// Properties
//------------------------------------------------------------------------------
//
void QESelector::setSource (const SourceOptions infoSourceIn)
{
   this->infoSource = infoSourceIn;

   // Update the reference.
   //
   this->infoList = (this->infoSource == stringListSource) ?
            &this->userInfoList :
            &this->fileInfoList;

   this->updateDropDownList();
}

//------------------------------------------------------------------------------
//
QESelector::SourceOptions QESelector::getSource () const
{
   return this->infoSource;
}

//------------------------------------------------------------------------------
//
void QESelector::setStringList (const QStringList& valueListIn)
{
   this->userInfoList = valueListIn;
   if (this->infoSource == stringListSource) {
      this->updateDropDownList();
   }
}

//------------------------------------------------------------------------------
//
QStringList QESelector::getStringList () const
{
   return this->userInfoList;
}

//------------------------------------------------------------------------------
//
void QESelector::setSourceFilename (const QString& userInfoFileIn)
{
   this->userInfoFile = userInfoFileIn;

   // Read file.
   this->fileInfoList = QESelector::readList (userInfoFileIn);

   if (this->infoSource == textFileSource) {
      this->updateDropDownList();
   }
}

//------------------------------------------------------------------------------
//
QString QESelector::getSourceFilename () const
{
   return this->userInfoFile;
}

//------------------------------------------------------------------------------
//
void QESelector::setDelimiter (const Delimiters delimiterIn)
{
   this->delimiter = delimiterIn;
}

//------------------------------------------------------------------------------
//
QESelector::Delimiters QESelector::getDelimiter () const
{
   return this->delimiter;
}

//------------------------------------------------------------------------------
//
void QESelector::setSubscribe (const bool subscribeIn)
{
   this->subscribe = subscribeIn;
}

//------------------------------------------------------------------------------
//
bool QESelector::getSubscribe () const
{
   return this->subscribe;
}

//------------------------------------------------------------------------------
//
void QESelector::setWriteOnChange (const  bool writeOnChangeIn)
{
   this->writeOnChange = writeOnChangeIn;
}

//------------------------------------------------------------------------------
//
bool QESelector::getWriteOnChange () const
{
   return this->writeOnChange;
}

//------------------------------------------------------------------------------
//
void QESelector::setAllowFocusUpdate (const bool allowFocusUpdateIn)
{
   this->isAllowFocusUpdate = allowFocusUpdateIn;
}

//------------------------------------------------------------------------------
//
bool QESelector::getAllowFocusUpdate () const
{
   return this->isAllowFocusUpdate;
}

//------------------------------------------------------------------------------
//
QSize QESelector::sizeHint () const
{
   QSize result;
   if (this->internalWidget) {
      result = this->internalWidget->sizeHint();
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QESelector::focusInEvent (QFocusEvent* event)
{
   this->internalWidget->setFocus ();   // pass to enclosed widget
   QWidget::focusInEvent (event);       // pass to parent
}

//------------------------------------------------------------------------------
//
bool QESelector::eventFilter (QObject* watched, QEvent* event)
{
   const QEvent::Type type = event->type ();
   bool result = false;

   switch (type) {
      case QEvent::FontChange:
         if (watched == this) {
            // Font must be mapped to the internal button
            //
            if (this->internalWidget) {
               this->internalWidget->setFont (this->font ());
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
void QESelector::establishConnection (unsigned int variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   // Create a connection.
   // If successfull, the QCaObject object that will supply data update
   // signals will be returned.  Note createConnection creates the connection
   // and returns reference to existing QCaObject.
   //
   qcaobject::QCaObject* qca = this->createConnection (variableIndex);

   // If a QCaObject object is now available to supply data update signals,
   // connect it to the appropriate slots.
   //
   if (qca) {
      QObject::connect (qca,  SIGNAL (connectionChanged (QCaConnectionInfo&, const unsigned int&)),
                        this, SLOT   (connectionChanged (QCaConnectionInfo&, const unsigned int&)));

      QObject::connect (qca,  SIGNAL (stringChanged (const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)),
                        this, SLOT   (valueUpdate   (const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)));
   }
}

//------------------------------------------------------------------------------
//
qcaobject::QCaObject* QESelector::createQcaItem (unsigned int variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return NULL;
   }

   qcaobject::QCaObject* result = NULL;
   result = new QEString (this->getSubstitutedVariableName (variableIndex),
                          this, &this->formatting, variableIndex);

   // Apply currently defined array index/elements request values.
   //
   this->setSingleVariableQCaProperties (result);

   return result;
}

//------------------------------------------------------------------------------
//
void QESelector::connectionChanged (QCaConnectionInfo& connectionInfo,
                                    const unsigned int &variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   // Note the connected state
   //
   bool isConnected = connectionInfo.isChannelConnected ();

   // More toub. than worth to check if this a connect or a disconnect.
   //
   this->isFirstUpdate = true;

   // Display the connected state
   // Note: only one/first "variable" is a PV. Modify the tool tip class object
   //       to only display actual PV name and connection status.
   //
   this->setNumberToolTipVariables (1);
   this->updateToolTipConnection (isConnected, variableIndex);
   this->processConnectionInfo (isConnected, variableIndex);

   this->internalWidget->setEnabled (isConnected);

   // Set cursor to indicate access mode.
   //
   this->setAccessCursorStyle ();

   // Signal channel connection change to any (Link) widgets.
   // using signal dbConnectionChanged.
   //
   this->emitDbConnectionChanged (PV_VARIABLE_INDEX);
}

//------------------------------------------------------------------------------
//
void QESelector::valueUpdate (const QString& text,
                              QCaAlarmInfo& alarmInfo,
                              QCaDateTime&,
                              const unsigned int& variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   bool found = false;
   if (text.isEmpty()) {
      this->internalWidget->setCurrentIndex(0);
      found = true;
   } else {
      for (int j = 0; j < this->infoList->count(); j++) {
         QString option = this->infoList->value(j);
         option = this->extractValue (option);
         if (text == option) {
            // Found it
            //
            found = true;

            // Update the text if appropriate. If the user is editing the object
            // then updates will be inapropriate, unless it is the first update
            // or allow updated while focused explicitly permitted. Allow if the
            // form designer has specifically allowed updates while the widget
            // has focus.
            //
            if (this->isAllowFocusUpdate || !this->hasFocus() || this->isFirstUpdate) {

               // recall 0th entry used for null text
               this->internalWidget->setCurrentIndex(j + 1);
            }

            break;
         }
      }
   }

   if (!found) {
     DEBUG << text << "did not match any of the allowed values";
   }

   // Invoke common alarm handling processing.
   //
   this->processAlarmInfo (alarmInfo, variableIndex);

   // First (and subsequent) update is now over
   //
   this->isFirstUpdate = false;

   // Signal a database value change to any Link (or other) widgets using one
   // of the dbValueChanged. Because the write underying QComboBox may not have
   // occured (because we had focus), we cannot use the currentText () function.
   //
   this->emitDbValueChanged (text, PV_VARIABLE_INDEX);
}

//------------------------------------------------------------------------------
//
void QESelector::useNewVariableNameProperty (QString variableNameIn,
                                             QString substitutionsIn,
                                             unsigned int variableIndex)
{
   this->setVariableNameAndSubstitutions (variableNameIn,
                                          substitutionsIn,
                                          variableIndex);
}

//------------------------------------------------------------------------------
// slot
void QESelector::writeNow()
{
   QEString* qca = qobject_cast<QEString*>(this->getQcaItem (PV_VARIABLE_INDEX));
   if (!qca)  return;  // sanity check

   QString text = this->internalWidget->currentText();
   if (text == EMPTY_TEXT) {
      text.clear();
   }

   text = this->extractValue (text);
   qca->writeStringElement (text);
}

//------------------------------------------------------------------------------
// slot
void QESelector::selected (const QString&)
{
   if (this->writeOnChange) {
      this->writeNow();
   }
}

//------------------------------------------------------------------------------
// Copy / Paste
//
QString QESelector::copyVariable ()
{
   return this->getSubstitutedVariableName (PV_VARIABLE_INDEX);
}

//------------------------------------------------------------------------------
//
QVariant QESelector::copyData ()
{
   QString item = this->extractValue(this->internalWidget->currentText ());
   return QVariant (item);
}

//------------------------------------------------------------------------------
//
void QESelector::paste (QVariant v)
{
   this->setVariableName (v.toString (), PV_VARIABLE_INDEX);
   this->establishConnection (PV_VARIABLE_INDEX);
}

//------------------------------------------------------------------------------
//
QString QESelector::extractValue (const QString& item) const
{
   QString result = item;
   int p;

   switch (this->delimiter) {
      case NoDelimiter:
         break;

      case SpaceDelimiter:
         p = result.indexOf (' ', 0);
         if (p >= 0) {
            result.truncate (p);
         }
         break;

      case CommaDelimiter:
         p = result.indexOf (',', 0);
         if (p >= 0) {
            result.truncate (p);
         }
         break;

      default:
         DEBUG << "unexpected delimter: " << int(this->delimiter);
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
// static
QStringList QESelector::readList (const QString& filename)
{
   // Cribbed from kdm_settings_utilities::readNameList()

   QStringList result;

   QFile file (filename);
   if (!file.open (QIODevice::ReadOnly | QIODevice::Text)) {
      DEBUG << filename << " file open (read) failed";
      return result;
   }

   QTextStream source (&file);
   while (!source.atEnd()) {
      QString item = source.readLine ().trimmed ();

      // Skip empty line and comment lines.
      //
      if (item.length () == 0) continue;
      if (item.left (1) == "#") continue;

      // Trim off comments.
      //
      int p = item.indexOf ('#', 0);
      if (p >= 0) {
         item.truncate (p);
         item = item.trimmed ();
      }

      result.append (item);
   }
   file.close ();

   return result;
}

// end
