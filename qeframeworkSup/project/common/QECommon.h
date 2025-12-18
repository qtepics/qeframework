/*  QECommon.h
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

#ifndef QE_COMMON_H
#define QE_COMMON_H

#include <math.h>
#include <QDateTime>
#include <QList>
#include <QRect>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QEFrameworkLibraryGlobal.h>


// Useful type neutral numerical macro fuctions.
//
// Note: the input parameter(s) supplied to these macros is evaluated two (or
// three in the case of LIMIT) times. As such, no expression with a side effect
// should be used, nor an expression that takes excessive time to compute.
//
// Erroneous                           Use this (with suitable t declaration).
// f = MAX (random (), 0.25)           t = random (); f = MAX (t, 0.25);
// k = MAX (j++, 32);                  t = j++;       k = MAX (t, 32);
// r = MAX (cpu_hog(), 71.1);          t = cpu_hog(); r = MAX (t, 71.1);
//
#define ABS(a)             ((a) >= 0  ? (a) : -(a))
#define MIN(a, b)          ((a) <= (b) ? (a) : (b))
#define MAX(a, b)          ((a) >= (b) ? (a) : (b))
#define LIMIT(x,low,high)  (MAX(low, MIN(x, high)))

// Calculates number of items in an array.
//
#define ARRAY_LENGTH(xx)   (int (sizeof (xx) /sizeof (xx [0])))


// Provide log and exp 10 macro functions.
//
// LOG10 is a safe log in that it avoids attempting to take the log of negative
// or zero values. The 1.0e-20 limit is somewhat arbitary, but in practice is
// good for most numbers encountered at the synchrotron.
//
// EXP10 is the the inverse function.
// Not all platforms provide exp10. What about pow () ??
//
#define LOG10(x)  ( (x) >=  1.0e-20 ? log10 (x) : -20.0 )
#define EXP10(x)  exp (2.302585092994046 * (x))

// 2 pi
//
#define TAU       6.283185307179586477

// Expose access to the internal object's set/get functions.
// Uses same function names.
//
#define QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS(object, type, getfunc, setfunc)   \
   void setfunc (const type& v) { this->object->setfunc (v); }                \
   type getfunc () const { return this->object->getfunc (); }


// We do not include QColor and QWidget header files in this header file (they
// are called b QECommon.cpp), we just provide differed declarations.
// This particularly usefull for non-gui command line programs.
//
class QColor;
class QLabel;
class QMainWindow;
class QWidget;
class QEWidget;

// General purpose utility functions.
// We use a class of static methods as opposed to a set of regular functions.
// This provide a name space, but also allows inheritance if needs be.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEUtilities {
public:
    /// For the specified bacground colour, returns font colour (black or white)
    /// with a suitable contrast. Currently based on rgb values, and ignores alpha.
    ///
    static QColor fontColour (const QColor& backgroundColour);

    /// Converts a given colours to the associated style.
    ///
    static QString colourToStyle (const QColor& backgroundColour,
                                  const QColor& foregroundColour);

    /// Converts a given colour to associated background-color style,
    /// and sets foreground (font) colour to black or white accordingly.
    ///
    static QString colourToStyle (const QColor& backgroundColour);

    /// Provides a slightly lighter colour than the standard form background
    /// colour. This is particularly useful for QLabel based widgets.
    ///
    static QString offBackgroundStyle ();

    // Same colour, just darker - good for edges.
    //
    static QColor darkColour (const QColor& lightColour);

    // Same colour, just washed-out/greyed-out - good for disabled widgets.
    //
    static QColor blandColour (const QColor& vibrantColour);

    /// Rounds a float to the nearest integer. Not all compilers support lround.
    /// If x outside of integer range, the result is set to the min or max allowed
    /// integer value and the ok parameter (if defined) set false if x out of range.
    ///
    static int roundToInt (const double x, bool* ok = NULL);

    /// Converts a time interval (in seconds) to format  +d hh:mm:ss.fff
    ///
    static QString intervalToString (const double time,
                                     const int precision,
                                     const bool showDays);

    /// Get the local time zone offset (in seconds) for the nominated time.
    /// This is not fixed for the location and will depend on if the the time
    /// is a standard time or a daylight savings/summer time.
    ///
    //  Option: default atTime = QDateTime::currentDateTime () ??
    //
    static int getTimeZoneOffset (const QDateTime & atTime);

    /// Extract the local time zone three letter acronym for the time.
    /// If the timeSpec patameter is Qt::UTC, then this simply returns "UTC".
    /// However, if the timeSpec patameter is Qt::LocalTime, then extract the TLA
    /// application of the time in question (and NOT for the current time).
    /// E.g. for the UK this would be GMT or BST, for Victoria Australia EST or EST.
    ///
    static QString getTimeZoneTLA (const Qt::TimeSpec timeSpec,
                                   const QDateTime & atTime);

    /// Overloaded function that uses the timeSpec assocaited with atTime.
    ///
    static QString getTimeZoneTLA (const QDateTime & atTime);

    /// This function returns the image, as a QString, of a enumeration value (cast as an integer).
    /// An invalid enumeration value returns a null string.
    ///
    /// Note: This functions rely on the meta object compiler (moc) generated code.
    /// To use this functions, the enumeration type must be declared within a Q_OBJECT QObject
    /// and the enum type qualifier with the Q_ENUM, e.g. Q_ENUM (Shapes).
    ///
    static QString enumToString (const QObject& object,
                                 const QString& enumTypeName,
                                 const int enumValue);

    /// This function returns the image, as a QString, of a enumeration value (cast as an integer).
    /// An invalid enumeration value returns a null string.
    ///
    /// Note: This functions rely on the meta object compiler (moc) generated code.
    /// To use this functions, the enumeration type must be declared within a Q_OBJECT QObject
    /// and the enum type qualifier with the Q_ENUM, e.g. Q_ENUM (Shapes).
    ///
    static QString enumToString (const QMetaObject& mo,
                                 const QString& enumTypeName,
                                 const int enumValue);

    /// This function returns the enumeration value given an enumeration image. The caller must
    /// cast the result to the appropriate enumeration type. The image must be exact match including
    /// case. The only tolerance allowed for is that the image is trimmed.
    /// An invalid image cause this function to return -1. However, -1 is an uncommon but not invalid
    /// enumeration value. If -1 might be a valid value, then the caller should specify the and
    /// check the ok parameter.
    ///
    /// Note: This functions rely on the meta object compiler (moc) generated code.
    /// To use this functions, the enumeration type must be declared within a Q_OBJECT QObject
    /// and the enum type qualifier with the Q_ENUM, e.g. Q_ENUM (Shapes).
    ///
    static int stringToEnum  (const QObject& object,
                              const QString& enumTypeName,
                              const QString& enumImage,
                              bool* ok = 0);

    /// This function returns the enumeration value given an enumeration image. The caller must
    /// cast the result to the appropriate enumeration type. The image must be exact match including
    /// case. The only tolerance allowed for is that the image is trimmed.
    /// An invalid image cause this function to return -1. However, -1 is an uncommon but not invalid
    /// enumeration value. If -1 might be a valid value, then the caller should specify the and
    /// check the ok parameter.
    ///
    /// Note: This functions rely on the meta object compiler (moc) generated code.
    /// To use this functions, the enumeration type must be declared within a Q_OBJECT QObject
    /// and the enum type qualifier with the Q_ENUM, e.g. Q_ENUM (Shapes).
    ///
    static int stringToEnum  (const QMetaObject& mo,
                              const QString& enumTypeName,
                              const QString& enumImage,
                              bool* ok = 0);

    /// This function tree walks the given parent looking a widget of the specified class
    /// name. The function returns the first found or NULL.
    ///
    static QWidget* findWidget (QWidget* parent, const QString& className);

    /// Split a string to a string list based on white space, skipping empty parts.
    ///
    static QStringList split (const QString& s);

    /// Converts QVariant to a QStringList, where each string item is further
    /// split using white space as delimiter. This function was principally
    /// designed for use within the paste/drop functions of widgets that support
    /// multiple PVs.
    ///
    static QStringList variantToStringList (const QVariant& v);

    /// Performs deep search and writes all PV names to nominated target file.
    ///
    static void listPVNames (QWidget* rootWidget,
                             const QString& targetFile,
                             const QString& comment);

    /// Performs deep search and initiates updates.
    ///
    static void activate (QWidget* rootWidget);

    /// Performs deep search and terminates updates.
    ///
    static void deactivate (QWidget* rootWidget);

    /// Returns the directory name as QString of the given pathname.
    /// This is essentially like the Linux dirname command.
    /// This conveniance functions hides the faffing with QFileInfo/QDir
    ///
    static QString dirName (const QString& pathName);

    /// Find containging MainWindow object, or return NULL if it does not exist.
    /// The secified widget itself can be the MainWindow.
    //
    static QMainWindow* findMainWindow (QWidget* widget);

    /// Sets the form title of the containing MainWindow if found.
    ///
    static void setWindowTitle (QWidget* widget, const QString& title);

    /// Gets the form title of the containing MainWindow if found, else empty string.
    ///
    static QString getWindowTitle (QWidget* widget);

    /// Returns the virtual desktop geometry.
    ///
    static QRect desktopGeometry ();

    /// Returns the screen geometry (of the specified widget)
    /// If specified widhet is null, then returns desktopGeometry.
    ///
    static QRect screenGeometry (QWidget* widget);

    /// Allows an object to be assigned an arbitartint tag value.
    /// Under the covers, an obkect property is created/updated.
    // Cribbed from KDM
    //
    static void tagObject (QObject* object, const int tag);
    static int objectTag (const QObject* object, const int defaultValue = -1);

    /// Repeatedly right pads text with pad, and then applies to the label.
    /// Does nothing if label is nullptr, no padding if pad is an empty string.
    // Cribbed from kubili.
    //
    static void setPaddedText (QLabel* label,
                               const QString& text,
                               const QString& pad = " . ");

    /// Testing only - outputs the widget hierarchy to std out.
    ///
    static void debugWidgetHierarchy (const QWidget* root,
                                      const int instance = 0,
                                      const int level = 0);
private:
    typedef QList<QEWidget*> QEWidgetList;
    typedef QList<QWidget*> QWidgetList;

    static void treeWalkAndAppend (QObject* item, QEWidgetList& list);
    static QEWidgetList findAllQEWidgets (QWidget* rootWidget);

    static void treeWalkAndAppend (QObject* item, QWidgetList& list);
    static QWidgetList findAllQWidgets (QWidget* rootWidget);
};

# endif // QE_COMMON_H
