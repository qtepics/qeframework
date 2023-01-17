/*  QEScaling.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2022 Australian Synchrotron
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

#include "QEScaling.h"

#include <QDebug>
#include <QLabel>
#include <QLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QRegularExpression>
#include <QSize>
#include <QTableWidget>
#include <QTreeView>
#include <QWidget>
#include <QVariantList>

#include <QEResizeableFrame.h>
#include <QSimpleShape.h>
#include <QEWidget.h>
#include <QECommon.h>

#define DEBUG qDebug () << "QEScaling" << __LINE__ << __FUNCTION__ << "  "


//==============================================================================
// Local function
//==============================================================================
// Euclidean algorithm for computing greatest common divisor of two integers.
// Ref: Hand Book of Applied Crytpgraphy (1997), p66, sec 2.104
//
static int gcd (int a, int b)
{
   int r;

   while (b) {
      r = a % b;
      a = b;
      b = r;
   }
   return a;
}

//==============================================================================
// QEScaling
//==============================================================================
//
#define MAGIC_NUMBER        0x23571113
#define BASELINE_SIZEING    "__QE_BASELINE_SIZEING__"
#define CURRENT_SCALE       "__QE_CURRENT_SCALE__"

// The baseline sizing information is stored as a dynamic widget property.
// The stored porprty is a QVariantList with two elements, the enum type names
// the indicies associated with each QVariant in the list.
//
enum PropertyIndex {
   piStyleSheet = 0,
   piGenericData,
   piNUMBER_OF_ITEMS       // MUST be last
};


//------------------------------------------------------------------------------
//
QEScaling::QEScaling ()
{
   this->firstMember = MAGIC_NUMBER;
   this->isDefined = false;
   this->layoutIsDefined = false;
   this->indent = -1;
   this->resizeFrameAllowedMin = -1;
   this->resizeFrameAllowedMax = -1;
   this->tableDefaultHorizontalSectionSize  = -1;
   this->tableDefaultVerticalSectionSize  = -1;
}

//------------------------------------------------------------------------------
//
QEScaling::~QEScaling ()
{
   this->firstMember = !MAGIC_NUMBER;
}

//------------------------------------------------------------------------------
//
void QEScaling::captureBaselineInformation (QWidget* widget)
{
   if (!widget) return;  // sainity check.

   this->extractFromWidget (widget);
   QVariant property = this->encodeProperty ();
   widget->setProperty (BASELINE_SIZEING, property);
}

//------------------------------------------------------------------------------
//
bool QEScaling::extractBaselineInformation (const QWidget* widget)
{
   if (!widget) return false;  // sainity check.
   QVariant property = widget->property (BASELINE_SIZEING);
   bool result = this->decodeProperty (property);
   return result;
}

//------------------------------------------------------------------------------
//
int QEScaling::dataSize () const
{
   return int (size_t (&this->lastMember) - size_t (&this->firstMember));
}

//------------------------------------------------------------------------------
// Encodes from firstMember upto, but excluding, lastMember as QByteArray QVariant.
//
QVariant QEScaling::encodeProperty () const
{
   if (!isDefined) {
      return QVariant (QVariant::Invalid);
   }

   const void* base = &this->firstMember;
   const int size = this->dataSize ();

   QByteArray data ((char *)base, size);

   QVariantList result;
   result.append (QVariant (this->styleSheet));
   result.append (QVariant (data));
   return result;
}

//------------------------------------------------------------------------------
//
bool QEScaling::decodeProperty (const QVariant& property)
{
   bool result = false;

   this->isDefined = false;

   if (property.type () != QVariant::List) return false;

   QVariantList variantList = property.toList ();
   if (variantList.count () != piNUMBER_OF_ITEMS) return false;

   QVariant styl = variantList.value (piStyleSheet);
   if (styl.type () != QVariant::String) return false;

   QVariant data = variantList.value (piGenericData);
   if (data.type () != QVariant::ByteArray) return false;

   // Extract style sheet
   this->styleSheet = styl.toString ();

   // Extract generic data
   //
   QByteArray byteArray = data.toByteArray ();

   const int size = this->dataSize ();
   if (byteArray.size () == size) {
      int magic;

      memcpy (&magic, byteArray.data (), sizeof (int));

      // Sanity check
      if (magic == MAGIC_NUMBER) {

         // Look good copy the lot.
         //
         void* base = &this->firstMember;
         memcpy (base, byteArray.data(), size);
         result = true;

      } else {
         qDebug () << __FUNCTION__ << " magic number fail."
                   << "expecected:" << MAGIC_NUMBER
                   << ", actual:"  << magic;
      }

   } else {
      qDebug () << __FUNCTION__ << " size mis-match"
                << "expecected:" << size
                << ", actual:"  <<  byteArray.size ();
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QEScaling::extractFromWidget (const QWidget* widget)
{
   QLayout* layout = NULL;

   this->isDefined = false;
   if (!widget) {   // sainity check.
      return;
   }

   this->styleSheet = widget->styleSheet();
   this->geometry = widget->geometry ();
   this->minimumSize = widget->minimumSize ();
   this->maximumSize = widget->maximumSize ();
   this->pointSize = widget->font ().pointSize ();
   this->pixelSize = widget->font ().pixelSize ();

   layout = widget->layout ();
   if (layout) {
      layout->getContentsMargins (&this->layoutMarginLeft,  &this->layoutMarginTop,
                                  &this->layoutMarginRight, &this->layoutMarginBottom);
      this->layoutSpacing = layout->spacing ();

      QGridLayout* gridLayout = dynamic_cast <QGridLayout*> (layout);
      if (gridLayout) {
         // Grid layout extras
         //
         this->layoutHorizontalSpacing = gridLayout->horizontalSpacing ();
         this->layoutVerticalSpacing = gridLayout->verticalSpacing ();
      } else {
         this->layoutHorizontalSpacing = 0;
         this->layoutVerticalSpacing = 0;
      }

      this->layoutIsDefined = true;
   } else {
      this->layoutIsDefined = false;
   }

   // Specials.
   // Q? How expensive are dynamic castes? Use Qt's own caste?
   //    Leverage off some items being mutually exclusive.
   //
   const QLabel* label = dynamic_cast <const QLabel*>(widget);
   if (label) {
      this->indent = label->indent ();
   }

   const QSimpleShape* shape = dynamic_cast <const QSimpleShape*>(widget);
   if (shape) {
      this->indent = shape->getIndent ();
   }

   const QEResizeableFrame* resizeableFrame = dynamic_cast <const QEResizeableFrame*>(widget);
   if (resizeableFrame) {
      this->resizeFrameAllowedMin = resizeableFrame->getAllowedMinimum ();
      this->resizeFrameAllowedMax = resizeableFrame->getAllowedMaximum ();
   }

   const QTableWidget* tableWidget = dynamic_cast <const QTableWidget *>(widget);
   if (tableWidget) {
      this->tableDefaultHorizontalSectionSize = tableWidget->horizontalHeader ()->defaultSectionSize ();
      this->tableDefaultVerticalSectionSize = tableWidget->verticalHeader ()->defaultSectionSize ();
   }

   const QTreeView* treeView = dynamic_cast <const QTreeView *>(widget);
   if (treeView) {
      this->indent = treeView->indentation ();
   }

   this->isDefined = true;
}


//------------------------------------------------------------------------------
// Static members
//------------------------------------------------------------------------------
//
int QEScaling::currentScaleM = 1;
int QEScaling::currentScaleD = 1;
int QEScaling::currentFontScaleM = 1;
int QEScaling::currentFontScaleD = 1;

//------------------------------------------------------------------------------
//
void QEScaling::setScaling (const int m, const int d)
{
   // sanity check - m and d both positive.
   //
   if ((m > 0) && (d > 0)) {
      // Normalise rational number.
      //
      int g = gcd (m, d);
      QEScaling::currentScaleM = m/g;
      QEScaling::currentScaleD = d/g;
   }
}

//------------------------------------------------------------------------------
//
void QEScaling::getScaling (int& m, int& d)
{
   m = QEScaling::currentScaleM;
   d = QEScaling::currentScaleD;
}

//------------------------------------------------------------------------------
// static
//
void QEScaling::setFontScaling (const int fm, const int fd)
{
   // sanity check - m and d both positive.
   //
   if ((fm > 0) && (fd > 0)) {
      // Normalise rational number.
      //
      int g = gcd (fm, fd);
      QEScaling::currentFontScaleM = fm/g;
      QEScaling::currentFontScaleD = fd/g;
   }
}

//------------------------------------------------------------------------------
// static
//
void QEScaling::getFontScaling (int& fm, int& fd)
{
   fm = QEScaling::currentFontScaleM;
   fd = QEScaling::currentFontScaleD;
}

//------------------------------------------------------------------------------
// static
//
void QEScaling::widgetCapture (QWidget* widget)
{
   if (!widget) return;

   // Extract base line sizing and constraints. This is idempotent, first time
   // through, it extracts data from the widget and creates a property to save
   // relevant data; second and subqequent times through extracts data from the
   // property.
   //
   QEScaling s;
   bool okay = s.extractBaselineInformation (widget);
   if (!okay) {
      // The extraction of the baseline sizing info returned false, so assume
      // first time called for this widget - capture the sizing data.
      //
      s.captureBaselineInformation (widget);
   } // else func. returned true - info already extracted.
}

//------------------------------------------------------------------------------
// static
void QEScaling::applyScalingToWidget (QWidget* widget)
{
   // sainity check.
   //
   if (!widget) return;

   // Extract base line sizing and constraints.
   //
   QEScaling baseline;
   bool okay = baseline.extractBaselineInformation (widget);
   if (!okay) {
      // no/invalid baseline scaling info.
      // This typically in internal widget created post capture - just ignore.
      return;
   }

   QEWidget* qeWidget = dynamic_cast <QEWidget *>(widget);
   if (!qeWidget) {
      // This widget is not is not a QEWidget.
      //
      // QEWidgets make use of style to show alarm and connection status,
      // so we definitely don't want to mess with that.
      //
      // Attempt to apply scaling to any defined point and/or pixel sizes if defined.
      // If no scaleable px/pt attributes exist in the style sheet, just leave it.
      //
      // NOTE: This is a all a bit of a compromise. If a form designer defines
      // px/pt attributes for a QEWidget's style (e.g. default and/or user level
      // style) they will not get scaled; and if he/she defines px/pt attributes
      // in a non-QEWidget and sets other style attributes dynamically, the baseline
      // style is rescaled and the dynamic style attributes will be lost.
      //
      QString ss = QEScaling::scaleStyleSheet (baseline.styleSheet);
      if (ss != baseline.styleSheet) widget->setStyleSheet (ss);
   }

   QSize minSize = baseline.minimumSize;
   QSize maxSize = baseline.maximumSize;
   QRect geo = baseline.geometry;

   minSize.setWidth  (QEScaling::scale (minSize.width ()));
   minSize.setHeight (QEScaling::scale (minSize.height ()));

   // QWIDGETSIZE_MAX is the default max size - do not scale nor exceed this value.
   //
   if (maxSize.width () != QWIDGETSIZE_MAX) {
      maxSize.setWidth  (MIN (QEScaling::scale (maxSize.width ()), QWIDGETSIZE_MAX));
   }
   if (maxSize.height () != QWIDGETSIZE_MAX) {
      maxSize.setHeight (MIN (QEScaling::scale (maxSize.height ()), QWIDGETSIZE_MAX));
   }

   geo = QRect (QEScaling::scale (geo.left ()),
                QEScaling::scale (geo.top ()),
                QEScaling::scale (geo.width ()),
                QEScaling::scale (geo.height ()));

   if (QEScaling::currentScaleM >= QEScaling::currentScaleD) {
      // getting bigger - ensure consistancy - do max size constraint first.
      //
      widget->setMaximumSize (maxSize);
      widget->setMinimumSize (minSize);
   } else {
      // getting smaller - to min size constraint first.
      //
      widget->setMinimumSize (minSize);
      widget->setMaximumSize (maxSize);
   }

   widget->setGeometry (geo);

   QFont font = widget->font ();
   int pointSize = baseline.pointSize;
   int pixelSize = baseline.pixelSize;

   if (pointSize >= 0) {
      // Font point sizes must me at least one.
      font.setPointSize (MAX (1, QEScaling::scaleFont (pointSize)));
   }
   else if (pixelSize >= 0) {
      font.setPixelSize (MAX (1, QEScaling::scaleFont (pixelSize)));
   }
   widget->setFont (font);

   // Check if there is a layout
   //
   QLayout* layout = widget->layout ();
   if (layout && baseline.layoutIsDefined) {

       layout->setContentsMargins (QEScaling::scale (baseline.layoutMarginLeft),
                                   QEScaling::scale (baseline.layoutMarginTop),
                                   QEScaling::scale (baseline.layoutMarginRight),
                                   QEScaling::scale (baseline.layoutMarginBottom));

       QGridLayout* gridLayout = dynamic_cast <QGridLayout*> (layout);

       // Is this a grid layout??
       //
       if (gridLayout) {
          gridLayout->setHorizontalSpacing (QEScaling::scale (baseline.layoutHorizontalSpacing));
          gridLayout->setVerticalSpacing (QEScaling::scale (baseline.layoutVerticalSpacing));
       } else {
          // Horizontal or vertical layout.
          //
          layout->setSpacing (QEScaling::scale (baseline.layoutSpacing));
       }
   }

   // Specials.
   // Q? How expensive are dynamic casts? Use Qt's own caste?
   //    Leverage off some items being mutually exclusive.
   //
   QLabel* label = dynamic_cast <QLabel*>(widget);
   if (label) {
      int indent = baseline.indent;
      if (indent > 0) {
         indent = QEScaling::scale (indent);
         label->setIndent (indent);
      }
   }

   QSimpleShape* shape = dynamic_cast <QSimpleShape*>(widget);
   if (shape) {
      int indent = baseline.indent;
      if (indent > 0) {
         indent = QEScaling::scale (indent);
         shape->setIndent (indent);
      }
   }

   QEResizeableFrame* resizeableFrame = dynamic_cast <QEResizeableFrame*>(widget);
   if (resizeableFrame) {
      int allowedMin = baseline.resizeFrameAllowedMin;
      int allowedMax = baseline.resizeFrameAllowedMax;

      // scale
      allowedMin = QEScaling::scale (allowedMin);
      allowedMax = QEScaling::scale (allowedMax);

      if (QEScaling::currentScaleM >= QEScaling::currentScaleD) {
         // getting bigger - ensure consistancy - do max size constraint first.
         //
         resizeableFrame->setAllowedMaximum (allowedMax);
         resizeableFrame->setAllowedMinimum (allowedMin);
      } else {
         // getting smaller - to min size constraint first.
         //
         resizeableFrame->setAllowedMinimum (allowedMin);
         resizeableFrame->setAllowedMaximum (allowedMax);
      }
   }

   QTableWidget* tableWidget = dynamic_cast <QTableWidget *>(widget);
   if (tableWidget) {
      int defaultSectionSize;

      defaultSectionSize = baseline.tableDefaultHorizontalSectionSize;
      defaultSectionSize = QEScaling::scale (defaultSectionSize);
      tableWidget->horizontalHeader ()->setDefaultSectionSize (defaultSectionSize);

      defaultSectionSize =  baseline.tableDefaultVerticalSectionSize;
      defaultSectionSize = QEScaling::scale (defaultSectionSize);
      tableWidget->verticalHeader ()->setDefaultSectionSize (defaultSectionSize);
   }

   QTreeView* treeView = dynamic_cast <QTreeView *>(widget);
   if (treeView) {
      int indentation = baseline.indent;
      if (indentation > 0) {
         indentation = QEScaling::scale (indentation);
         treeView->setIndentation (indentation);
      }
   }

   if (qeWidget) {
      // For QEWidget objects, scaleBy is virtual function. This allows geometrically
      // complicated widgets, such as QEShape, to provide a bespoke scaling function.
      //
      qeWidget->scaleBy (QEScaling::currentScaleM, QEScaling::currentScaleD);
   }

   // Lastly save the scaling as applied to THIS widget.
   //
   QVariant mp = QVariant (int (QEScaling::currentScaleM));
   QVariant md = QVariant (int (QEScaling::currentScaleD));

   QVariantList appliedScale;
   appliedScale.append (mp);
   appliedScale.append (md);

   widget->setProperty (CURRENT_SCALE, appliedScale);
}

//------------------------------------------------------------------------------
//
void QEScaling::widgetTreeWalk (QWidget* widget, ScalingFunction sf)
{
   int j, n;
   QObjectList childList;
   QObject* child = NULL;
   QWidget* childWidget = NULL;

   // sainity checks and avoid divide by zero.
   //
   if (!widget) return;
   if (!sf) return;

   // Apply scaling function to this widget.
   //
   sf (widget);

   // Apply scaling to any child widgets.
   //
   childList = widget->children ();
   n = childList.count();
   for (j = 0; j < n; j++) {
      child = childList.value (j);
      // We need only tree walk widgets. All widget parents are themselves widgets.
      //
      childWidget = dynamic_cast <QWidget *>(child);
      if (childWidget) {
         // Recursive call.
         //
         QEScaling::widgetTreeWalk (childWidget, sf);
      }
   }
}

//------------------------------------------------------------------------------
// static
void QEScaling::applyToWidget (QWidget* widget)
{
   if (!widget) return;

   // We do two tree walks, first does a pure data capture, the second applies scaling.
   // The capture phase only actually captures info the first time call for the widget.
   //
   // This is particularly important for font sizes. If a child's font same as its
   // parent's then is scaled auto-magically when the parent's font is scaled, and
   // if we do it again it will get scalled twice. And the font of a grand-child item
   // will be scaled three times etc.
   //
   QEScaling::widgetTreeWalk (widget, QEScaling::widgetCapture);
   QEScaling::widgetTreeWalk (widget, QEScaling::applyScalingToWidget);
}

//------------------------------------------------------------------------------
// static
void QEScaling::rescaleWidget (QWidget* widget, const int rm, const int rd)
{
   // sanity check - rm and rd both positive.
   //
   if ((rm > 0) && (rd > 0)) {

      // Save application wide scaling parameters.
      //
      const int savedM = QEScaling::currentScaleM;
      const int savedD = QEScaling::currentScaleD;

      // There is no need to normalise rational number here.
      // But to restrict range 0.1 to 4.0 (i.e. 10% to 400%)
      //
      int lrm = LIMIT (rm, int (rd/10), int (rd*4));

      // Calculate combined scale.
      //
      int modM = lrm * savedM;
      int modD = rd  * savedD;

      QEScaling::setScaling (modM, modD);
      QEScaling::applyToWidget (widget);

      // Finally restore application wide scaling parameters.
      //
      QEScaling::setScaling (savedM, savedD);
   }
}

//------------------------------------------------------------------------------
// static
void QEScaling::rescaleWidget (QWidget* widget, const double newScale)
{
   int rm = int (100 * newScale);
   int rd = 100;
   QEScaling::rescaleWidget (widget, rm, rd);
}

//------------------------------------------------------------------------------
// static
void QEScaling::getWidgetScaling (const QWidget* widget, int& m, int& d)
{
   // Ensure not erroneous.
   //
   m = 1;
   d = 1;
   if (!widget) return;

   QVariant property = widget->property (CURRENT_SCALE);

   if (property.type () != QVariant::List) return;

   QVariantList variantList = property.toList ();
   if (variantList.count () != 2) return;

   QVariant mp = variantList.value (0);
   if (mp.type() != QVariant::Int) return;

   QVariant dp = variantList.value (1);
   if (dp.type() != QVariant::Int) return;

   m = mp.toInt ();
   d = dp.toInt ();
}

//------------------------------------------------------------------------------
// static
//
int QEScaling::scaleFont (const int v) {
   return (v * QEScaling::currentScaleM * QEScaling::currentFontScaleM ) /
              (QEScaling::currentScaleD * QEScaling::currentFontScaleD);
}

//------------------------------------------------------------------------------
//
void QEScaling::applyToPoint (QPoint& point)
{
   if (QEScaling::currentScaleM == QEScaling::currentScaleD) return;   // skip null scaling

   int x = point.x ();
   int y = point.y ();

   x = QEScaling::scale (x);
   y = QEScaling::scale (y);
   point = QPoint (x, y);
}

//------------------------------------------------------------------------------
// static
QString QEScaling::scaleStyleSheet (const QString& input)
{
   // skip scaling empty style sheet
   //
   if (input.isEmpty()) return input;

   const QString pattern = "[0-9][0-9]*p[xt]";
   const QRegularExpression re (pattern, QRegularExpression::NoPatternOption);

   QString workingInput = input;
   QString result = "";

   int next = workingInput.indexOf (re, 0);
   int safety_count = 0;
   while (next >= 0 && safety_count < 200) {
      // Move pre-regular expression text from input to result
      //
      result.append (workingInput.left (next));
      workingInput.remove (0, next);

      // Peel of the point/pixel size number, digit-by-digit
      //
      int px = 0;
      while ((workingInput.length() > 0) && (workingInput [0] >= '0') && (workingInput [0] <= '9')) {
         const QString d = workingInput.left (1);
         px = 10 * px + d.toInt();
         workingInput.remove (0, 1);
      }

      const int npx = QEScaling::scaleFont (px);

      result.append (QString::number(npx));
      next = workingInput.indexOf (re, 0);
      safety_count++;
   }

   result.append (workingInput);
   return result;
}

// end
