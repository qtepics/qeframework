/*  QEScaling.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
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
 *  Copyright (c) 2013 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *
 */

#include <QDebug>
#include <QLabel>
#include <QLayout>
#include <QHeaderView>
#include <QSize>
#include <QTableWidget>
#include <QTreeView>
#include <QWidget>

#include <QEResizeableFrame.h>
#include <QEWidget.h>
#include <QECommon.h>

#include "QEScaling.h"


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
#define BASELINE_SCALING    "QE_BASELINE_SCALING"

//------------------------------------------------------------------------------
//
QEScaling::QEScaling (QWidget* widget)
{
   this->firstMember = MAGIC_NUMBER;
   this->isDefined = false;
   this->layoutIsDefined = false;
   this->labelIndent = -1;
   this->resizeFrameAllowedMin = -1;
   this->resizeFrameAllowedMax = -1;
   this->tableDefaultHorizontalSectionSize  = -1;
   this->tableDefaultVerticalSectionSize  = -1;
   this->treeViewIndentation  = -1;

   QVariant baselinePropertyData;
   baselinePropertyData = widget->property (BASELINE_SCALING);

   if (baselinePropertyData.type () == QVariant::ByteArray) {
      // Base line data exists and is valid - use it.
      //
      if (this->decodeProperty (baselinePropertyData)) {
         // all okay
      } else {
         // decode failed - re-extract.
         //
         qDebug () << __FUNCTION__ << "scaling extraction failed.";

         this->extractFromWidget (widget);
         baselinePropertyData = this->encodeProperty ();
         widget->setProperty (BASELINE_SCALING, baselinePropertyData);
      }
   } else {
      // Base line data does not exists or is invalid.
      // Use current widget values as base line and save in property.
      //
      this->extractFromWidget (widget);
      baselinePropertyData = this->encodeProperty ();
      widget->setProperty (BASELINE_SCALING, baselinePropertyData);
   }
}

//------------------------------------------------------------------------------
//
QEScaling::~QEScaling ()
{
   this->firstMember = !MAGIC_NUMBER;
 }

//------------------------------------------------------------------------------
// Encodes from firstMember upto, but excluding lastMember as QByteArray QVariant.
//
QVariant QEScaling::encodeProperty () const
{
   if (!isDefined) {
      return    QVariant (QVariant::Invalid);
   }

   const void* base = NULL;
   int size;

   base = &this->firstMember;
   size = int (size_t (&this->lastMember) - size_t (&this->firstMember));

   QByteArray data ((char *)base, size);
   return QVariant (data);
}

//------------------------------------------------------------------------------
//
bool QEScaling::decodeProperty (const QVariant& property)
{
   bool result = false;
   void* base = NULL;
   int size;

   this->isDefined = false;

   base = &this->firstMember;
   size = int (size_t (&this->lastMember) - size_t (&this->firstMember));

   if (property.type () != QVariant::Invalid) {
      if (property.type () == QVariant::ByteArray) {
         QByteArray byetArray = property.toByteArray ();

         if (byetArray.size () == size) {
            int magic;

            memcpy (&magic, byetArray.data (), sizeof (int));

            if (magic == MAGIC_NUMBER) {

               // Look good copy the lot.
               //
               memcpy (base, byetArray.data(), size);
               result = true;

            } else {
               qDebug () << __FUNCTION__ << " magic number fail."
                         << "expecected:" << MAGIC_NUMBER
                         << ", actual:"  << magic;
            }

         } else {
            qDebug () << __FUNCTION__ << " size mis-match"
                      << "expecected:" << size
                      << ", actual:"  <<  byetArray.size ();
         }

      } else {
         qDebug () << __FUNCTION__ << "not a ByteArray QVariant:" << property.type ();
      }
   } // else no error per se

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
      this->labelIndent = label->indent ();
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
      this->treeViewIndentation = treeView->indentation ();
   }

   this->isDefined = true;
}


//------------------------------------------------------------------------------
// Static members
//------------------------------------------------------------------------------
//
int QEScaling::currentScaleM = 1;
int QEScaling::currentScaleD = 1;

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
//
void QEScaling::widgetCapture (QWidget* widget)
{
   if (!widget) return;

   // Extract base line sizing and constraints. This is idempotent, first time
   // through, it extracts data from the widget and creates a property to save
   // relevant data; second and subqequent times through extracts data from the
   // property.
   //
   QEScaling* baseline = new QEScaling (widget);
   delete baseline;
}

//------------------------------------------------------------------------------
//
void QEScaling::widgetScale (QWidget* widget)
{
   QLabel* label = NULL;
   QEWidget* qeWidget = NULL;
   QLayout* layout = NULL;
   QEResizeableFrame* resizeableFrame = NULL;
   QTableWidget* tableWidget = NULL;
   QTreeView* treeView = NULL;

   // sainity check.
   //
   if (!widget) return;

   // Extract base line sizing and constraints.
   //
   QEScaling* baseline = new QEScaling (widget);

   QSize minSize = baseline->minimumSize;
   QSize maxSize = baseline->maximumSize;
   QRect geo = baseline->geometry;

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
   int pointSize = baseline->pointSize;
   int pixelSize = baseline->pixelSize;

   if (pointSize >= 0) {
      // Font point sizes must me at least one.
      font.setPointSize (MAX (1, QEScaling::scale (pointSize)));
   }
   else if (pixelSize >= 0) {
      font.setPixelSize (MAX (1, QEScaling::scale (pixelSize)));
   }
   widget->setFont (font);

   // Check if there is a layout
   //
   layout = widget->layout ();
   if (layout && baseline->layoutIsDefined) {

       layout->setContentsMargins (QEScaling::scale (baseline->layoutMarginLeft),
                                   QEScaling::scale (baseline->layoutMarginTop),
                                   QEScaling::scale (baseline->layoutMarginRight),
                                   QEScaling::scale (baseline->layoutMarginBottom));

       layout->setSpacing (QEScaling::scale (baseline->layoutSpacing));
   }

   // Specials.
   // Q? How expensive are dynamic castes? Use Qt's own caste?
   //    Leverage off some items being mutually exclusive.
   //
   label = dynamic_cast <QLabel*>(widget);
   if (label) {
      int indent = baseline->labelIndent;
      if (indent > 0) {
         indent = QEScaling::scale (indent);
         label->setIndent (indent);
      }
   }

   resizeableFrame = dynamic_cast <QEResizeableFrame*>(widget);
   if (resizeableFrame) {
      int allowedMin = baseline->resizeFrameAllowedMin;
      int allowedMax = baseline->resizeFrameAllowedMax;

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

   tableWidget = dynamic_cast <QTableWidget *>(widget);
   if (tableWidget) {
      int defaultSectionSize;

      defaultSectionSize = baseline->tableDefaultHorizontalSectionSize;
      defaultSectionSize = QEScaling::scale (defaultSectionSize);
      tableWidget->horizontalHeader ()->setDefaultSectionSize (defaultSectionSize);

      defaultSectionSize =  baseline->tableDefaultVerticalSectionSize;
      defaultSectionSize = QEScaling::scale (defaultSectionSize);
      tableWidget->verticalHeader ()->setDefaultSectionSize (defaultSectionSize);
   }

   treeView = dynamic_cast <QTreeView *>(widget);
   if (treeView) {
      int indentation = baseline->treeViewIndentation;
      if (indentation > 0) {
         indentation = QEScaling::scale (indentation);
         treeView->setIndentation (indentation);
      }
   }

   qeWidget = dynamic_cast <QEWidget *>(widget);
   if (qeWidget) {
      // For QEWidget objects, scaleBy is virtual function. This allows geometrically
      // complicated widgets, such as QEShape, to provide a bespoke scaling function.
      //
      qeWidget->scaleBy (QEScaling::currentScaleM, QEScaling::currentScaleD);
   }

   delete baseline;
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
   if (QEScaling::currentScaleM == QEScaling::currentScaleD) return;   // skip null scaling

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
//
void QEScaling::applyToWidget (QWidget* widget)
{
   if (!widget) return;

   // We do two tree walks, first does a pure data capture, the second applyies scaling.
   //
   // This is particularly important for font sizes. If a child's font same as its
   // parent's then is scaled auto-magically when the parent's font is scaled, and
   // if we do it again it will get scalled twice. And the font of a grand-child item
   // will be scaled three times etc.
   //
   QEScaling::widgetTreeWalk (widget, QEScaling::widgetCapture);

   if (QEScaling::currentScaleM == QEScaling::currentScaleD) return;   // skip null scaling
   QEScaling::widgetTreeWalk (widget, QEScaling::widgetScale);
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

// end
