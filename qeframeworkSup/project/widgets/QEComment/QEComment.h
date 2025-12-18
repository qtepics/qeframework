/*  QEComment.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2025-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_COMMENT_H
#define QE_COMMENT_H

#include <QString>
#include <QLabel>
#include <QWidget>
#include <QCaVariableNamePropertyManager.h>
#include <QEFrameworkLibraryGlobal.h>

/// This widget does nothing per say, save hold a comment string available within
/// designer.
/// Note: the widget itself non-visible by default.
///
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEComment :
   public QWidget
{
   Q_OBJECT

public:
   Q_PROPERTY (QString comment     READ getComment     WRITE setComment)

   /// Not sure how usefull it would be to have this widget visible at run time,
   /// but let's not second guess the users' needs and desires.
   Q_PROPERTY (bool runVisible     READ getRunVisible  WRITE setRunVisible)

public:
   explicit QEComment (QWidget* parent = 0);
   ~QEComment();

   // Property access functions.
   //
   void setComment(const QString& text);
   QString getComment() const;

   void setRunVisible (const bool flag);
   bool getRunVisible() const;

protected:
   QSize sizeHint ();
   void paintEvent (QPaintEvent* event);   // draw something as design time

private:
   QString commentText;
   bool isRunVisible;
};

#endif // QE_COMMENT_H
