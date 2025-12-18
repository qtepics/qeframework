/*  QEDelayedText.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2014-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_DELAYED_TEXT_H
#define QE_DELAYED_TEXT_H

#include <QObject>
#include <QString>
#include <QTimer>
#include <QEFrameworkLibraryGlobal.h>

/// Provides a delayed text signal, noy unlike that employed by QCaVariableNamePropertyManager
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEDelayedText : public QObject {
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
