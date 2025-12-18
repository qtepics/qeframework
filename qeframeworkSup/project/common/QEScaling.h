/*  QEScaling.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2013-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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

/// This class provide the means to scale a Qt application and/or an individual widget.
/// When scaled, the size related attributes including size, min sizee, max size and
/// font size are scaled (note: different from the standard Qt layout streching).
/// While not perfect, this can be useful and acceptable.
///
/// The scaling is defined using a rational number specifed by two integers (m, d).
/// The first (m) integer is the multiplier and the second (d) integer is the divisor.
/// For example, if m = 4 and d = 5, then this specifies an 80%; and if m = 5 and d = 4,
/// this specifies that a 125% scaling is required.
///
/// Additonal font Scaling, above and beyond the overall scaling, may also be specified.
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
    /// The final font scaling is thus (m * fm) / (d * fd)
    /// Both values default to 1, which is a null scaling.
    /// Only valid scaling values (fm > 0, fd > 0) are accepted.
    //
    static void setFontScaling (const int fm, const int fd);

    /// Extract currently applied font scaling - allows widgets to perform widget class
    /// specific scaling functionality.
    ///
    static void getFontScaling (int& fm, int& fd);

    /// Adjust the geometry and font scaling of the widget and all child widgets by the
    /// defined global application scaling parameters (m, d). Unless m and d different,
    /// no scaling occurs.
    /// The function tree walks the hiearchy of widgets paranted by the specified widget.
    /// This function is idempotent.
    //
    static void applyToWidget (QWidget* widget);

    /// Applies the specified scaling to the nominated widget and all child widgets.
    /// This is in addition to the application wide scaling defined by setScaling.
    /// The scaling is limited to 0.1 to 400.0  (10% to 400%)
    //
    static void rescaleWidget (QWidget* widget, const int rm, const int rd);
    static void rescaleWidget (QWidget* widget, const double newScale);

    /// Get the current widget scaling. This may not be the global application
    /// scaling if rescaleWidget has been called, but does include the global scaling.
    //
    static void getWidgetScaling (const QWidget* widget, int& m, int& d);

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
    static void applyScalingToWidget (QWidget* widget);

    /// Applies scale to a size object.
    //
    static QSize scaleSize (const QSize& size);

private:
   int firstMember;    // used in conjection with lastMember to define object size.

   /// Static functions create an instance of this object. This object used to
   /// hold base line widget sizing data. The data is encoded and stored in a
   /// property associated with the widget.
   ///
   explicit QEScaling ();
   ~QEScaling ();

   // Captures and saves information necessary to scale a widget. This information
   // includes widget's geometry, minimum and maximum sizes point/pixel size etc.
   // The information is saved as a dynamic widget property.
   //
   void captureBaselineInformation (QWidget* widget);

   // Extracts captured scaling data from dynamic widget property if it exists,
   // and validates the data. The restored data is stored in this QEScaling object.
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
   QSize iconSize;   // QPushButton
   int indent;  // QLabel, QSimpleShape and QTreeView
   int resizeFrameAllowedMin;
   int resizeFrameAllowedMax;
   int tableDefaultHorizontalSectionSize;
   int tableDefaultVerticalSectionSize;

private:
   int lastMember;

   QString styleSheet;   // stylesheet not saved as generic data
};

# endif // QE_SCALING_H
