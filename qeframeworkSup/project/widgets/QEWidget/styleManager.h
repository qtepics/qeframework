/*  styleManager.h
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
 *  Copyright (c) 2009, 2010 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#ifndef STYLEMANAGER_H
#define STYLEMANAGER_H

#include <ContainerProfile.h>

#include <QEPluginLibrary_global.h>

class changeEventFilter; // Forward declaration

/*!
  This class adds common style support to all QE widgets if required.

  Standard properties for all QE widgets specify a style to be applied for user, scientist, and engineer mode.
  Also QE widgets can specify data or status related style changes.

  The syntax for all Style Sheet strings used by this class is the standard Qt Style Sheet syntax.
  For example, 'background-color: red'. Refer to Qt Style Sheets Reference for full details.

  Note, as well as the large number of defined Style Sheet properties, the Style Sheet syntax allows setting any widget property using the 'qproperty' property.
  For example 'qproperty-geometry:rect(10 10 100 100);'
  Caution, any amount of weird behaviour can be effected using Style Sheet strings.
  Without carefull consideration they should only be used for simple visualisation effects such as altering the backgrouond color.

  All QE widgets (eg, QELabel, QELineEdit) have an instance of this class as they based on
  QEWidget which itself uses this class as a base class.

  To use the functionality provided by this class, QE widgets must include the following:

    \li Q_PROPERTY(QString styleSheet READ styleSheet WRITE setStyleSheet  DESIGNABLE false)
    \li Q_PROPERTY(QString defaultStyle READ getStyleDefault  WRITE setStyleDefault)
    \li Q_PROPERTY(QString userLevelUserStyle READ getStyleUser WRITE setStyleUser)
    \li Q_PROPERTY(QString userLevelScientistStyle READ getStyleScientist WRITE setStyleScientist)
    \li Q_PROPERTY(QString userLevelEngineerStyle READ getStyleEngineer WRITE setStyleEngineer)

  To use this class to manage style changes related to data and status the QE widget must include the following:

    \li On presenation of new data call updateDataStyle() with the Style Sheet string related to the new data (if any).
    \li On change of data status, call updateStatusStyle() with the Style Sheet string related to the new status (if any).

  Note, this class notes the initial style when instantiated and uses that style as the base style for
  all style changes. This means any style changes not performed through this class will be lost the next
  time this class changes the style.

  Note, the stylesheet built by this class is not actually applied if the widget being managed is disabled.
  Instead it is noted and applied if and when the widget becomed enabled.
  Changes that affect the style will still cause a regeneration of the style while the widget is disabled, but the
  updated style will not be applied until the widget is enabled.
*/

class QEPLUGINLIBRARYSHARED_EXPORT styleManager {
    friend class changeEventFilter; // The event filter is really part of this style manager class

public:
    styleManager( QWidget* ownerIn );
    ~styleManager();

    void setStyleDefault( QString style );  //!< Set the default Style Sheet string.
                                            //!< The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'.

    QString getStyleDefault() const;        //!< Get the default Style Sheet string.
                                            //!<

    void setStyleUser( QString style );     //!< Set the Style Sheet string to be applied when the widget is displayed in 'User' mode.
                                            //!< The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'.

    QString getStyleUser() const;           //!< Get the Style Sheet string to be applied when the widget is displayed in 'User' mode.
                                            //!<

    void setStyleScientist( QString style );//!< Set the Style Sheet string to be applied when the widget is displayed in 'Scientist' mode.
                                            //!< The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'

    QString getStyleScientist() const;      //!< Get the Style Sheet string to be applied when the widget is displayed in 'Scientist' mode.
                                            //!<

    void setStyleEngineer( QString style ); //!< Set the Style Sheet string to be applied when the widget is displayed in 'Engineer' mode.
                                            //!< The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'

    QString getStyleEngineer() const;       //!< Get the Style Sheet string to be applied when the widget is displayed in 'Engineer' mode.
                                            //!<

    void updateDataStyle( QString style );  //!< Set the Style Sheet string to be applied to reflect an aspect of the current data.
                                            //!< For example, a value over a high limit may be displayed in red.

    void updateStatusStyle( QString style );//!< Set the Style Sheet string to be applied to reflect an aspect of the current status.
                                            //!< For example, invalid data may be displayed with a white background.

    void updatePropertyStyle( QString style );//!< Set the Style Sheet string to be applied to implement a widget property.
                                            //!< For example, a style string is used to set QE button text alignment.

    void updateConnectionStyle( bool connected );  //!< Set the Style Sheet string to be applied to reflect the current connection state (connected or disconnected) of the current data.
                                                   //!< For example, a disconnected value may be greyed out.

    void styleUserLevelChanged( userLevelTypes::userLevels levelIn );/**< Set the current user level.*/

private:
    void enabledChange();               // Called to notify the manager that the enabled state of the widget has changed

    QWidget* owner;                     // Widget to which style sheet strings will be applied
    QString currentStyle;               // Current style sheet (or that that will be applied when not disabled). This is kept up to date as components change even if it not being applied to the widget due to the widget being disabled.

    QString userUserStyle;              // Style to apply to widget when current user is a 'user'
    QString userScientistStyle;         // Style to apply to widget when current user is a 'scientist'
    QString userEngineerStyle;          // Style to apply to widget when current user is a 'engineer'

    QString defaultStyleSheet;          // Style sheet prior to any manipulation by this class
    QString statusStyleSheet;           // Style to apply to reflect current status
    QString dataStyleSheet;             // Style to apply to reflect current data
    QString propertyStyleSheet;         // Style to apply to implement a QE widget property
    QString connectionStyleSheet;       // Style to apply to reflect current connection state

    void updateStyleSheet();            // Update the style sheet with the various style sheet components used to modify the label style (alarm info, enumeration color)

    userLevelTypes::userLevels level;   // Current user level - used to select appropriate user style

    changeEventFilter* eventFilter;     // Event filter to catch enables and disabled (all styles are removed when disabled)
};

// Event filter that will be added to the widget mening managed by the styleManager class.
// This filter will be used to catch change events to keep a track of the enabled/disabled
// state of the widget as the style is only applied if the widget is enabled allowing the
// full disabled look to be displayed.
// Note, the filter functionality cant be added to the styleManager class itself
// as it is not a QObject (and can't be as it is a base class to QE widgets and
// there can't be more than a single base class that is a QObject)
class changeEventFilter : public QObject
{
    Q_OBJECT

public:
    changeEventFilter( styleManager* managerIn ){ manager = managerIn; }

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    styleManager* manager;  // Events are passed back to the manager
};


#endif // STYLEMANAGER_H
