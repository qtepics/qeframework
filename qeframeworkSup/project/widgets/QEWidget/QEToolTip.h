/*  QEToolTip.h
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

/// QE widget tool tips may be set to display the names of the process variables
/// supplying data to the widget and the alarm state and connectino status of those variables.
/// The QE widget may also set some custom text to be displayed along with this information.
/// The QEToolTip class manages building and setting the QE widget tool tips when this functino is required.

#ifndef QE_TOOL_TIP_H
#define QE_TOOL_TIP_H

#include <QEvent>
#include <QList>
#include <QObject>
#include <QWidget>
#include <QString>
#include <QStringList>
#include <QEString.h>
#include <QEStringFormatting.h>

#include <QEFrameworkLibraryGlobal.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEToolTip
{
    public:
        QEToolTip( QWidget* ownerIn );
        virtual ~QEToolTip();
        void setNumberToolTipVariables ( const unsigned int number );
        void updateToolTipVariable ( const QString& variable, const unsigned int variableIndex );   // Update a variable name
        void updateToolTipAlarm ( const QString& alarm, const unsigned int variableIndex );         // Update an alarm state
        void updateToolTipConnection ( bool connection, const unsigned int variableIndex = 0 );     // Update a connection status
        void updateToolTipCustom ( const QString& custom );                                         // Update the customisable part of the tooltip

        void setVariableAsToolTip( const bool variableAsToolTip );                                  // Set flag indicating variable names should be in the tool tip
        bool getVariableAsToolTip() const;                                                          // Get the flag indicating variable names should be in the tool tip

    private:
        void updateToolTipDescription ( const QString& desc, const unsigned int variableIndex );    // Update description
        bool variableAsToolTip;         // Flag the tool tip should be set to the variable name
        void displayToolTip();          // Built a tool tip from all the required information and set it
        unsigned int number;            // Count of variables that will be included in the tooltip
        QStringList toolTipVariable;    // List of variable names to be included in the tooltip
        QStringList toolTipDescription; // List of variable descriptions to be included in the tooltip
        QStringList toolTipAlarm;       // List of alarm states to be included in the tool tip (same size as toolTipVariable)
        QList<bool> toolTipIsConnected; // List of connection status to be included in the tool tip (same size as toolTipVariable)
        QString toolTipCustom;          // Custion tool tip extra for specific widget types
        QWidget* owner;                 // Widget whos tool tip will be updated

        friend class QEToolTipSingleton;
};


// This is a singleton class - the single instance is declared in the .cpp file.
// It's only exposed in a header because the Qt SDK framework requires that signals
// and slots are declared in header files. Clients should use the QEToolTip
// specified above.
//
class QEWidget;

class QEToolTipSingleton : public QObject {
    Q_OBJECT
private:
    static void constructSingleton();

    explicit QEToolTipSingleton( QObject* parent = 0 );
    ~QEToolTipSingleton();

    void clear ();

    void registerWidget( QWidget* widget );
    void deregisterWidget( QWidget* widget );

    void enterQEWidget ( QEWidget* qewidget );
    void leaveQEWidget ( QEWidget* qewidget );
    bool eventFilter( QObject* obj, QEvent* event );

    typedef QList<QEString*> QEStringList;

    QEStringFormatting descriptionFormatting;
    QEStringList descriptionStringList;
    QEWidget* currentQEWidget;

private slots:
    void descriptionUpdate (const QString& value, QCaAlarmInfo& alarmInfo,
                            QCaDateTime& dateTime, const unsigned int& variableIndex);

    friend class QEToolTip;
};

#endif // QE_TOOL_TIP_H
