/*  QEGroupBox.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2012-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_GROUP_BOX_H
#define QE_GROUP_BOX_H

#include <QGroupBox>
#include <QEEnums.h>
#include <QEWidget.h>
#include <QEFrameworkLibraryGlobal.h>

// The QEGroupBox class provides a minimalist extention to the QGroupBox class
// in that it provides user level enabled and user level visibility control to
// the group box but more significantly to all the widgets enclosed within the
// QEGroupBox container also.
// See QEFrame as well.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEGroupBox :
      public QGroupBox, public QEWidget
{
Q_OBJECT

    // QEGroupBox specific properties ==========================================
    //
    /// Group box title text to be substituted.
    /// This text will be copied to the group box title text after applying any
    /// macro substitutions from the variableSubstitutions property.
    /// The ownSubstitutedTitle property is depricated and included for backward
    /// compatabilty only and not presented on designer.
    /// The title property hides the parent title property.
    ///
    Q_PROPERTY (QString ownSubstitutedTitle READ getSubstitutedTitleProperty WRITE setSubstitutedTitleProperty  DESIGNABLE false)
    Q_PROPERTY (QString title               READ getSubstitutedTitleProperty WRITE setSubstitutedTitleProperty)

    /// Text substitutions.
    /// These substitutions are applied to the 'substitutedTitle' property prior to copying it to the label text.
    Q_PROPERTY(QString textSubstitutions READ getSubstitutionsProperty WRITE setSubstitutionsProperty)
    //
    // End QEGroupBox specific properties ======================================


    // Standard properties less tooltip, allowDrop, use alarm state, oos aware.
    //
    // BEGIN-STANDARD-V2-PROPERTIES ===================================================
    // Standard properties
    // These properties should be identical for every widget using them.
    // WHEN MAKING CHANGES: Use the update_widget_properties script in the
    // resources directory.
public slots:
    /// Slot to set the visibility of a QE widget, taking into account the user level.
    /// Widget will be hidden if hidden by a call this slot, by will only be made
    /// visible by a calll to this slot if the user level allows.
    ///
    void setManagedVisible (bool v) { this->setRunVisible(v); }

public:
    /// variableAsToolTip not applicale to V2 options.
    /// Class should ensure this option is initialised to false.

    /// allowDrop not applicale to V2 options.
    /// Class should ensure this option is initialised to false.

    /// Display the widget. Default is true.
    /// Setting this property false is usefull if widget is only used to provide a signal - for example, when supplying data to a QELink widget.
    /// Note, when false the widget will still be visible in Qt Designer.
    Q_PROPERTY(bool visible READ getRunVisible WRITE setRunVisible)

    /// Set the ID used by the message filtering system. Default is zero.
    /// Widgets or applications that use messages from the framework have the option of filtering on this ID.
    /// For example, by using a unique message source ID a QELog widget may be set up to only log messages from a select set of widgets.
    Q_PROPERTY(unsigned int messageSourceId READ getMessageSourceId WRITE setMessageSourceId )

    /// Hide style sheet from designer as style calculation by the styleManager and not directly setable per se.
    /// This also stops transient styles being saved to the ui file.
    Q_PROPERTY(QString styleSheet   READ styleSheet       WRITE setStyleSheet  DESIGNABLE false)

    /// Style Sheet string to be applied before, i.e. lower priority than, any other style, e.g. alarm style and/or user level style.
    /// Default is an empty string.
    Q_PROPERTY(QString defaultStyle READ getStyleDefault  WRITE setStyleDefault)

    /// Style Sheet string to be applied when the widget is displayed in 'User' mode. Default is an empty string.
    /// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
    /// This Style Sheet string will be applied by the styleManager class.
    /// Refer to the styleManager class for details about how this Style Sheet string will be merged with any pre-existing Style Sheet string
    /// and any Style Sheet strings generated during the display of data.
    Q_PROPERTY(QString userLevelUserStyle READ getStyleUser WRITE setStyleUser)

    /// Style Sheet string to be applied when the widget is displayed in 'Scientist' mode. Default is an empty string.
    /// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
    /// This Style Sheet string will be applied by the styleManager class.
    /// Refer to the styleManager class for details about how this Style Sheet string will be merged with any pre-existing Style Sheet string
    /// and any Style Sheet strings generated during the display of data.
    Q_PROPERTY(QString userLevelScientistStyle READ getStyleScientist WRITE setStyleScientist)

    /// Style Sheet string to be applied when the widget is displayed in 'Engineer' mode. Default is an empty string.
    /// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
    /// This Style Sheet string will be applied by the styleManager class.
    /// Refer to the styleManager class for details about how this Style Sheet string will be merged with any pre-existing Style Sheet string
    /// and any Style Sheet strings generated during the display of data.
    Q_PROPERTY(QString userLevelEngineerStyle READ getStyleEngineer WRITE setStyleEngineer)

    /// Lowest user level at which the widget is visible. Default is 'User'.
    /// Used when designing GUIs that display more and more detail according to the user mode.
    /// The user mode is set application wide through the QELogin widget, or programatically through setUserLevel()
    /// Widgets that are always visible should be visible at 'User'.
    /// Widgets that are only used by scientists managing the facility should be visible at 'Scientist'.
    /// Widgets that are only used by engineers maintaining the facility should be visible at 'Engineer'.
    ///
    Q_PROPERTY(QE::UserLevels userLevelVisibility READ getUserLevelVisibility WRITE setUserLevelVisibility)

    /// Lowest user level at which the widget is enabled. Default is 'User'.
    /// Used when designing GUIs that allow access to more and more detail according to the user mode.
    /// The user mode is set application wide through the QELogin widget, or programatically through setUserLevel()
    /// Widgets that are always accessable should be visible at 'User'.
    /// Widgets that are only accessable to scientists managing the facility should be visible at 'Scientist'.
    /// Widgets that are only accessable to engineers maintaining the facility should be visible at 'Engineer'.
    ///
    Q_PROPERTY(QE::UserLevels userLevelEnabled READ getUserLevelEnabled WRITE setUserLevelEnabled)

    /// displayAlarmStateOption not applicale to V2 options.
    /// Class should ensure this option is initialised to DISPLAY_ALARM_STATE_NEVER.

public:
    // END-STANDARD-V2-PROPERTIES =====================================================

public:
    explicit QEGroupBox (QWidget *parent = 0);
    explicit QEGroupBox (const QString &title, QWidget* parent=0);

    virtual ~QEGroupBox ();
    QSize sizeHint () const;

protected:
    // Make accessable/overridable by derived classes.
    //
    virtual void setSubstitutionsProperty (const QString macroSubstitutions);
    virtual QString getSubstitutionsProperty () const;

    virtual void setSubstitutedTitleProperty (const QString ownSubstitutedTitle);
    virtual QString getSubstitutedTitleProperty () const;

private:
    QString ownSubstitutedTitle;
};

#endif     // QE_GROUP_BOX_H
