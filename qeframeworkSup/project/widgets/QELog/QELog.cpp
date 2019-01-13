/*  QELog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at
 *  the Australian Synchrotron.
 *
 *  Copyright (c) 2012-2019 Australian Synchrotron
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
 *  Author:
 *    Ricardo Fernandes
 *  Contact details:
 *    ricardo.fernandes@synchrotron.org.au
 */

#include <QELog.h>
#include <QDebug>
#include <QList>
#include <QStringList>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QHeaderView>
#include <QECommon.h>

#define DEBUG qDebug() << "QELog" << __LINE__ << __FUNCTION__ << "  "


//==============================================================================
// UserMessageReceiver - almost a direct crib from kubili
//==============================================================================
// We don't use the form as a message receiver as it may not exist before
// messages are created.
//
class QELog::UserMessageReceiver : public UserMessage {
public:
   explicit UserMessageReceiver ();
   ~UserMessageReceiver ();

   void registerLogWidget (QELog* logWidget);
   void deregisterLogWidget (QELog* logWidget);

protected:
   // override parent class virtual function
   //
   void newMessage (QString message, message_types mt);

private:
   struct MessageData {
      QDateTime dateTime;
      QString message;
      message_types mt;
   };

   typedef QList <MessageData> MessageDataList;
   MessageDataList messageDataCache;
   QELog* logWidget;   // only one for now.
};

//------------------------------------------------------------------------------
//
QELog::UserMessageReceiver::UserMessageReceiver () : UserMessage ()
{
   this->logWidget = NULL;
   this->messageDataCache.clear();

   // Grab any and all messages.
   //
   this->setFormFilter (MESSAGE_FILTER_ANY);
   this->setSourceFilter (MESSAGE_FILTER_ANY);
}

//------------------------------------------------------------------------------
// Place holder
QELog::UserMessageReceiver::~UserMessageReceiver () { }

//------------------------------------------------------------------------------
//
void QELog::UserMessageReceiver::registerLogWidget (QELog* logWidgetIn)
{
   this->logWidget = logWidgetIn;
   if (!this->logWidget) return;  // null logWidget - ignore.

   while (this->messageDataCache.count() > 0) {
      const MessageData md = this->messageDataCache.value (0);
      this->logWidget->processMessage (md.message, md.mt, md.dateTime);
      this->messageDataCache.removeFirst ();
   }
}

//------------------------------------------------------------------------------
//
void QELog::UserMessageReceiver::deregisterLogWidget (QELog* logWidgetIn)
{
   if (this->logWidget == logWidgetIn) this->logWidget = NULL;
}

//------------------------------------------------------------------------------
// overrides parent function
void QELog::UserMessageReceiver::newMessage (QString message, message_types mt)
{
   // Has the logWidget registered? If so, it's own newMessage function
   // handles the receiving of messages - we are now essentially inactive.
   //
   if (this->logWidget) return;

   // Filter for events - skip status messages
   //
   if ((mt.kind_set & MESSAGE_KIND_EVENT) != 0) {

      const QDateTime dateTime = QDateTime::currentDateTime();

      // Store until form is registered.
      // Keep upto a maximum of 1000 messages.
      // This limit is somewhat arbitary.
      //
      if (this->messageDataCache.length() >= 1000) {
         this->messageDataCache.removeFirst ();
      }

      MessageData md;
      md.dateTime = dateTime;
      md.message = message;
      md.mt = mt;
      this->messageDataCache.append (md);
   }
}

//------------------------------------------------------------------------------
// Alas allocating a new object here at elaboration time causes a seg fault on
// Windows with some versions of Qt (Qt5.6). So we do a delayed construction,
// invoked by any other UserMessage object construction.
// Note: we must avoid infinite loops.
//
static QELog::UserMessageReceiver* messageReceiver = NULL;
static bool constructionStarted = false;

//------------------------------------------------------------------------------
// static
void QELog::createUserMessageReceiver ()
{
   // messageReceiver is a singleton.
   //
   if (!constructionStarted && !messageReceiver) {
      constructionStarted = true;   // avoid infinite loops
      messageReceiver = new QELog::UserMessageReceiver();
   }
}

// =============================================================================
//  QELOG METHODS
// =============================================================================
QELog::QELog (QWidget* parent) : QEFrame (parent)
{
   QFont qFont;

   // Set the default frame settings.
   //
   this->setFrameShape (QFrame::NoFrame);
   this->setFrameShadow (QFrame::Plain);

   this->isMaster = false;
   this->mainLayoutMargin = 6;

   this->qTableWidgetLog = new QTableWidget (this);
   this->qCheckBoxInfoMessage = new QCheckBox (this);
   this->qCheckBoxWarningMessage = new QCheckBox (this);
   this->qCheckBoxErrorMessage = new QCheckBox (this);
   this->qPushButtonClear = new QPushButton (this);
   this->qPushButtonSave = new QPushButton (this);

   this->qTableWidgetLog->setColumnCount (3);
   this->qTableWidgetLog->setHorizontalHeaderItem (0, new QTableWidgetItem ("Time"));
   this->qTableWidgetLog->setHorizontalHeaderItem (1, new QTableWidgetItem ("Type"));
   this->qTableWidgetLog->setHorizontalHeaderItem (2, new QTableWidgetItem ("Message"));
   this->qTableWidgetLog->setColumnWidth (0, 156);
   this->qTableWidgetLog->setColumnWidth (1, 80);
   this->qTableWidgetLog->horizontalHeader()->setStretchLastSection (true);
   this->qTableWidgetLog->setToolTip ("Current log messages");
   this->qTableWidgetLog->setEditTriggers (QAbstractItemView::NoEditTriggers);
   this->qTableWidgetLog->setSelectionBehavior (QAbstractItemView::SelectRows);
   this->qTableWidgetLog->setSelectionMode (QAbstractItemView::SingleSelection);
   this->qTableWidgetLog->verticalHeader ()->hide ();
   qFont = this->qTableWidgetLog->font();
   qFont.setPointSize (9);
   this->qTableWidgetLog->setFont (qFont);

   this->qCheckBoxInfoMessage->setText ("Info messages");
   this->qCheckBoxInfoMessage->setToolTip ("Show/hide info messages");
   this->qCheckBoxInfoMessage->setChecked (true);
   QObject::connect (this->qCheckBoxInfoMessage, SIGNAL (toggled (bool)),
                     this, SLOT (checkBoxInfoToggled (bool)));

   this->qCheckBoxWarningMessage->setText ("Warning messages");
   this->qCheckBoxWarningMessage->setToolTip ("Show/hide warning messages");
   this->qCheckBoxWarningMessage->setChecked (true);
   QObject::connect (this->qCheckBoxWarningMessage, SIGNAL (toggled (bool)),
                     this, SLOT (checkBoxWarningToggled (bool)));

   this->qCheckBoxErrorMessage->setText ("Error messages");
   this->qCheckBoxErrorMessage->setToolTip ("Show/hide error messages");
   this->qCheckBoxErrorMessage->setChecked (true);
   QObject::connect (this->qCheckBoxErrorMessage, SIGNAL (toggled (bool)),
                     this, SLOT (checkBoxErrorToggled (bool)));

   this->qPushButtonClear->setText ("Clear");
   this->qPushButtonClear->setToolTip ("Clear log messages");
   QObject::connect (this->qPushButtonClear, SIGNAL (clicked ()),
                     this, SLOT (buttonClearClicked ()));

   this->qPushButtonSave->setText ("Save");
   this->qPushButtonSave->setToolTip ("Save log messages");

   QObject::connect (this->qPushButtonSave, SIGNAL (clicked ()),
                     this, SLOT (buttonSaveClicked ()));

   this->setInfoColor (QColor (0, 0, 255));
   this->setWarningColor (QColor (255, 160, 0));
   this->setErrorColor (QColor (255, 0, 0));
   this->setScrollToBottom (true);
   this->setOptionsLayout (Bottom);

   this->clearLog ();

   // Set up the UserMessage class to match on any messages from widgets on the same form by default
   this->setFormFilter (MESSAGE_FILTER_MATCH);
   this->setSourceFilter (MESSAGE_FILTER_NONE);

   // Set the form ID to use when matching the form of received message
   this->setChildFormId (this->getFormId ());
}

//------------------------------------------------------------------------------
//
QELog::~QELog ()
{
   if (messageReceiver) {
      messageReceiver->deregisterLogWidget (this);
   }
}

//------------------------------------------------------------------------------
//
void QELog::resizeEvent(QResizeEvent* )
{
   // Get current point size
   //
   const int ps = this->qTableWidgetLog->font().pointSize();

   // The col widths 156/80 based on a point size of 9 - apply size scale.
   //
   this->qTableWidgetLog->setColumnWidth (0, (156 * ps) / 9);
   this->qTableWidgetLog->setColumnWidth (1, (80 * ps) / 9);
   this->qTableWidgetLog->horizontalHeader()->setStretchLastSection (true);
}

//------------------------------------------------------------------------------
//
void QELog::setShowColumnTime (bool pValue)
{
   this->qTableWidgetLog->setColumnHidden (0, pValue == false);
   // qTableWidgetLog->refreshSize ();
}

//------------------------------------------------------------------------------
//
bool QELog::getShowColumnTime () const
{
   return (this->qTableWidgetLog->isColumnHidden (0) == false);
}

//------------------------------------------------------------------------------
//
void QELog::setShowColumnType (bool pValue)
{
   this->qTableWidgetLog->setColumnHidden (1, pValue == false);
   // this->qTableWidgetLog->refreshSize ();
}

//------------------------------------------------------------------------------
//
bool QELog::getShowColumnType () const
{
   return (this->qTableWidgetLog->isColumnHidden (1) == false);
}

//------------------------------------------------------------------------------
//
void QELog::setShowColumnMessage (bool pValue)
{
   this->qTableWidgetLog->setColumnHidden (2, pValue == false);
   // this->qTableWidgetLog->refreshSize ();
}

//------------------------------------------------------------------------------
//
bool QELog::getShowColumnMessage () const
{
   return (this->qTableWidgetLog->isColumnHidden (2) == false);
}

//------------------------------------------------------------------------------
//
void QELog::setShowMessageFilter (bool pValue)
{
   this->qCheckBoxInfoMessage->setVisible (pValue);
   this->qCheckBoxWarningMessage->setVisible (pValue);
   this->qCheckBoxErrorMessage->setVisible (pValue);
}

//------------------------------------------------------------------------------
//
bool QELog::getShowMessageFilter () const
{
   return this->qCheckBoxInfoMessage->isVisible ();
}

//------------------------------------------------------------------------------
//
void QELog::setShowClear (bool pValue)
{
   this->qPushButtonClear->setVisible (pValue);
}

//------------------------------------------------------------------------------
//
bool QELog::getShowClear () const
{
   return this->qPushButtonClear->isVisible ();
}

//------------------------------------------------------------------------------
//
void QELog::buttonClearClicked ()
{
   int answer = QMessageBox::
                question (this, "Info", "Do you want to clear the log messages?",
                          QMessageBox::Yes, QMessageBox::No);
   if (answer == QMessageBox::Yes) {
      this->clearLog ();
   }
}

//------------------------------------------------------------------------------
//
void QELog::setShowSave (bool pValue)
{
   this->qPushButtonSave->setVisible (pValue);
}

//------------------------------------------------------------------------------
//
bool QELog::getShowSave () const
{
   return this->qPushButtonSave->isVisible ();
}

//------------------------------------------------------------------------------
//
void QELog::setOptionsLayout (optionsLayoutProperty pValue)
{
   QLayout* qLayoutMain;
   QLayout* qLayoutChild;

   delete layout ();

   switch (pValue) {
      case Top:
         this->optionsLayout = Top;
         qLayoutMain = new QVBoxLayout (this);
         qLayoutMain->setMargin (this->mainLayoutMargin);
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

      case Bottom:
         this->optionsLayout = Bottom;
         qLayoutMain = new QVBoxLayout (this);
         qLayoutMain->setMargin (this->mainLayoutMargin);
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

      case Left:
         this->optionsLayout = Left;
         qLayoutMain = new QHBoxLayout (this);
         qLayoutMain->setMargin (this->mainLayoutMargin);
         qLayoutChild = new QVBoxLayout ();
         qLayoutChild->addWidget (qCheckBoxInfoMessage);
         qLayoutChild->addWidget (qCheckBoxWarningMessage);
         qLayoutChild->addWidget (qCheckBoxErrorMessage);
         qLayoutChild->addWidget (qPushButtonClear);
         qLayoutChild->addWidget (qPushButtonSave);
         qLayoutMain->addItem (qLayoutChild);
         qLayoutMain->addWidget (qTableWidgetLog);
         break;

      case Right:
         this->optionsLayout = Right;
         qLayoutMain = new QHBoxLayout (this);
         qLayoutMain->setMargin (this->mainLayoutMargin);
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
QELog::optionsLayoutProperty QELog::getOptionsLayout () const
{
   return this->optionsLayout;
}

//------------------------------------------------------------------------------
//
void QELog::setScrollToBottom (bool pValue)
{
   this->isScrollToBottom = pValue;
   if (this->isScrollToBottom) {
      this->qTableWidgetLog->scrollToBottom ();
   }
}

//------------------------------------------------------------------------------
//
bool QELog::getScrollToBottom () const
{
   return this->isScrollToBottom;
}

//------------------------------------------------------------------------------
//
void QELog::setMaster (const bool isMasterIn)
{
   this->isMaster = isMasterIn;
   if (this->isMaster && messageReceiver) {
      messageReceiver->registerLogWidget (this);
   }
}

//------------------------------------------------------------------------------
//
bool QELog::getMaster () const
{
   return this->isMaster;
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
   this->refreshLog ();
}

//------------------------------------------------------------------------------
//
QColor QELog::getInfoColor () const
{
   return this->qColorInfo;
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
   this->refreshLog ();
}

//------------------------------------------------------------------------------
//
QColor QELog::getWarningColor () const
{
   return this->qColorWarning;
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
   this->refreshLog ();
}

//------------------------------------------------------------------------------
//
QColor QELog::getErrorColor () const
{
   return this->qColorError;
}

//------------------------------------------------------------------------------
//
void QELog::checkBoxInfoToggled (bool)
{
   this->refreshLog ();
}

//------------------------------------------------------------------------------
//
void QELog::checkBoxWarningToggled (bool)
{
   this->refreshLog ();
}

//------------------------------------------------------------------------------
//
void QELog::checkBoxErrorToggled (bool)
{
   this->refreshLog ();
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
            if (this->qTableWidgetLog->isRowHidden (i) == false) {
               if (this->getShowColumnTime ()) {
                  line = this->qTableWidgetLog->item (i, 0)->text ();
               } else {
                  line = "";
               }
               if (this->getShowColumnType ()) {
                  QString typeText = this->qTableWidgetLog->item (i, 1)->text ();
                  while (typeText.length() < 7) {
                     typeText.append(" ");
                  }

                  if (line.isEmpty ()) {
                     line = typeText;
                  } else {
                     line += ", " + typeText;
                  }
               }
               if (this->getShowColumnMessage ()) {
                  if (line.isEmpty ()) {
                     line = this->qTableWidgetLog->item (i, 2)->text ();
                  } else {
                     line += ", " + this->qTableWidgetLog->item (i, 2)->text ();
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
   this->qTableWidgetLog->setRowCount (0);
   this->qPushButtonClear->setEnabled (false);
   this->qPushButtonSave->setEnabled (false);
}

//------------------------------------------------------------------------------
//
void QELog::addLog (int pType, QString pMessage,
                    const QDateTime& dateTime)
{
   QTableWidgetItem *qTableWidgetItem;
   QString type;
   QColor color;

   switch (pType) {
      case MESSAGE_TYPE_INFO:
         type = "INFO";
         color = this->qColorInfo;
         break;

      case MESSAGE_TYPE_WARNING:
         type = "WARNING";
         color = this->qColorWarning;
         break;

      case MESSAGE_TYPE_ERROR:
         type = "ERROR";
         color = this->qColorError;
         break;

      default:
         DEBUG << "unexpected message type:" << int (pType);
         return;
   }

   int i = this->qTableWidgetLog->rowCount ();
   this->qTableWidgetLog->insertRow (i);
   if (type == "INFO") {
      qTableWidgetLog->setRowHidden (i, !this->qCheckBoxInfoMessage->isChecked ());
   } else if (type == "WARNING") {
      qTableWidgetLog->setRowHidden (i, !this->qCheckBoxWarningMessage->isChecked ());
   } else {
      qTableWidgetLog->setRowHidden (i, !this->qCheckBoxErrorMessage->isChecked ());
   }

   qTableWidgetItem =
         new QTableWidgetItem (dateTime.toString ("yyyy/MM/dd - hh:mm:ss"));
   qTableWidgetItem->setTextColor (color);
   this->qTableWidgetLog->setItem (i, 0, qTableWidgetItem);

   qTableWidgetItem = new QTableWidgetItem (type);
   qTableWidgetItem->setTextColor (color);
   this->qTableWidgetLog->setItem (i, 1, qTableWidgetItem);

   qTableWidgetItem = new QTableWidgetItem (pMessage);
   qTableWidgetItem->setTextColor (color);
   this->qTableWidgetLog->setItem (i, 2, qTableWidgetItem);

   if (this->isScrollToBottom) {
      this->qTableWidgetLog->scrollToBottom ();
   }

   this->qPushButtonClear->setEnabled (true);
   this->qPushButtonSave->setEnabled (true);
}

//------------------------------------------------------------------------------
//
void QELog::refreshLog ()
{
   QColor color;

   for (int i = 0; i < this->qTableWidgetLog->rowCount (); i++) {

      QTableWidgetItem* qTableWidgetItem = this->qTableWidgetLog->item (i, 1);

      if (qTableWidgetItem->text () == "INFO") {
         this->qTableWidgetLog->setRowHidden (i, !this->qCheckBoxInfoMessage->isChecked ());
         color = this->qColorInfo;
      } else if (qTableWidgetItem->text () == "WARNING") {
         this->qTableWidgetLog->setRowHidden (i, !this->qCheckBoxWarningMessage->isChecked ());
         color = this->qColorWarning;
      } else {
         this->qTableWidgetLog->setRowHidden (i, !this->qCheckBoxErrorMessage->isChecked ());
         color = this->qColorError;
      }

      qTableWidgetItem->setTextColor (color);

      qTableWidgetItem = this->qTableWidgetLog->item (i, 0);
      qTableWidgetItem->setTextColor (color);

      qTableWidgetItem = this->qTableWidgetLog->item (i, 2);
      qTableWidgetItem->setTextColor (color);
   }
}

//------------------------------------------------------------------------------
//
void QELog::setMargin (int marginIn)
{
   this->mainLayoutMargin = LIMIT (marginIn, -1, 32);
   QLayout* layout = this->layout();
   if (layout) {
      layout->setMargin (this->mainLayoutMargin);
   }
}

//------------------------------------------------------------------------------
//
int QELog::getMargin () const
{
   return this->mainLayoutMargin;
}

//------------------------------------------------------------------------------
//
QELog::MessageFilterOptions QELog::getMessageFormFilter () const
{
   return MessageFilterOptions (this->getFormFilter ());
}

//------------------------------------------------------------------------------
//
void QELog::setMessageFormFilter (MessageFilterOptions messageFormFilter)
{
   this->setFormFilter (message_filter_options (messageFormFilter));
}

//------------------------------------------------------------------------------
//
QELog::MessageFilterOptions QELog::getMessageSourceFilter () const
{
   return MessageFilterOptions (this->getSourceFilter ());
}

//------------------------------------------------------------------------------
//
void QELog::setMessageSourceFilter (MessageFilterOptions messageSourceFilter)
{
   this->setSourceFilter (message_filter_options (messageSourceFilter));
}

//------------------------------------------------------------------------------
// Receive new log messages from other QEWidgets.
//
void QELog::newMessage (QString msg, message_types type)
{
   // Add the message to the log - time stamp now
   //
   if ((type.kind_set & MESSAGE_KIND_EVENT) != 0) {
      this->addLog (type.severity, msg, QDateTime::currentDateTime ());
   }
}

//------------------------------------------------------------------------------
//
void QELog::processMessage (const QString& msg,
                            const message_types type,
                            const QDateTime& dateTime)
{
   // Add the message to the log - use saved time stamp
   //
   if ((type.kind_set & MESSAGE_KIND_EVENT) != 0) {
      this->addLog (type.severity, msg, dateTime);
   }
}

// end
