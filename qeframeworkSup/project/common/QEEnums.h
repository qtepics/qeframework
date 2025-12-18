/*  QEEnums.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2022-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_ENUMS_H
#define QE_ENUMS_H

#include <QMetaType>
#include <QObject>
#include <QEFrameworkLibraryGlobal.h>

/// This class defines common widget property types.
/// This will apply to the EPICS Qt 4 series
///
/// This class is not intended to be inherited by any other class.
/// The use of the class, as opposed to a just name space, is to allow
/// moc to generate the associated meta data.
/// The advantage of this is two-fold, firstly it simplifies all the header
/// files in that they all do not need to redefine the enums; and secondly
/// if the widget hierarchy is ever re-organised, the enumeration names in
/// the ui file will remain constant.
///
/// Roughly the eqivilent of qnamespace.h, without the clever namespace/object
/// compile time selection.
//
// Note: these enums will become the master/single source of truth definitions.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QE : public QObject
{
   Q_OBJECT
public:

   //---------------------------------------------------------------------------
   // Provides conveniance/utility functions for the enums.
   //
#define QE_ENUM_FN(typeName)                                                \
   static int typeName##Count();                                            \
   static QString typeName##Image (const typeName enumValue);               \
   static typeName typeName##Value (const QString& image, bool& okay);


   //---------------------------------------------------------------------------
   /// \enum MessageFilterOptions
   /// Message properties
   /// Not all QE widgets need these properties (only those that do something with
   /// messages - like the logging widget)
   ///
   enum MessageFilterOptions {
      Any = 0,        ///<
      Match,          ///<
      None            ///<
   };
   Q_ENUM (MessageFilterOptions)
   QE_ENUM_FN (MessageFilterOptions)


   //---------------------------------------------------------------------------
   // Standard properties
   //
   /// \enum UserLevels
   /// User friendly enumerations for #userLevelVisibility and #userLevelEnabled
   /// properties - refer to #userLevelVisibility and #userLevelEnabled properties
   /// and userLevel enumeration for details.
   ///
   enum UserLevels {
      User = 0,       ///< User level - least privilaged
      Scientist,      ///< User level - more privilaged than user, less than engineer
      Engineer        ///< User level - most privilaged
   };
   Q_ENUM (UserLevels)
   QE_ENUM_FN (UserLevels)


   /// \enum DisplayAlarmStateOptions
   /// User friendly enumerations for #displayAlarmStateOption property - refer
   /// to #displayAlarmStateOption property and displayAlarmStateOptions enumeration
   /// for details.
   ///
   enum DisplayAlarmStateOptions {
      Always = 0,     ///< Always display the alarm state
      WhenInAlarm,    ///< Display the alarn state in alarm, i.e. severity >= MINOR
      WhenInvalid,    ///< Display the alarn state invalid, i.e. severity = INVALID
      Never           ///< Never display the alarm state
   };
   Q_ENUM (DisplayAlarmStateOptions)
   QE_ENUM_FN (DisplayAlarmStateOptions)


   //---------------------------------------------------------------------------
   // String formatting
   //
   /// \enum    Formats
   /// User friendly enumerations for format property - refer to
   /// formats for details.
   ///
   enum Formats {
      Default  = 0,     ///< Format as best appropriate for the data type
      Floating,         ///< Format as a floating point number
      Integer,          ///< Format as an integer
      UnsignedInteger,  ///< Format as an unsigned integer
      Time,             ///< Format as a time, i.e. days hh:mm:ss[.frac]
      LocalEnumeration, ///< Format as a selection from the #localEnumeration property
      String            ///< Format as a string
   };
   Q_ENUM (Formats)
   QE_ENUM_FN (Formats)


   /// \enum Separators
   /// User friendly enumerations for seprator property - refer to formats for details.
   ///
   enum Separators {
      NoSeparator = 0,  ///< Use no separator,  e.g. 123456.123456789
      Comma,            ///< Use ',' as separator, e.g. 123,456.123,456,789
      Underscore,       ///< Use '_' as separator, e.g. 123_456.123_456_789
      Space             ///< Use ' ' as separator, e.g. 123 456.123 456 789
   };
   Q_ENUM (Separators)
   QE_ENUM_FN (Separators)


   /// \enum Notations
   /// User friendly enumerations for notation property - refer to notations
   /// for details.
   ///
   enum Notations {
      Fixed = 0,        ///< Standard floating point, e.g. 123456.789
      Scientific,       ///< Scientific representation, e.g. 1.23456789e6
      Automatic         ///< Automatic choice of standard or scientific notation
   };
   Q_ENUM (Notations)
   QE_ENUM_FN (Notations)


   /// \enum ArrayActions
   /// User friendly enumerations for arrayAction property - refer to
   /// arrayActions for details.
   ///
   enum ArrayActions {
      Append = 0,       ///< Interpret each element in the array as a value
                        ///  and append string representations of each
                        /// element from the array with a space in between each.
      Ascii,            ///< Interpret each element from the array as a character
                        ///  in a string. Translate all non printing characters
                        ///  to '?' except for trailing zeros (ignore them).
      Index             ///< Interpret the element selected by setArrayIndex()
                        ///  as the value.
   };
   Q_ENUM (ArrayActions)
   QE_ENUM_FN (ArrayActions)


   /// \enum Radicies
   /// Defines allowed radix values.
   enum Radicies {
      Decimal = 0,         ///< base 10
      Hexadecimal,         ///< base 16
      Octal,               ///< base 8
      Binary               ///< base 2
   };
   Q_ENUM (Radicies)
   QE_ENUM_FN (Radicies)


   //---------------------------------------------------------------------------
   // Buttons/QMenuButton
   //
   /// \enum UpdateOptions
   /// User friendly enumerations for button updateOption property.
   /// Refer to QEGenericButton for details.
   // Applicable when subscribe is set true.
   // Would have liked to use a flag, but want to maintain backward compatibility.
   //
   enum UpdateOptions {
      NoUpdate         = 0,  ///< Data updates will not update button
      Text             = 1,  ///< Data updates will update the button text
      Icon             = 2,  ///< Data updates will update the button icon
      TextAndIcon      = 3,  ///< Data updates will update the button text and icon
      State            = 4,  ///< Data updates will update the button state (checked or unchecked)
      TextAndState     = 5,  ///< Data updates will update the button text and state
      IconAndState     = 6,  ///< Data updates will update the button icon and state
      TextIconAndState = 7   ///< Data updates will update the button - the lot
   };
   Q_ENUM (UpdateOptions)
   QE_ENUM_FN (UpdateOptions)


   /// Specifies how a (control) widget should behave if/when the underlying
   /// record becomes disabled (i.e. DISA and DISV fields become equal).
   ///
   enum DisabledRecordPolicy {
      ignore = 0,       ///< No change in appearance - the default
      grayout,          ///< Grayed out apperance, i.e. same as when the widget's PV is disconnected
      disable           ///< Widget is disabled.
   };
   Q_ENUM (DisabledRecordPolicy)
   QE_ENUM_FN (DisabledRecordPolicy)


   /// \enum ProgramStartupOptions
   /// Startup options. Just run the command, run the command within a terminal,
   /// or display the output in QE message system.
   ///
   enum ProgramStartupOptions {
      NoOutput = 0,     ///< Just run the program - was None
      Terminal,         ///< Run the program in a termainal (in Windows a command
                        ///  interpreter will also be started, so the program may
                        ///  be a built-in command like 'dir')
      LogOutput,        ///< Run the program, and log the output in the QE message system
      StdOutput         ///< Run the program, and send doutput to standard output and standard error
   };
   Q_ENUM (ProgramStartupOptions)
   QE_ENUM_FN (ProgramStartupOptions)


   /// \enum CreationOption
   /// Creation options. Used to indicate how to present a GUI when requesting
   /// a new GUI be created. Open a new window, open a new tab, or replace the
   /// current window.
   ///
   enum CreationOptions {
      Open = 0,         ///< Replace the current GUI with the new GUI
      NewTab,           ///< Open new GUI in a new tab
      NewWindow,        ///< Open new GUI in a new window
      DockTop,          ///< Open new GUI in a top dock window
      DockBottom,       ///< Open new GUI in a bottom dock window
      DockLeft,         ///< Open new GUI in a left dock window
      DockRight,        ///< Open new GUI in a right dock window
      DockTopTabbed,    ///< Open new GUI in a top dock window (tabbed with any existing dock in that area)
      DockBottomTabbed, ///< Open new GUI in a bottom dock window (tabbed with any existing dock in that area)
      DockLeftTabbed,   ///< Open new GUI in a left dock window (tabbed with any existing dock in that area)
      DockRightTabbed,  ///< Open new GUI in a right dock window (tabbed with any existing dock in that area)
      DockFloating      ///< Open new GUI in a floating dock window
   };
   Q_ENUM (CreationOptions)
   QE_ENUM_FN (CreationOptions)


   //---------------------------------------------------------------------------
   // QEPlotter/QEStripChart
   //
   /// \enum VideoModes
   /// Allow background mode selection
   // Change to BackgroundMode??
   //
   enum VideoModes {
      normal,              ///< white background
      reverse              ///< black backgound
   };
   Q_ENUM (VideoModes)
   QE_ENUM_FN (VideoModes)


   //---------------------------------------------------------------------------
   // QELineEdit
   //
   /// \enum DropOption
   /// Allow drag/drops mode to this widget.
   ///
   enum DropOption {
      DropToVariable = 0,  ///< the dropped text will be used as a new variable name.
      DropToText,          ///< just set text on the widget
      DropToTextAndWrite   ///< write the text to the PV if the PV has been connected
                           ///  otherwise equivalent to DropToText
   };
   Q_ENUM (DropOption)
   QE_ENUM_FN (DropOption)


   //---------------------------------------------------------------------------
   // QEPlot and Abstract 2D data option.
   //
   /// \enum MouseMoveSignals
   /// Mouse move signal selection options.
   ///
   enum MouseMoveSignals {
      signalNone   = 0x0000,  ///< Can't use literal 0 with new Qt and/or compilers
      signalStatus = 0x0001,  ///< signals row, col and value as status text via sendMessage
      signalData   = 0x0002,  ///< signals row, col and value emited as binary data
      signalText   = 0x0004,  ///< signals row, col and value emited as as text
   };
   Q_ENUM (MouseMoveSignals)
   QE_ENUM_FN (MouseMoveSignals)

   Q_DECLARE_FLAGS (MouseMoveSignalFlags, MouseMoveSignals)
   Q_FLAG (MouseMoveSignalFlags)


   //---------------------------------------------------------------------------
   // QEImage related
   //
   /// \enum ImageFormatOptions
   /// User friendly enumerations for #formatOption property - refer to
   /// #formatOption property and #formatOptions enumeration for details.
   ///
   enum ImageFormatOptions {
      Mono = 0,         ///< Grey scale
      Bayer,            ///< Colour (Bayer Red Green)
      BayerGB,          ///< Colour (Bayer Green Blue)
      BayerBG,          ///< Colour (Bayer Blue Green)
      BayerGR,          ///< Colour (Bayer Green Red)
      BayerRG,          ///< Colour (Bayer Red Green)
      rgb1,             ///< Colour (24 bit RGB)
      rgb2,             ///< Colour (??? bit RGB)
      rgb3,             ///< Colour (??? bit RGB)
      yuv444,           ///< Colour (???)
      yuv422,           ///< Colour (???)
      yuv421            ///< Colour (???)
   };
   Q_ENUM (ImageFormatOptions)
   QE_ENUM_FN (ImageFormatOptions)

   // We don't want this as the last enumeration value as it will
   // "stuff--up" the QEImage format property in designer.
   //
   static const int numberOfImageFormats = int (QE::yuv421) + 1;

   //---------------------------------------------------------------------------
   // QImage and 2D data rotation option.
   //
   /// \enum RotationOptions
   /// User friendly enumerations for #rotation property
   ///
   enum RotationOptions {
      NoRotation = 0,   ///< No image rotation
      Rotate90Right,    ///< Rotate image 90 degrees clockwise
      Rotate90Left,     ///< Rotate image 90 degrees anticlockwise
      Rotate180         ///< Rotate image 180 degrees
   };
   Q_ENUM (RotationOptions)
   QE_ENUM_FN (RotationOptions)


   /// \enum PVLabelMode
   /// The default is useAliasName, however the default aliasName are undefined,
   /// so the effective default is usePvName.
   ///
   enum PVLabelMode {
      usePvName = 0,    ///< display the PV name
      useAliasName,     ///< display alias if defined otherwise display the PV name
      useDescription    ///< display DESCription if defined otherwise display the PV name
   };
   Q_ENUM (PVLabelMode)
   QE_ENUM_FN (PVLabelMode)


   //---------------------------------------------------------------------------
   // Miscellaneous
   //
   // Used by QELog, QEConfiguredLayout, QEFileBrowser, QERecipe and QEScript
   /// \enum    optionsLayoutProperty
   /// Provides the location of items within a widget
   ///
   enum LayoutOptions {
      Top = 0,          ///<
      Bottom,           ///<
      Left,             ///<
      Right             ///<
   };
   Q_ENUM (LayoutOptions)
   QE_ENUM_FN (LayoutOptions)


   // Used by QEPeriodic, QESelector, QEConfiguredLayout, QERecipe, QEScript
   /// \enum    UserInfoSourceOptions
   /// Provides the user defined source information for the widget
   ///
   enum SourceOptions {
      SourceText = 0,   ///< User info held in string/string list property.
      SourceFile        ///< User info held in a configuration text file
   };
   Q_ENUM (SourceOptions)
   QE_ENUM_FN (SourceOptions)


   //---------------------------------------------------------------------------
   /// \enum    GridOrders
   /// GridOrders specfies how grid elements are layed out.
   ///
   /// Row major (example 10, items, 3 cols):
   ///    0  1  2
   ///    3  4  5
   ///    6  7  8
   ///    9  -  -
   ///
   /// Col major (example 10, items, 3 cols):
   ///    0  4  8
   ///    1  5  9
   ///    2  6  -
   ///    3  7  -
   ///
   enum GridOrders {
      rowMajor = 0,     ///<
      colMajor          ///<
   };
   Q_ENUM (GridOrders)
   QE_ENUM_FN (GridOrders)

private:
   // The QE object itself is private and just a means to do enum to string
   // and string to enum conversion using the QE::<typename>Image and
   // QE::<typeName>Value static methods.
   //
   explicit QE (QObject* parent = 0);
   ~QE ();

#undef QE_ENUM_FN
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QE::MessageFilterOptions)
Q_DECLARE_METATYPE (QE::UserLevels)
Q_DECLARE_METATYPE (QE::DisplayAlarmStateOptions)
Q_DECLARE_METATYPE (QE::Formats)
Q_DECLARE_METATYPE (QE::Separators)
Q_DECLARE_METATYPE (QE::Notations)
Q_DECLARE_METATYPE (QE::ArrayActions)
Q_DECLARE_METATYPE (QE::Radicies)
Q_DECLARE_METATYPE (QE::UpdateOptions)
Q_DECLARE_METATYPE (QE::DisabledRecordPolicy)
Q_DECLARE_METATYPE (QE::ProgramStartupOptions)
Q_DECLARE_METATYPE (QE::CreationOptions)
Q_DECLARE_METATYPE (QE::VideoModes)
Q_DECLARE_METATYPE (QE::DropOption)
Q_DECLARE_METATYPE (QE::MouseMoveSignals)
Q_DECLARE_METATYPE (QE::MouseMoveSignalFlags)
Q_DECLARE_METATYPE (QE::ImageFormatOptions)
Q_DECLARE_METATYPE (QE::RotationOptions)
Q_DECLARE_METATYPE (QE::PVLabelMode)
Q_DECLARE_METATYPE (QE::LayoutOptions)
Q_DECLARE_METATYPE (QE::SourceOptions)
Q_DECLARE_METATYPE (QE::GridOrders)
#endif

#endif  // QE_ENUMS_H
