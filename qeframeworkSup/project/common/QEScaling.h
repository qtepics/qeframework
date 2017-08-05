/*  QEScaling.h
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
 *  Copyright (c) 2013,2017 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *
 */

#ifndef QE_SCALING_H
#define QE_SCALING_H

#include <QObject>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QString>
#include <QVariant>

#include <QEFrameworkLibraryGlobal.h>

/// The scaling is defined using a rational number specifed by two integers (m, d).
/// The first (m) integer is the multiplier and the second (d) integer is the divisor.
/// For example, if m = 4 and d = 5, then this specifies an 80%; and if m = 5 and d = 4,
/// this specifies that a 125% scaling is required.
///
/// Scaling is deemed to be application wide, hence all scaling data (and functions)
/// are static.
///
// Note: This was formerly part of QECommon.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEScaling {
public:
    /// Set application wide general gui scaling values.
    /// Both values default to 1, which is a null scaling.
    /// Only valid scaling values (m > 0, d > 0) are accepted.
    ///
    static void setScaling (const int m, const int d);

    /// Extract currently applied scaling - allows widgets to perform widget class
    /// specific scaling functionality.
    ///
    static void getScaling (int& m, int& d);

    /// Set application wide additional font scaling values.
    /// The final font scaling is this (m * fm) / (d * fd)
    /// Both values default to 1, which is a null scaling.
    /// Only valid scaling values (fm > 0, fd > 0) are accepted.
    //
    static void setFontScaling (const int fm, const int fd);

    /// Extract currently applied font scaling - allows widgets to perform widget class
    /// specific scaling functionality.
    ///
    static void getFontScaling (int& fm, int& fd);

    /// Adjust the geometry and font scaling of the widget and all child widgets
    /// by the defined scaling parameters (m, d). Unless m and d different, no scaling occurs.
    /// The function tree walks the hiearchy of widgets paranted by the specified widget.
    /// This function is idempotent.
    //
    static void applyToWidget (QWidget* widget);

    /// Applies applied scaling to the nominated widget. This is in addition to
    /// the application wide scalining defined by setScaling. The scaling is
    /// limited to 0.1 to 400.0  (10% to 400%)
    //
    static void rescaleWidget (QWidget* widget, const double newScale);

    /// Conveniance functions for widget specific 'scaleBy' functions.
    ///
    /// Scales a single value. Note: all other scaling functions take a object by reference
    /// and modify that object. Only this function returns a scaled value.
    ///
    /// General scale function.
    //
    static inline int scale (const int v) { return (v * QEScaling::currentScaleM) /
                                                        QEScaling::currentScaleD; }

    /// Font specific scale function.
    //
    static int scaleFont (const int v);

    /// Scales a point.
    ///
    static void applyToPoint (QPoint& point);

    /// Scales styleSheet - looks for  "... <number>px ..." or "... <number>pt ..."
    /// and scales the number.
    ///
    static QString scaleStyleSheet (const QString& input);

private:
    static int currentScaleM;
    static int currentScaleD;
    static int currentFontScaleM;
    static int currentFontScaleD;

    /// Tree walks the QWidget hierarchy in order to apply supplied scaling finction.
    ///
    typedef void (*ScalingFunction) (QWidget* widget);
    static void widgetTreeWalk (QWidget* widget, ScalingFunction sf);

    /// Captures baseline scaling info as property, if not already done so.
    ///
    static void widgetCapture (QWidget* widget);

    /// Scales a single widget
    /// Applies some special processing above and beyond size, min size, max size and font
    /// depending on the type of widget. Also if is a QEWidget then calls QEWidget's scaleBy
    /// method.
    // This function does all the hard work.
    //
    static void widgetScale (QWidget* widget);

private:
   int firstMember;    // used in conjection with lastMember to define size.

   /// Static functions create an instance of this object. This object used to
   /// hold base line widget sizing data. The data is encoded and stored in a
   /// property associated with the widget.
   ///
   explicit QEScaling ();
   ~QEScaling ();

   // Captures and saves information necessary to scale a widget. This information
   // includes widigets geometry, minimum and maximum sizes point/pixel size etc.
   // The information is saved as a dynamic widget property.
   //
   void captureBaselineInformation (QWidget* widget);

   // Extracts captured scaling data from dynamic widget property if it exists,
   // validates date. The restored data is stored in this QEScaling object.
   // Note: this function is not the opposite/inverse of captureBaselineInformation
   // as the extracted data is not applied to the widget.
   //
   bool extractBaselineInformation (const QWidget* widget);

   // Utility functions
   //
   int dataSize () const;
   void extractFromWidget (const QWidget* widget);
   bool decodeProperty (const QVariant& property);
   QVariant encodeProperty () const;

   bool isDefined;

   // Basic geomertry and size constraints.
   //
   QRect geometry;
   QSize minimumSize;
   QSize maximumSize;

   // Font size inofrmation.
   //
   int pointSize;
   int pixelSize;

   // Layouts
   //
   bool layoutIsDefined;
   int layoutMarginLeft;
   int layoutMarginTop;
   int layoutMarginRight;
   int layoutMarginBottom;
   int layoutSpacing;
   int layoutHorizontalSpacing;
   int layoutVerticalSpacing;

   // Specials - for particular widget types.
   //
   int labelIndent;
   int resizeFrameAllowedMin;
   int resizeFrameAllowedMax;
   int tableDefaultHorizontalSectionSize;
   int tableDefaultVerticalSectionSize;
   int treeViewIndentation;

private:
   int lastMember;

   QString styleSheet;   // stylesheet not saved as generic data
};

# endif // QE_SCALING_H
