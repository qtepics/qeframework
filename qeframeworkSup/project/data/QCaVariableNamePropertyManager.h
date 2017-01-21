/*  QCaVariableNamePropertyManager.h
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
 *  Copyright (c) 2009, 2010, 2014, 2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

// Handles variable name sets.

#ifndef QCA_VARIABLE_NAME_PROPERTY_MANAGER_H
#define QCA_VARIABLE_NAME_PROPERTY_MANAGER_H

#include <QTimer>
#include <QString>
#include <QEPluginLibrary_global.h>

class QEPLUGINLIBRARYSHARED_EXPORT QCaVariableNamePropertyManager : public QTimer {
    Q_OBJECT

  public:
    QCaVariableNamePropertyManager();

    QString getVariableNameProperty() const;
    void setVariableNameProperty( QString variableNamePropertyIn );

    QString getSubstitutionsProperty() const;
    void setSubstitutionsProperty( QString substitutionsPropertyIn );

    void setVariableIndex( unsigned int variableIndexIn );
    unsigned int getVariableIndex() const;

signals:
    void newVariableNameProperty( QString variable, QString Substitutions, unsigned int variableIndex );

private slots:
    void subscribeDelayExpired();       // Called a short time after a user stops typing in 'designer'


private:
    QString variableNameProperty;
    QString substitutionsProperty;

    unsigned int variableIndex;     // Index into the list of variable names maintained by a QE widgets
    bool interactive;   // If set, there is a user typing new variable names and macro substitutions. Use timers to wait for typing to finish.
};

#endif // QCA_VARIABLE_NAME_PROPERTY_MANAGER_H
