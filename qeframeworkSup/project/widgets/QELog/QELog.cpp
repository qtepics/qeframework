/*  QELog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at
 *  the Australian Synchrotron.
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
 *  Copyright (c) 2012,2016 Australian Synchrotron
 *
 *  Author:
 *    Ricardo Fernandes
 *  Contact details:
 *    ricardo.fernandes@synchrotron.org.au
 */

#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QHeaderView>
#include <QELog.h>

// =============================================================================
//  QELOG METHODS
// =============================================================================
QELog::QELog (QWidget* parent) : QWidget (parent), QEWidget (this)
{
   QFont qFont;

   qTableWidgetLog = new _QTableWidgetLog (this);
   qCheckBoxInfoMessage = new QCheckBox (this);
   qCheckBoxWarningMessage = new QCheckBox (this);
   qCheckBoxErrorMessage = new QCheckBox (this);
   qPushButtonClear = new QPushButton (this);
   qPushButtonSave = new QPushButton (this);

   qTableWidgetLog->setColumnCount (3);
   qTableWidgetLog->setHorizontalHeaderItem (0, new QTableWidgetItem ("Time"));
   qTableWidgetLog->setHorizontalHeaderItem (1, new QTableWidgetItem ("Type"));
   qTableWidgetLog->setHorizontalHeaderItem (2, new QTableWidgetItem ("Message"));
   qTableWidgetLog->setToolTip ("Current log messages");
   qTableWidgetLog->setEditTriggers (QAbstractItemView::NoEditTriggers);
   qTableWidgetLog->setSelectionBehavior (QAbstractItemView::SelectRows);
   qTableWidgetLog->setSelectionMode (QAbstractItemView::SingleSelection);
   qTableWidgetLog->verticalHeader ()->hide ();
   qFont.setPointSize (9);
   qTableWidgetLog->setFont (qFont);

   qCheckBoxInfoMessage->setText ("Info messages");
   qCheckBoxInfoMessage->setToolTip ("Show/hide info messages");
   qCheckBoxInfoMessage->setChecked (true);
   QObject::connect (qCheckBoxInfoMessage, SIGNAL (toggled (bool)), this,
                     SLOT (checkBoxInfoToggled (bool)));

   qCheckBoxWarningMessage->setText ("Warning messages");
   qCheckBoxWarningMessage->setToolTip ("Show/hide warning messages");
   qCheckBoxWarningMessage->setChecked (true);
   QObject::connect (qCheckBoxWarningMessage, SIGNAL (toggled (bool)), this,
                     SLOT (checkBoxWarningToggled (bool)));

   qCheckBoxErrorMessage->setText ("Error messages");
   qCheckBoxErrorMessage->setToolTip ("Show/hide error messages");
   qCheckBoxErrorMessage->setChecked (true);
   QObject::connect (qCheckBoxErrorMessage, SIGNAL (toggled (bool)), this,
                     SLOT (checkBoxErrorToggled (bool)));

   qPushButtonClear->setText ("Clear");
   qPushButtonClear->setToolTip ("Clear log messages");
   QObject::connect (qPushButtonClear, SIGNAL (clicked ()), this, SLOT (buttonClearClicked ()));

   qPushButtonSave->setText ("Save");
   qPushButtonSave->setToolTip ("Save log messages");
   QObject::connect (qPushButtonSave, SIGNAL (clicked ()), this, SLOT (buttonSaveClicked ()));

   setInfoColor (QColor (0, 0, 255));
   setWarningColor (QColor (255, 160, 0));
   setErrorColor (QColor (255, 0, 0));
   setScrollToBottom (true);
   setOptionsLayout (BOTTOM);

   clearLog ();

   // Set up the UserMessage class to match on any messages from widgets on the same form by default
   setFormFilter (MESSAGE_FILTER_MATCH);
   setSourceFilter (MESSAGE_FILTER_NONE);

   // Set the form ID to use when matching the form of received message
   setChildFormId (getFormId ());
}

//------------------------------------------------------------------------------
//
QELog::~QELog () {}

//------------------------------------------------------------------------------
//
void QELog::setShowColumnTime (bool pValue)
{
   qTableWidgetLog->setColumnHidden (0, pValue == false);
   qTableWidgetLog->refreshSize ();
}

//------------------------------------------------------------------------------
//
bool QELog::getShowColumnTime ()
{
   return (qTableWidgetLog->isColumnHidden (0) == false);
}

//------------------------------------------------------------------------------
//
void QELog::setShowColumnType (bool pValue)
{
   qTableWidgetLog->setColumnHidden (1, pValue == false);
   qTableWidgetLog->refreshSize ();
}

//------------------------------------------------------------------------------
//
bool QELog::getShowColumnType ()
{
   return (qTableWidgetLog->isColumnHidden (1) == false);
}

//------------------------------------------------------------------------------
//
void QELog::setShowColumnMessage (bool pValue)
{
   qTableWidgetLog->setColumnHidden (2, pValue == false);
   qTableWidgetLog->refreshSize ();
}

//------------------------------------------------------------------------------
//
bool QELog::getShowColumnMessage ()
{
   return (qTableWidgetLog->isColumnHidden (2) == false);
}

//------------------------------------------------------------------------------
//
void QELog::setShowMessageFilter (bool pValue)
{
   qCheckBoxInfoMessage->setVisible (pValue);
   qCheckBoxWarningMessage->setVisible (pValue);
   qCheckBoxErrorMessage->setVisible (pValue);
}

//------------------------------------------------------------------------------
//
bool QELog::getShowMessageFilter ()
{
   return qCheckBoxInfoMessage->isVisible ();
}

//------------------------------------------------------------------------------
//
void QELog::setShowClear (bool pValue)
{
   qPushButtonClear->setVisible (pValue);
}

//------------------------------------------------------------------------------
//
bool QELog::getShowClear ()
{
   return qPushButtonClear->isVisible ();
}

//------------------------------------------------------------------------------
//
void QELog::buttonClearClicked ()
{
   if (QMessageBox::
       question (this, "Info", "Do you want to clear the log messages?", QMessageBox::Yes,
                 QMessageBox::No) == QMessageBox::Yes) {
      clearLog ();
   }
}

//------------------------------------------------------------------------------
//
void QELog::setShowSave (bool pValue)
{
   qPushButtonSave->setVisible (pValue);
}

//------------------------------------------------------------------------------
//
bool QELog::getShowSave ()
{
   return qPushButtonSave->isVisible ();
}

//------------------------------------------------------------------------------
//
void QELog::setOptionsLayout (int pValue)
{
   QLayout *qLayoutMain;
   QLayout *qLayoutChild;

   delete layout ();

   switch (pValue) {
      case TOP:
         optionsLayout = TOP;
         qLayoutMain = new QVBoxLayout (this);
         qLayoutChild = new QHBoxLayout ();
         qLayoutChild->addWidget (qCheckBoxInfoMessage);
         qLayoutChild->addWidget (qCheckBoxWarningMessage);
         qLayoutChild->addWidget (qCheckBoxErrorMessage);
         qLayoutMain->addItem (qLayoutChild);
         qLayoutChild = new QHBoxLayout ();
         qLayoutChild->addWidget (qPushButtonClear);
         qLayoutChild->addWidget (qPushButtonSave);
         qLayoutMain->addItem (qLayoutChild);
         qLayoutMain->addWidget (qTableWidgetLog);
         break;

      case BOTTOM:
         optionsLayout = BOTTOM;
         qLayoutMain = new QVBoxLayout (this);
         qLayoutChild = new QHBoxLayout ();
         qLayoutMain->addWidget (qTableWidgetLog);
         qLayoutChild->addWidget (qCheckBoxInfoMessage);
         qLayoutChild->addWidget (qCheckBoxWarningMessage);
         qLayoutChild->addWidget (qCheckBoxErrorMessage);
         qLayoutMain->addItem (qLayoutChild);
         qLayoutChild = new QHBoxLayout ();
         qLayoutChild->addWidget (qPushButtonClear);
         qLayoutChild->addWidget (qPushButtonSave);
         qLayoutMain->addItem (qLayoutChild);
         break;

      case LEFT:
         optionsLayout = LEFT;
         qLayoutMain = new QHBoxLayout (this);
         qLayoutChild = new QVBoxLayout ();
         qLayoutChild->addWidget (qCheckBoxInfoMessage);
         qLayoutChild->addWidget (qCheckBoxWarningMessage);
         qLayoutChild->addWidget (qCheckBoxErrorMessage);
         qLayoutChild->addWidget (qPushButtonClear);
         qLayoutChild->addWidget (qPushButtonSave);
         qLayoutMain->addItem (qLayoutChild);
         qLayoutMain->addWidget (qTableWidgetLog);
         break;

      case RIGHT:
         optionsLayout = RIGHT;
         qLayoutMain = new QHBoxLayout (this);
         qLayoutChild = new QVBoxLayout ();
         qLayoutChild->addWidget (qCheckBoxInfoMessage);
         qLayoutChild->addWidget (qCheckBoxWarningMessage);
         qLayoutChild->addWidget (qCheckBoxErrorMessage);
         qLayoutChild->addWidget (qPushButtonClear);
         qLayoutChild->addWidget (qPushButtonSave);
         qLayoutMain->addWidget (qTableWidgetLog);
         qLayoutMain->addItem (qLayoutChild);
   }
}

//------------------------------------------------------------------------------
//
int QELog::getOptionsLayout ()
{
   return optionsLayout;
}

//------------------------------------------------------------------------------
//
void QELog::setScrollToBottom (bool pValue)
{
   scrollToBottom = pValue;
   if (scrollToBottom) {
      qTableWidgetLog->scrollToBottom ();
   }
}

//------------------------------------------------------------------------------
//
bool QELog::getScrollToBottom ()
{
   return scrollToBottom;
}

//------------------------------------------------------------------------------
//
void QELog::setInfoColor (QColor pValue)
{
   qColorInfo = pValue;
   qCheckBoxInfoMessage->setStyleSheet ("QCheckBox{color: rgb(" +
                                        QString::number (pValue.red ()) + ", " +
                                        QString::number (pValue.green ()) + ", " +
                                        QString::number (pValue.blue ()) + ")}");
   refreshLog ();
}

//------------------------------------------------------------------------------
//
QColor QELog::getInfoColor ()
{
   return qColorInfo;
}

//------------------------------------------------------------------------------
//
void QELog::setWarningColor (QColor pValue)
{
   qColorWarning = pValue;
   qCheckBoxWarningMessage->setStyleSheet ("QCheckBox{color: rgb(" +
                                           QString::number (pValue.red ()) + ", " +
                                           QString::number (pValue.green ()) + ", " +
                                           QString::number (pValue.blue ()) + ")}");
   refreshLog ();
}

//------------------------------------------------------------------------------
//
QColor QELog::getWarningColor ()
{
   return qColorWarning;
}

//------------------------------------------------------------------------------
//
void QELog::setErrorColor (QColor pValue)
{
   qColorError = pValue;
   qCheckBoxErrorMessage->setStyleSheet ("QCheckBox{color: rgb(" +
                                         QString::number (pValue.red ()) + ", " +
                                         QString::number (pValue.green ()) + ", " +
                                         QString::number (pValue.blue ()) + ")}");
   refreshLog ();
}

//------------------------------------------------------------------------------
//
QColor QELog::getErrorColor ()
{
   return qColorError;
}

//------------------------------------------------------------------------------
//
void QELog::checkBoxInfoToggled (bool)
{
   refreshLog ();
}

//------------------------------------------------------------------------------
//
void QELog::checkBoxWarningToggled (bool)
{
   refreshLog ();
}

//------------------------------------------------------------------------------
//
void QELog::checkBoxErrorToggled (bool)
{
   refreshLog ();
}

//------------------------------------------------------------------------------
//
void QELog::buttonSaveClicked ()
{
   QFileDialog *qFileDialog;
   QFile *file;
   QString filename;
   QString line;
   int i;

   qFileDialog = new QFileDialog (this, "Save log messages", QString ());
   qFileDialog->setAcceptMode (QFileDialog::AcceptSave);

   if (qFileDialog->exec ()) {
      filename = qFileDialog->selectedFiles ().at (0);
      file = new QFile (filename);
      if (file->open (QFile::WriteOnly | QFile::Text)) {
         QTextStream stream (file);
         for (i = 0; i < qTableWidgetLog->rowCount (); i++) {
            if (qTableWidgetLog->isRowHidden (i) == false) {
               if (getShowColumnTime ()) {
                  line = qTableWidgetLog->item (i, 0)->text ();
               } else {
                  line = "";
               }
               if (getShowColumnType ()) {
                  if (line.isEmpty ()) {
                     line = qTableWidgetLog->item (i, 1)->text ();
                  } else {
                     line += ", " + qTableWidgetLog->item (i, 1)->text ();
                  }
               }
               if (getShowColumnMessage ()) {
                  if (line.isEmpty ()) {
                     line = qTableWidgetLog->item (i, 2)->text ();
                  } else {
                     line += ", " + qTableWidgetLog->item (i, 2)->text ();
                  }
               }
               stream << line << "\n";
            }
         }
         file->close ();
         QMessageBox::information (this, "Info",
                                   "The log messages were successfully saved in file '" +
                                   filename + "'!");
      } else {
         QMessageBox::critical (this, "Error",
                                "Unable to save log messages in file '" + filename + "'!");
      }

   }
}

//------------------------------------------------------------------------------
//
void QELog::clearLog ()
{
   qTableWidgetLog->setRowCount (0);
   qPushButtonClear->setEnabled (false);
   qPushButtonSave->setEnabled (false);
}

//------------------------------------------------------------------------------
//
void QELog::addLog (int pType, QString pMessage)
{
   QTableWidgetItem *qTableWidgetItem;
   QString type;
   QColor color;
   int i;

   switch (pType) {
      case MESSAGE_TYPE_INFO:
         type = "INFO";
         color = qColorInfo;
         break;

      case MESSAGE_TYPE_WARNING:
         type = "WARNING";
         color = qColorWarning;
         break;

      case MESSAGE_TYPE_ERROR:
         type = "ERROR";
         color = qColorError;
         break;

      default:
         type = "";
   }

   if (type.isEmpty () == false) {
      i = qTableWidgetLog->rowCount ();
      qTableWidgetLog->insertRow (i);
      if (type == "INFO") {
         qTableWidgetLog->setRowHidden (i, qCheckBoxInfoMessage->isChecked () == false);
      } else if (type == "WARNING") {
         qTableWidgetLog->setRowHidden (i, qCheckBoxWarningMessage->isChecked () == false);
      } else {
         qTableWidgetLog->setRowHidden (i, qCheckBoxErrorMessage->isChecked () == false);
      }
      qTableWidgetItem =
            new QTableWidgetItem (QDateTime ().currentDateTime ().
                                  toString ("yyyy/MM/dd - hh:mm:ss"));
      qTableWidgetItem->setTextColor (color);
      qTableWidgetLog->setItem (i, 0, qTableWidgetItem);
      qTableWidgetItem = new QTableWidgetItem (type);
      qTableWidgetItem->setTextColor (color);
      qTableWidgetLog->setItem (i, 1, qTableWidgetItem);
      qTableWidgetItem = new QTableWidgetItem (pMessage);
      qTableWidgetItem->setTextColor (color);
      qTableWidgetLog->setItem (i, 2, qTableWidgetItem);
      if (scrollToBottom) {
         qTableWidgetLog->scrollToBottom ();
      }
      qPushButtonClear->setEnabled (true);
      qPushButtonSave->setEnabled (true);
   }
}

//------------------------------------------------------------------------------
//
void QELog::refreshLog ()
{
   QTableWidgetItem *qTableWidgetItem;
   QColor color;
   int i;

   for (i = 0; i < qTableWidgetLog->rowCount (); i++) {

      qTableWidgetItem = qTableWidgetLog->item (i, 1);

      if (qTableWidgetItem->text () == "INFO") {
         qTableWidgetLog->setRowHidden (i, qCheckBoxInfoMessage->isChecked () == false);
         color = qColorInfo;
      } else if (qTableWidgetItem->text () == "WARNING") {
         qTableWidgetLog->setRowHidden (i, qCheckBoxWarningMessage->isChecked () == false);
         color = qColorWarning;
      } else {
         qTableWidgetLog->setRowHidden (i, qCheckBoxErrorMessage->isChecked () == false);
         color = qColorError;
      }

      qTableWidgetItem->setTextColor (color);

      qTableWidgetItem = qTableWidgetLog->item (i, 0);
      qTableWidgetItem->setTextColor (color);

      qTableWidgetItem = qTableWidgetLog->item (i, 2);
      qTableWidgetItem->setTextColor (color);

   }
}

//------------------------------------------------------------------------------
//
void QELog::setOptionsLayoutProperty (optionsLayoutProperty pOptionsLayout)
{
   setOptionsLayout ((optionsLayoutProperty) pOptionsLayout);
}

//------------------------------------------------------------------------------
//
QELog::optionsLayoutProperty QELog::getOptionsLayoutProperty ()
{
   return (optionsLayoutProperty) getOptionsLayout ();
}

//------------------------------------------------------------------------------
//
QELog::MessageFilterOptions QELog::getMessageFormFilter ()
{
   return (MessageFilterOptions) getFormFilter ();
}

//------------------------------------------------------------------------------
//
void QELog::setMessageFormFilter (MessageFilterOptions messageFormFilter)
{
   setFormFilter ((message_filter_options) messageFormFilter);
}

//------------------------------------------------------------------------------
//
QELog::MessageFilterOptions QELog::getMessageSourceFilter ()
{
   return (MessageFilterOptions) getSourceFilter ();
}

//------------------------------------------------------------------------------
//
void QELog::setMessageSourceFilter (MessageFilterOptions messageSourceFilter)
{
   setSourceFilter ((message_filter_options) messageSourceFilter);
}

//------------------------------------------------------------------------------
// Receive new log messages from other QEWidgets.
void QELog::newMessage (QString msg, message_types type)
{
   // Add the message to the log
   if ((type.kind_set & MESSAGE_KIND_EVENT) != 0) {
      addLog (type.severity, msg);
   }
}


// =============================================================================
//  _QTABLEWIDGETLOG METHODS
// =============================================================================
_QTableWidgetLog::_QTableWidgetLog (QWidget * pParent):QTableWidget (pParent)
{
   initialized = false;
}

//------------------------------------------------------------------------------
//
void _QTableWidgetLog::refreshSize ()
{
   int sizeColumn0;
   int sizeColumn1;
   int sizeColumn2;


   if (isColumnHidden (0)) {
      if (isColumnHidden (1)) {
         if (isColumnHidden (2)) {
            sizeColumn0 = 0;
            sizeColumn1 = 0;
            sizeColumn2 = 0;
         } else {
            sizeColumn0 = 0;
            sizeColumn1 = 0;
            sizeColumn2 = width ();
         }
      } else {
         if (isColumnHidden (2)) {
            sizeColumn0 = 0;
            sizeColumn1 = width ();
            sizeColumn2 = 0;
         } else {
            sizeColumn0 = 0;
            sizeColumn1 = 1 * width () / 5;
            sizeColumn2 = 4 * width () / 5 - 1;
         }
      }
   } else {
      if (isColumnHidden (1)) {
         if (isColumnHidden (2)) {
            sizeColumn0 = width ();
            sizeColumn1 = 0;
            sizeColumn2 = 0;
         } else {
            sizeColumn0 = 1 * width () / 5;
            sizeColumn1 = 0;
            sizeColumn2 = 4 * width () / 5 - 1;
         }
      } else {
         if (isColumnHidden (2)) {
            sizeColumn0 = width () / 2;
            sizeColumn1 = width () / 2 - 1;
            sizeColumn2 = 0;
         } else {
            sizeColumn0 = 1 * width () / 5;
            sizeColumn1 = 1 * width () / 5;
            sizeColumn2 = 3 * width () / 5 - 1;
         }
      }
   }

   setColumnWidth (0, sizeColumn0);
   setColumnWidth (1, sizeColumn1);
   setColumnWidth (2, sizeColumn2);
}

//------------------------------------------------------------------------------
//
void _QTableWidgetLog::resizeEvent (QResizeEvent *)
{
   // TODO: this condition should always be execute when inside Qt Designer
   if (initialized == false) {
      refreshSize ();
      initialized = true;
   }
}

// end
