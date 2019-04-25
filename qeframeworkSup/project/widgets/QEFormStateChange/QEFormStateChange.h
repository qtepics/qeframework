/*  QEFormStateChange.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2019 Australian Synchrotron.
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
 *    andrews@ansto.gov.au
 */

#ifndef QE_FORM_STATE_CHANGE_H
#define QE_FORM_STATE_CHANGE_H

#include <QWidget>
#include <QMainWindow>
#include <QEWidget.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEStringFormatting.h>
#include <QEStringFormattingMethods.h>
#include <applicationLauncher.h>
#include <QEFrameworkLibraryGlobal.h>

/// This widget is capable of responding to window/form open and close events.
/// On open and/or close the widget can write to a PV and/or executate an arbitary
/// local program/scripts. This is not unlike automatically clicking a QEPushButton
/// on open/close, save that there is no option to open another ui file.
/// Note: the widget itself non-visible by default.
///
// TODO?? Capture form minimised amd maximised as well.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEFormStateChange :
   public QWidget,
   public QEWidget,
   public QEStringFormattingMethods
{
   Q_OBJECT

public:
   /// The variable to write to on open
   Q_PROPERTY (QString variableOpen
               READ getOpenVariableNameProperty
               WRITE setOpenVariableNameProperty)

   /// The variable to write to on close. May be the same as the open variable.
   Q_PROPERTY (QString variableClose
               READ getCloseVariableNameProperty
               WRITE setCloseVariableNameProperty)

   /// The default variable subsitutions - same for both open and close variables.
   Q_PROPERTY (QString variableSubstitutions
               READ getVariableNameSubstitutionsProperty
               WRITE setVariableNameSubstitutionsProperty)

   /// \enum  Formats
   /// User friendly enumerations for format property - refer to QEStringFormatting::formats for details.
   /// Not all formats are available.
   ///
   enum Formats { Default          = QEStringFormatting::FORMAT_DEFAULT,            ///< Format as best appropriate for the data type
                  Floating         = QEStringFormatting::FORMAT_FLOATING,           ///< Format as a floating point number
                  Integer          = QEStringFormatting::FORMAT_INTEGER,            ///< Format as an integer
                  UnsignedInteger  = QEStringFormatting::FORMAT_UNSIGNEDINTEGER,    ///< Format as an unsigned integer
                  String           = QEStringFormatting::FORMAT_STRING              ///< Format as a string
               };
   Q_ENUMS(Formats)

   /// Format to apply to data. Default is 'Default' in which case the data type
   /// supplied with the data determines how the data is formatted.
   /// For all other options, an attempt is made to format the data as requested
   /// (whatever its native form).
   /// NOTE: The same format value applies to both the open and close PV formatting
   ///
   Q_PROPERTY (Formats format             READ getFormatProperty WRITE setFormatProperty)

   Q_PROPERTY (QString openText           READ getOpenText       WRITE setOpenText)       /// value to write on open
   Q_PROPERTY (QString openProgram        READ getOpenProgram    WRITE setOpenProgram)    /// program to run
   Q_PROPERTY (QStringList openArguments  READ getOpenArguments  WRITE setOpenArguments)  /// program arguments

   Q_PROPERTY (QString closeText          READ getCloseText      WRITE setCloseText)      /// value to write on close
   Q_PROPERTY (QString closeProgram       READ getCloseProgram   WRITE setCloseProgram)   /// program to run
   Q_PROPERTY (QStringList closeArguments READ getCloseArguments WRITE setCloseArguments) /// program arguments

   /// Not sure how usefull it would be to have this widget visible at run time,
   /// but let's not second guess the users' needs and desires.
   Q_PROPERTY (bool runVisible            READ getRunVisible     WRITE setRunVisible)

public:
   enum VariableIndicies {
      viOpen = 0,
      viClose,
      NUMBER_OF_VARIABLES    // must be last
   };

   explicit QEFormStateChange (QWidget* parent = 0);
   explicit QEFormStateChange (const QString &openVariableName,
                               QWidget* parent = 0);
   explicit QEFormStateChange (const QString &openVariableName,
                               const QString &closeVariableName,
                               QWidget* parent = 0);
   ~QEFormStateChange();

   // Property access functions.
   //
   void setOpenVariableNameProperty(const QString value);
   QString getOpenVariableNameProperty() const;

   void setOpenText(const QString text);
   QString getOpenText() const;

   void setOpenProgram(const QString program);
   QString getOpenProgram() const;

   void setOpenArguments(const QStringList arguments);
   QStringList getOpenArguments() const;

   void setCloseVariableNameProperty(const QString text);
   QString getCloseVariableNameProperty() const;

   void setCloseText(const QString value);
   QString getCloseText() const;

   void setCloseProgram(const QString program);
   QString getCloseProgram() const;

   void setCloseArguments(const QStringList arguments);
   QStringList getCloseArguments() const;

   void setVariableNameSubstitutionsProperty(const QString value);
   QString getVariableNameSubstitutionsProperty() const;

   void setFormatProperty (const Formats format);
   Formats getFormatProperty () const;

protected:
   QSize sizeHint ();
   void paintEvent (QPaintEvent* event);   // draw something as design time

   qcaobject::QCaObject* createQcaItem (unsigned int variableIndex);
   void establishConnection (unsigned int variableIndex);
   void stringFormattingChange ();         // must be defined, but not used.

private:
   void setup ();

   class Actions;  // differed
   Actions* actionList [NUMBER_OF_VARIABLES];

private slots:
   void windowOpened();   // actually called when object created
   void windowClosed();   // and deleted.
   void newVariableNameProperty (QString variableNameIn,
                                 QString variableNameSubstitutionsIn,
                                 unsigned int variableIndex);
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEFormStateChange::Formats)
#endif

#endif // QE_FORM_STATE_CHANGE_H
