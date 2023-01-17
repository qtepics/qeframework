/*  QECommon.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2023 Australian Synchrotron.
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
 *
 */

#include "QECommon.h"

#include <math.h>
#include <iostream>

#include <QApplication>
#if QT_VERSION < 0x060000
#include <QDesktopWidget>
#endif
#include <QDir>
#include <QLayout>
#include <QtGlobal>
#include <QColor>
#include <QDebug>
#include <QFileInfo>
#include <QMainWindow>
#include <QMetaEnum>
#include <QMetaObject>
#include <QPoint>
#include <QRegularExpression>
#include <QSize>
#include <QScreen>
#include <QWidget>
#include <QEPlatform.h>
#include <QEWidget.h>
#include <QELabel.h>

#define DEBUG qDebug () << "QECommon" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
QColor QEUtilities::fontColour (const QColor& backgroundColour)
{
   QColor result;
   int r, g, b, a;

   // Split colour into components.
   //
   backgroundColour.getRgb (&r, &g, &b, &a);

   // Counting the perceptive luminance - human eye favors green color...
   // Form weighted component wc. Weights based on algorithm:
   // http://stackoverflow.com/questions/1855884/determine-font-color-based-on-background-color
   //
   const int wc = ((299 * r) + (587 * g) + (114 * b)) / 1000;   // 299 + 587 + 114 = 1000

   // Dark or bright background colour ?
   //
   const bool isDark = (wc < 124);
   if (isDark) {
      result = QColor (255, 255, 255, 255);    // white font
   } else {
      result = QColor (0, 0, 0, 255);          // black font
   }

   return result;
}

//------------------------------------------------------------------------------
//
QString  QEUtilities::colourToStyle (const QColor& backgroundColour,
                                     const QColor& foregroundColour)
{
   QString result;
   int br, bg, bb, ba;
   int fr, fg, fb, fa;

   // Split colours into components and aggragate into a style sheet.
   //
   backgroundColour.getRgb (&br, &bg, &bb, &ba);
   foregroundColour.getRgb (&fr, &fg, &fb, &fa);

   result = QString::asprintf ("QWidget { background-color: rgba(%d, %d, %d, %d);"
                               " color: rgba(%d, %d, %d, %d); }",
                               br, bg, bb, ba, fr, fg, fb, fa );
   return result;
}

//------------------------------------------------------------------------------
//
QString QEUtilities::colourToStyle (const QColor& backgroundColour)
{
   // Choose high contrast font/foreground colour.
   //
   QColor foregroundColour = QEUtilities::fontColour (backgroundColour);
   return QEUtilities::colourToStyle (backgroundColour, foregroundColour);
}

//------------------------------------------------------------------------------
//
QString QEUtilities::offBackgroundStyle ()
{
   // Standard background on Linux is #d6d2d0. Add #0a0a0a
   // Windows may be a bit different.
   //
   return QEUtilities::colourToStyle (QColor (0xe0dcda));
}

//------------------------------------------------------------------------------
//
QColor QEUtilities::darkColour (const QColor& lightColour)
{
   QColor result;
   int h, s, l, a;

   lightColour.getHsl (&h, &s, &l, &a);

   // 33.3% lightness
   //
   l = (2*l) / 3;

   result.setHsl (h, s, l, a);
   return result;
}

//------------------------------------------------------------------------------
//
QColor QEUtilities::blandColour (const QColor& vibrantColour)
{
   QColor result;
   int r, g, b, a;

   vibrantColour.getRgb (&r, &g, &b, &a);

   // Move 87.5% of the way towards light gray - #c8c8c8.
   //
   r = (7*200 + r) / 8;
   g = (7*200 + g) / 8;
   b = (7*200 + b) / 8;

   result.setRgb (r, g, b, a);
   return result;
}

//------------------------------------------------------------------------------
//
int QEUtilities::roundToInt (const double x, bool* ok)
{
   const double dimin = (double) INT_MIN;
   const double dimax = (double) INT_MAX;

   int result;
   bool okay;

   if (x < dimin) {
      // Too small
      //
      result = INT_MIN;
      okay = false;

   } else if (x > dimax) {
      // Tooo big
      //
      result = INT_MAX;
      okay = false;

   } else {
      // In range - conversion possible.
      //
      if (x >= 0.0) {
         // Conversion to int truncates towards 0, must treat positive and
         // negative numbers differently.
         //
         result = (int)(x + 0.5);
      } else {
         result = (int)(x - 0.5);
      }
      okay = true;
   }

   if (ok) *ok = okay;
   return result;
}

//------------------------------------------------------------------------------
//
QString QEUtilities::intervalToString (const double interval,
                                       const int precision,
                                       const bool showDays)
{
   QString result;
   double seconds;
   QString sign;
   QString image;
   QString fraction;
   int days;
   int hours;
   int mins;
   int secs;
   int nanoSecs;

   if (interval >= 0.0) {
      seconds = +interval;
      sign = "";
   } else {
      seconds = -interval;
      sign = "-";
   }

#define EXTRACT(item, spi) { item = int (floor (seconds / spi)); seconds = seconds - (spi * item); }

   EXTRACT (days, 86400.0);
   EXTRACT (hours, 3600.0);
   EXTRACT (mins, 60.0);
   EXTRACT (secs, 1.0);
   EXTRACT (nanoSecs, 1.0E-9);

#undef EXTRACT

   if (showDays || (days != 0)) {
      image = QString::asprintf ("%d %02d:%02d:%02d", days, hours, mins, secs);
   } else {
      image = QString::asprintf ("%02d:%02d:%02d", hours, mins, secs);
   }

   if (precision > 0) {
      // Limit precision to 9.
      //
      fraction = QString::asprintf (".%09d", nanoSecs);
      fraction.truncate (MIN (9, precision) + 1);
   } else {
      fraction = "";
   }

   result = sign.append (image).append (fraction);
   return result;
}

//------------------------------------------------------------------------------
//
int QEUtilities::getTimeZoneOffset (const QDateTime& atTime)
{
   QDateTime local = atTime.toLocalTime ();

   // This changes the time, e.g. from "2013-02-24 11:37:19 EST" to "2013-02-24 11:37:19 UTC" which
   // has same numbers but is not the same time in an absolute sense (with apologies to Einstein).
   //
   local.setTimeSpec (Qt::UTC);

   // The "same" time in different time zones is a different time.
   // Use that difference to determine the local time offset from UTC.
   //
   return atTime.secsTo (local);
}

//------------------------------------------------------------------------------
//
QString QEUtilities::getTimeZoneTLA (const Qt::TimeSpec timeSpec, const QDateTime & atTime)
{
   QString result;

   switch (timeSpec) {
      case Qt::UTC:
         result = "UTC";
         break;

      case Qt::LocalTime:
#ifdef _XOPEN_SOURCE
#include <time.h>
         {
            // Get offset and convert to hours
            //
            int actualOffset;
            actualOffset = QEUtilities::getTimeZoneOffset (atTime);

            // Ensure zone information initialised.
            //
            tzset ();

            // timezone is seconds West of GMT, whereas actualOffset is seconds East,
            // hence the negation in the equality test.
            //
            if ((actualOffset == -timezone) || (daylight == 0) ) {
               result = tzname [0];
            } else {
               // offsets not equal and daylight available - use it.
               //
               result = tzname [1];
            }
         }
#else
         // Not sure what Windows has to offer (yet).
         //
         result = "???";
#endif
         break;

      case Qt::OffsetFromUTC:    // Don't know what to do here,
      default:                   // or here - both unexpected;
         result = "ERR";
         break;
   }
   return result;
}

//------------------------------------------------------------------------------
//
QString QEUtilities::getTimeZoneTLA (const QDateTime & atTime)
{
   return QEUtilities::getTimeZoneTLA (atTime.timeSpec(), atTime);
}

//------------------------------------------------------------------------------
//
QString QEUtilities::enumToString (const QObject& object,
                                   const QString& enumTypeName,
                                   const int enumValue)
{
   const QMetaObject mo = *object.metaObject();

   return enumToString(mo, enumTypeName, enumValue);
}

QString QEUtilities::enumToString(const QMetaObject& mo,
                                  const QString& enumTypeName,
                                  const int enumValue)
{
   QString result;

   for (int e = 0; e < mo.enumeratorCount(); e++) {
      QMetaEnum metaEnum = mo.enumerator(e);
      if (metaEnum.isValid () && metaEnum.name () == enumTypeName) {
         // found it.
         //
         result = metaEnum.valueToKey (enumValue);
         break;
      }
   }

   return result;
}

//------------------------------------------------------------------------------
//
int QEUtilities::stringToEnum (const QObject& object,
                               const QString& enumTypeName,
                               const QString& enumImage,
                               bool* ok)
{
   const QMetaObject mo = *object.metaObject();

   return stringToEnum (mo, enumTypeName, enumImage, ok);
}

//------------------------------------------------------------------------------
//
int QEUtilities::stringToEnum (const QMetaObject& mo,
                               const QString& enumTypeName,
                               const QString& enumImage,
                               bool* ok)
{
   int result = -1;
   bool okay = false;

   for (int e = 0; e < mo.enumeratorCount(); e++) {
      QMetaEnum metaEnum = mo.enumerator(e);
      if (metaEnum.isValid () && metaEnum.name () == enumTypeName) {
         // found it.
         //
         result = metaEnum.keyToValue (enumImage.trimmed ().toLatin1 ().data ());
         if (result != (-1)) {
            // This is a good value.
            okay = true;
         } else {
            okay = false;  // hypothosize not okay
            for (int i = 0; i < metaEnum.keyCount (); i++) {
               if (result == metaEnum.value (i)) {
                  // This was a valid value after all.
                  //
                  okay = true;
                  break;
               }
            }
         }
         break;
      }
   }

   if (ok) *ok = okay;
   return result;
}

//------------------------------------------------------------------------------
//
QStringList QEUtilities::split (const QString& s)
{
   QStringList result;
   QRegularExpression re ("\\s+", QRegularExpression::NoPatternOption);
   result = s.split (re, QESkipEmptyParts);
   return result;
}

//------------------------------------------------------------------------------
//
QStringList QEUtilities::variantToStringList (const QVariant& v)
{
   QStringList result;
   QStringList vlist;

   result.clear ();
   vlist = v.toStringList ();  // Do variant split
   for (int v = 0; v < vlist.count(); v++) {
      const QString s = vlist.value (v);
      const QStringList slist = QEUtilities::split (s); // Do white space split
      result.append (slist);
   }

   return result;
}

//------------------------------------------------------------------------------
//
QWidget* QEUtilities::findWidget (QWidget* parent, const QString& className)
{
   const QMetaObject* meta = parent->metaObject ();

   QWidget* result = NULL;
   int j, n;
   QObjectList childList;
   QObject *child = NULL;
   QWidget *childWidget = NULL;

   if (meta->className () == className) {
      // found it - return immediatley
      return parent;
   }

   childList = parent->children ();
   n = childList.count();
   for (j = 0; j < n; j++) {
      child = childList.value (j);
      // We need only tree walk widgets. All widget parents are themselves widgets.
      //
      childWidget = dynamic_cast <QWidget *>(child);
      if (childWidget) {
         // Recursive call.
         //
         result = QEUtilities::findWidget (childWidget, className);
         if (result) break;
      }
   }

   return result;
}

//------------------------------------------------------------------------------
// static
// cribbed from kubili/KDM
void QEUtilities::listPVNames (QWidget* rootWidget,
                               const QString& targetFile,
                               const QString& comment)
{
   if (targetFile.isEmpty ()) {
      qDebug () << "QEUtilities::listPVNames - no filename specified";
      return;
   }

   QFile file (targetFile);
   if (!file.open (QIODevice::WriteOnly | QIODevice::Text)) {
      qDebug () << targetFile << " file open (write) failed";
      return;
   }

   QTextStream target (&file);

   target << "# " << comment << "\n";
   target << "#\n";
   target << "\n";

   QEWidgetList list = QEUtilities::findAllQEWidgets (rootWidget);

   const int number = list.count ();
   for (int j = 0; j < number; j++) {
      QEWidget* item = list.value (j, NULL);
      if (item) {
         int m = item->getNumberVariables ();
         for (int j = 0; j < m; j++) {
            QString pvName = item->getSubstitutedVariableName ((unsigned int) j);
            if (!pvName.isEmpty()) {
               target << pvName << "\n";
            }
         }
      }
   }

   target << "\n";
   target << "# end\n";
   file.close ();
}

//------------------------------------------------------------------------------
// static - cribbed from kubili and simplified.
//
void QEUtilities::activate (QWidget* rootWidget)
{
   QEWidgetList list = findAllQEWidgets (rootWidget);

   const int number = list.count ();
   for (int j = 0; j < number; j++) {
      QEWidget* item = list.value (j, NULL);
      if (item) {
         QWidget* widget = item->getQWidget ();
         if (widget) {
            QELabel* label = dynamic_cast <QELabel*> (widget);
            if (label) {
               // Once the PV connects, text value and style will be set up.
               //
               label->setText ("");
               label->setStyleSheet (label->getStyleDefault ());
            }
            item->activate ();
         }
      }
   }
}

//------------------------------------------------------------------------------
// static - cribbed from kubili and simplified.
//
void QEUtilities::deactivate (QWidget* rootWidget)
{
   QEWidgetList list = findAllQEWidgets (rootWidget);

   const int number = list.count ();
   for (int j = 0; j < number; j++) {
      QEWidget* item = list.value (j, NULL);
      if (item) {
         item->deactivate ();
      }
   }
}

//------------------------------------------------------------------------------
// static
//
QString QEUtilities::dirName (const QString& pathName)
{
   QFileInfo fileInfo (pathName);
   QDir dir = fileInfo.dir ();
   QString result = dir.path ();
   return result;
}

//------------------------------------------------------------------------------
// static
//
void QEUtilities::treeWalkAndAppend (QObject* item, QEWidgetList& list)
{
   // sainity check.
   if (!item) return;

   // First - is this item a QEWidget?
   //
   QEWidget* qewidget = dynamic_cast <QEWidget *>(item);
   if (qewidget) {
      list.append (qewidget);    // Yes - add to the list
   }

   // Next examine any/all children.
   //
   QObjectList childList = item->children ();
   const int n = childList.count();
   for (int j = 0; j < n; j++) {
      QObject* child = childList.value (j);
      // We need only tree walk widgets.
      //
      QWidget* widget = dynamic_cast <QWidget *>(child);
      if (widget) {
         treeWalkAndAppend (widget, list);   // Note: recursive.
      }
   }
}

//------------------------------------------------------------------------------
// static
QEUtilities::QEWidgetList QEUtilities::findAllQEWidgets (QWidget* rootWidget)
{
   QEWidgetList list;

   list.clear ();
   QEUtilities::treeWalkAndAppend (rootWidget, list);
   return list;
}

//------------------------------------------------------------------------------
// Same again
// static
void QEUtilities::treeWalkAndAppend (QObject* item, QWidgetList& list)
{
   // sainity check.
   if (!item) return;

   // First - is this item a QEWidget?
   //
   QWidget* qwidget = dynamic_cast <QWidget *>(item);
   if (qwidget) {
      list.append (qwidget);    // Yes - add to the list
   }

   // Next examine any/all children.
   //
   QObjectList childList = item->children ();
   const int n = childList.count();
   for (int j = 0; j < n; j++) {
      QObject* child = childList.value (j);
      // We need only tree walk widgets.
      //
      QWidget* widget = dynamic_cast <QWidget *>(child);
      if (widget) {
         treeWalkAndAppend (widget, list);   // Note: recursive.
      }
   }
}

//------------------------------------------------------------------------------
// static
QWidgetList QEUtilities::findAllQWidgets (QWidget* rootWidget)
{
   QWidgetList list;

   list.clear ();
   QEUtilities::treeWalkAndAppend (rootWidget, list);
   return list;
}

//------------------------------------------------------------------------------
// static
QMainWindow* QEUtilities::findMainWindow (QWidget* widget)
{
   // Find containing main form window.
   //
   QWidget* w = widget;
   QMainWindow* m = qobject_cast <QMainWindow*> (w);
   while (w && !m) {
      // There is a parent and it is not a main window - ascend one level.
      //
      w = w->parentWidget();
      m = qobject_cast <QMainWindow*> (w);
   }
   return m;
}

//------------------------------------------------------------------------------
// static
void QEUtilities::setWindowTitle (QWidget* widget, const QString& title)
{
   QMainWindow* m = QEUtilities::findMainWindow (widget);
   if (m) {
      m->setWindowTitle (title);
   }
}

//------------------------------------------------------------------------------
// static
QString QEUtilities::getWindowTitle (QWidget* widget)
{
   QString result = "";
   QMainWindow* m = QEUtilities::findMainWindow (widget);
   if (m) {
      result = m->windowTitle ();
   }
   return result;
}

//------------------------------------------------------------------------------
// static
QRect QEUtilities::desktopGeometry ()
{
   QRect result;
#if QT_VERSION < 0x060000
   // Qt5 - cribbed from QEDialog
   //
   const QRect screen = QApplication::desktop ()->screenGeometry (0);
   int stop = screen.top ();
   int sleft = screen.left ();
   int sright = screen.right ();
   int sbottom = screen.bottom ();

   // We assume screens are in a regular block, e.g. 1x1, 1x4, 2x2, 2x1 etc.
   //
   for (int j = 1; j < QApplication::desktop ()->screenCount(); j++) {
      const QRect s2 = QApplication::desktop ()->screenGeometry (j);
      stop    = MIN (stop,    s2.top());
      sleft   = MIN (sleft,   s2.left());
      sright  = MAX (sright,  s2.right());
      sbottom = MAX (sbottom, s2.bottom());
   }
   result = QRect (sleft, stop, sright - sleft, sbottom - stop);
#else
   // Qt6++
   QRegion virtualGeometry;
   for (auto screen : QGuiApplication::screens()) {
      virtualGeometry += screen->geometry();
   }
   result = virtualGeometry.boundingRect();
#endif
   return result;
}

//------------------------------------------------------------------------------
// static
QRect QEUtilities::screenGeometry (QWidget* widget)
{
   // If no widget is specified, go with the primary screen as default.
   //
   if (!widget) return QApplication::primaryScreen()->geometry();

   QPoint pos = widget->geometry().topLeft();   // should we use the centre?
   QPoint golbalPos;

   // In order to calculate the global position correctly, we
   // need to know if the widget is a main window.
   //
   QMainWindow* mainCheck = qobject_cast <QMainWindow*> (widget);
   if (mainCheck) {
      // Is a main window - just use the position as is.
      //
      golbalPos = pos;
   } else {
      // Regular widget - map it.
      //
      golbalPos = widget->mapToGlobal (pos);
   }

   QScreen* screen = QApplication::screenAt (golbalPos);

   // If we can't identify a screen, go with the default.
   //
   if (!screen) return QApplication::primaryScreen()->geometry();

   return screen->geometry();
}

//------------------------------------------------------------------------------
// static
void QEUtilities::debugWidgetHierarchy (const QWidget* root,
                                        const int instance,
                                        const int level)
{
   if (!root) return;

   QString gap = "";
   for (int j = 0; j < level; j++) {
      gap.append ("  ");
   }

   QString b1;       // basic class info
   QString b2;       // sizing info
   QString b3 = "";  // layout info

   b1 = QString::asprintf ("%d.%-2d%s %s:%s", level, instance,
                           gap.toStdString().c_str(),
                           root->objectName ().toStdString().c_str(),
                           root->metaObject()->className ());

   b2 = QString::asprintf (" (%3d,%4d %3dx%3d)  (%3dx%3d)  (%3dx%3d)",
                           root->geometry().x(), root->geometry().y(),
                           root->size().width(), root->size().height(),
                           root->minimumWidth(), root->minimumHeight(),
                           root->maximumWidth(), root->maximumHeight());

   QLayout* lay = root->layout();
   if (lay) {
      b3 = QString::asprintf ("  %s:%s",
                              lay->objectName ().toStdString().c_str(),
                              lay->metaObject()->className ());
   }

   QString buffer;
   buffer = QString::asprintf ("%-52s %-42s %s\n",
                               b1.toStdString().c_str(),
                               b2.toStdString().c_str(),
                               b3.toStdString().c_str());

   std::cout << buffer.toStdString().c_str();

   QObjectList objList = root->children ();
   for (int j = 0; j < objList.count (); j++) {
      QObject* child = objList.value (j, NULL);
      if( child && child->isWidgetType ()) {
         const QWidget* w = (QWidget*) child;
         QEUtilities::debugWidgetHierarchy (w, j, level + 1);
      }
   }
}

// end
