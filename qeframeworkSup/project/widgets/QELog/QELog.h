/*  QELog.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2024 Australian Synchrotron
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

#ifndef QE_LOG_H
#define QE_LOG_H

#include <QWidget>
#include <QCheckBox>
#include <QTableWidget>
#include <QPushButton>
#include <QEEnums.h>
#include <QEFrame.h>
#include <QEWidget.h>
#include <QEFrameworkLibraryGlobal.h>

/**
 * The QELog class provides the means to display messages to the user.
 */
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QELog : public QEFrame
{
   Q_OBJECT

public:
   Q_PROPERTY (bool showColumnTime    READ getShowColumnTime    WRITE setShowColumnTime)
   Q_PROPERTY (bool showColumnType    READ getShowColumnType    WRITE setShowColumnType)
   Q_PROPERTY (bool showColumnMessage READ getShowColumnMessage WRITE setShowColumnMessage)
   Q_PROPERTY (bool showMessageFilter READ getShowMessageFilter WRITE setShowMessageFilter)
   Q_PROPERTY (bool showClear         READ getShowClear         WRITE setShowClear)
   Q_PROPERTY (bool showSave          READ getShowSave          WRITE setShowSave)
   Q_PROPERTY (bool scrollToBottom    READ getScrollToBottom    WRITE setScrollToBottom)
   Q_PROPERTY (bool master            READ getMaster            WRITE setMaster)

   /// Provides the location of filter check boxes and save/clear buttons.
   ///
   Q_PROPERTY (QE::LayoutOptions optionsLayout READ getOptionsLayout
               WRITE setOptionsLayout)
   Q_PROPERTY (int    margin          READ getMargin            WRITE setMargin)

   Q_PROPERTY (QColor infoColor       READ getInfoColor         WRITE setInfoColor)
   Q_PROPERTY (QColor warningColor    READ getWarningColor      WRITE setWarningColor)
   Q_PROPERTY (QColor errorColor      READ getErrorColor        WRITE setErrorColor)

   /// Message properties
   /// Not all QE widgets need these properties (only those that do something with
   /// messages - like the logging widget)
   ///
   Q_PROPERTY (QE::MessageFilterOptions messageFormFilter READ getMessageFormFilter
               WRITE setMessageFormFilter)

   Q_PROPERTY (QE::MessageFilterOptions messageSourceFilter READ getMessageSourceFilter
               WRITE setMessageSourceFilter)

public:
   class UserMessageReceiver;  // differed - private
   friend class UserMessageReceiver;

   // construct a non-QELog message receiver - relevent when setMaster true.
   //
   static void createUserMessageReceiver ();

   explicit QELog (QWidget* parent = 0);
   virtual ~QELog ();

   void setShowColumnTime (bool pValue);
   bool getShowColumnTime () const;

   void setShowColumnType (bool pValue);
   bool getShowColumnType () const;

   void setShowColumnMessage (bool pValue);
   bool getShowColumnMessage () const;

   void setShowMessageFilter (bool pValue);
   bool getShowMessageFilter () const;

   void setShowClear (bool pValue);
   bool getShowClear () const;

   void setShowSave (bool pValue);
   bool getShowSave () const;

   void setOptionsLayout (QE::LayoutOptions pValue);
   QE::LayoutOptions getOptionsLayout () const;

   void setScrollToBottom (bool pValue);
   bool getScrollToBottom () const;

   void setMaster (const bool isMaster);
   bool getMaster () const;

   void setInfoColor (QColor pValue);
   QColor getInfoColor () const;

   void setWarningColor (QColor pValue);
   QColor getWarningColor () const;

   void setErrorColor (QColor pValue);
   QColor getErrorColor () const;

   void setMargin (int margin);
   int getMargin () const;

   void setMessageFormFilter (QE::MessageFilterOptions messageFormFilter);
   QE::MessageFilterOptions getMessageFormFilter () const;

   void setMessageSourceFilter (QE::MessageFilterOptions messageSourceFilter);
   QE::MessageFilterOptions getMessageSourceFilter () const;

   void clearLog ();

   void addLog (int pType, QString pMessage,
                const QDateTime& dateTime);

   void refreshLog ();

protected:
   void resizeEvent (QResizeEvent* resizeEvent);
   void newMessage (QString msg, message_types type);

private:
   QTableWidget* qTableWidgetLog;
   QCheckBox* qCheckBoxInfoMessage;
   QCheckBox* qCheckBoxWarningMessage;
   QCheckBox* qCheckBoxErrorMessage;
   QPushButton* qPushButtonClear;
   QPushButton* qPushButtonSave;
   QColor qColorInfo;
   QColor qColorWarning;
   QColor qColorError;
   bool isScrollToBottom;
   bool isMaster;
   QE::LayoutOptions optionsLayout;
   int mainLayoutMargin;

   void processMessage (const QString& msg,
                        const message_types type,
                        const QDateTime& dateTime);

private slots:
   void checkBoxInfoToggled (bool);
   void checkBoxWarningToggled (bool);
   void checkBoxErrorToggled (bool);
   void buttonClearClicked ();
   void buttonSaveClicked ();
};

#endif   // QE_LOG_H
