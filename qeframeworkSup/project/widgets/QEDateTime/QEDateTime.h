/*  QEDataTime.h
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
 *    Zai Wang
 *  Contact details:
 *    @ansto.gov.au
 */

#ifndef QE_DATETIME_H
#define QE_DATETIME_H

#include <QAction>
#include <QEvent>
#include <QLabel>
#include <QMenu>
#include <QDateTime>
#include <QPoint>
#include <QSize>
#include <QTimer>
#include <QEFrameworkLibraryGlobal.h>

/*!
  This class is a non EPICS aware label widget based on the Qt label widget and Qt DateTime class.
  It is only used for displaying data and time of the day.
 */
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEDateTime : public QLabel
{
   Q_OBJECT

   Q_PROPERTY(QString dateTimeFormat READ getDateTimeFormat WRITE setDateTimeFormat)
   Q_PROPERTY(Qt::TimeSpec timeZone  READ getTimeZone       WRITE setTimeZone)
   Q_PROPERTY(bool showZone          READ getShowZone       WRITE setShowZone)

public:
   explicit QEDateTime(QWidget *parent = 0);
   ~QEDateTime();

   QString getDateTimeFormat() const;
   void setDateTimeFormat(const QString format);

   Qt::TimeSpec getTimeZone() const;
   void setTimeZone(const Qt::TimeSpec zone);

   bool getShowZone() const;
   void setShowZone(const bool showZone);

protected:
   QSize sizeHint() const;
   bool eventFilter(QObject* watched, QEvent *event);

private:
   QString dateTimeFormat;
   Qt::TimeSpec timeZone;
   bool showZone;

   static QTimer* tickTimer;
   QMenu* contextMenu;

private slots:
   void kick();
   void customContextMenuRequested(const QPoint& pos);
   void contextMenuTriggered(QAction* action);
};

#endif // QE_DATETIME_H
