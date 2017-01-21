/*  QEDelayedText.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
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
 *  Copyright (c) 2014 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_DELAYED_TEXT_H
#define QE_DELAYED_TEXT_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QEPluginLibrary_global.h>

/// Provides a delayed text signal, noy unlike that employed by QCaVariableNamePropertyManager
///
class QEPLUGINLIBRARYSHARED_EXPORT QEDelayedText : public QObject {
   Q_OBJECT

public:
   explicit QEDelayedText (const double delay, QObject* parent = 0);
   virtual ~QEDelayedText ();

   // Performs connect sender/signal to this setText and connects this textChanged
   // to receiver/member.
   //
   bool doubleConnect (const QObject* sender,   const char* signal,
                       const QObject* receiver, const char* member) const;

signals:
   void textChanged (const QString &);

public slots:
   void setText (const QString &);

private:
   QTimer* timer;
   double delay;
   QString text;

private slots:
   void timeout ();
};

#endif   // QE_DELAYED_TEXT_H
