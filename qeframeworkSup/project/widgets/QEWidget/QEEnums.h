/*  QEEnums.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2022 Australian Synchrotron
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
 */

#ifndef QE_ENUMS_H
#define QE_ENUMS_H

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
// TODO: relocate other common property types here.
// We can't without breaking exisiting ui files.
// Need a ui update tool to support this.
// For the most part, just need to change, e.g.:
//   QELabel::Scientist => QE::Scientist
//
// Note: these enums will become the master/signgle source of truth definitions.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QE : public QObject
{
   Q_OBJECT
public:

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
   Q_ENUMS (UserLevels)


   /// \enum DisplayAlarmStateOptions
   /// User friendly enumerations for #displayAlarmStateOption property - refer
   /// to #displayAlarmStateOption property and displayAlarmStateOptions enumeration
   /// for details.
   ///
   enum DisplayAlarmStateOptions {
      Always = 0,     ///< Always display the alarm state
      WhenInAlarm,    ///< Display the alarn state in alarm, i.e. severiy >= MINOR
      WhenInvalid,    ///< Display the alarn state invalid, i.e. severiy = INVALID
      Never           ///< Never display the alarm state
   };
   Q_ENUMS (DisplayAlarmStateOptions)

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
   Q_ENUMS (Formats)


   /// \enum Separators
   /// User friendly enumerations for seprator property - refer to formats for details.
   enum Separators {
      NoSeparator = 0,  ///< Use no separator,  e.g. 123456.123456789
      Comma,            ///< Use ',' as separator, e.g. 123,456.123,456,789
      Underscore,       ///< Use '_' as separator, e.g. 123_456.123_456_789
      Space             ///< Use ' ' as separator, e.g. 123 456.123 456 789
   };
   Q_ENUMS (Separators)


   /// \enum Notations
   /// User friendly enumerations for notation property - refer to notations
   /// for details.
   ///
   enum Notations {
      Fixed = 0,        ///< Standard floating point, e.g. 123456.789
      Scientific,       ///< Scientific representation, e.g. 1.23456789e6
      Automatic         ///< Automatic choice of standard or scientific notation
   };
   Q_ENUMS (Notations)


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
                        ///  to '?' except for trailing zeros (ignore them)
      Index             ///< Interpret the element selected by setArrayIndex()
                        ///  as the value.
   };
   Q_ENUMS (ArrayActions)


   /// \enum UpdateOptions
   /// Determines if data updates the label text, or the label pixmap.
   /// For both options all normal string formatting is applied.
   /// If Text, the formatted text is simply presented as the label text.
   /// If Picture, the FORMATTED text is then interpreted as an integer and
   /// used to select one of the pixmaps specified by properties pixmap0
   /// through to pixmap7.
   ///
   enum UpdateOptions {
      Text     = 0,     ///< Data updates will update the label text
      Picture           ///< Data updates will update the label icon
   };
   Q_ENUMS (UpdateOptions)

   //---------------------------------------------------------------------------
   // Buttons/QMenuButton
   //
   /// Specifies how a (control) widget should behave if/when the underlying
   /// record becomes disabled (i.e. DISA and DISV fields become equal).
   ///
   enum DisabledRecordPolicy {
      ignore = 0,       ///< No change in appearance - the default
      grayout,          ///< Grayed out apperance, i.e. same as when the widget's PV is disconnected
      disable           ///< Widget is disabled.
   };
   Q_ENUMS (DisabledRecordPolicy)


   /// \enum ProgramStartupOptionNames
   /// Startup options. Just run the command, run the command within a terminal,
   /// or display the output in QE message system.
   ///
   enum ProgramStartupOptionNames {
      NoOutput = 0,     ///< Just run the program - was None
      Terminal,         ///< Run the program in a termainal (in Windows a command
      ///  interpreter will also be started, so the program may
      ///  be a built-in command like 'dir')
      LogOutput,        ///< Run the program, and log the output in the QE message system
      StdOutput         ///< Run the program, and send doutput to standard output and standard error
   };
   Q_ENUMS (ProgramStartupOptionNames)


   /// \enum CreationOptionNames
   /// Creation options. Used to indicate how to present a GUI when requesting
   /// a new GUI be created. Open a new window, open a new tab, or replace the
   /// current window.
   ///
   enum CreationOptionNames {
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
   Q_ENUMS (CreationOptionNames)


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
   Q_ENUMS (DropOption)


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
   Q_ENUMS (ImageFormatOptions)


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
   Q_ENUMS (RotationOptions)


   /// \enum PVLabelMode
   /// The default is useAliasName, however the default aliasName are undefined,
   /// so the effective default is usePvName.
   ///
   enum PVLabelMode {
      usePvName,         ///< display the PV name
      useAliasName,      ///< display alias if defined otherwise display the PV name
      useDescription     ///< display description if defined otherwise display the PV name
   };
   Q_ENUMS (PVLabelMode)


   //---------------------------------------------------------------------------
   // Miscellaneous
   //
   /// \enum    Orientations
   /// The orientation of indicators
   ///
   enum Orientations {
      Left_To_Right = 0,   ///< Left to right
      Top_To_Bottom,       ///< Top to bottom
      Right_To_Left,       ///< Right to left
      Bottom_To_Top        ///< Bottom to top
   };
   Q_ENUMS (Orientations)

   /// \enum    optionsLayoutProperty
   /// Provides the location of items within a widget
   ///
   enum optionsLayoutProperty {
      Top = 0,          ///<
      Bottom,           ///<
      Left,             ///<
      Right             ///<
   };
   Q_ENUMS (optionsLayoutProperty)


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
      RowMajor = 0,        ///<
      ColMajor             ///<
   };
   Q_ENUMS (GridOrders)

private:
   explicit QE (QObject* parent = 0) : QObject (parent) {}
   ~QE () {}
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QE::UserLevels)
Q_DECLARE_METATYPE (QE::DisplayAlarmStateOptions)
Q_DECLARE_METATYPE (QE::Formats)
Q_DECLARE_METATYPE (QE::Separators)
Q_DECLARE_METATYPE (QE::Notations)
Q_DECLARE_METATYPE (QE::ArrayActions)
Q_DECLARE_METATYPE (QE::UpdateOptions)
Q_DECLARE_METATYPE (QE::DisabledRecordPolicy)
Q_DECLARE_METATYPE (QE::ProgramStartupOptionNames)
Q_DECLARE_METATYPE (QE::CreationOptionNames)
Q_DECLARE_METATYPE (QE::DropOption)
Q_DECLARE_METATYPE (QE::ImageFormatOptions)
Q_DECLARE_METATYPE (QE::RotationOptions)
Q_DECLARE_METATYPE (QE::PVLabelMode)
Q_DECLARE_METATYPE (QE::Orientations)
Q_DECLARE_METATYPE (QE::optionsLayoutProperty)
Q_DECLARE_METATYPE (QE::GridOrders)
#endif

#endif  // QE_ENUMS_H
