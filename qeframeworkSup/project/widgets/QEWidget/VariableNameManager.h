/*  VariableNameManager.h
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

#ifndef VARIABLENAMEMANAGER_H
#define VARIABLENAMEMANAGER_H

#include <QString>
#include <QList>
#include <QEPluginLibrary_global.h>

class QEPLUGINLIBRARYSHARED_EXPORT VariableNameManager {

public:
    VariableNameManager();
    virtual ~VariableNameManager(){}

    void variableNameManagerInitialise( unsigned int numVariables ) ;       // Used to set up the number of variables required (defaults to 1 if this is not called)

    int getNumberVariables() const;                                         // Returns number of variables that can be set up for this object.
    QString getOriginalVariableName( unsigned int variableIndex ) const;    // Return variable name prior to any macro substitutions. (eg, SR$SECTOR$V )
    QString getVariableNameSubstitutions() const;                           // Return macro substitutions used for variable names (eg, SECTOR=01,V=PRESURE)
    QString getSubstitutedVariableName(unsigned int variableIndex ) const;  // Return variable name after macro substitutions

    void setVariableNameSubstitutionsOverride( const QString& substitutions );

    void setVariableName( const QString& variableName, unsigned int variableIndex );    // Accept a new variable name which may include substitution keys preceeded by $
    void setVariableNameSubstitutions( const QString& substitutions );                  // Accept a new set of macro substitutions in the form KEY1=VALUE1,KEY2=VALUE2

    QString substituteThis( const QString string ) const;                   // Perform the macro substitutions on a string. used internaly for variable names, but can be used for any string

  private:
    QString doSubstitution( unsigned int variableIndex ) const;
    void substituteKey( QString& string, QString key, const QString value );


    QString macroSubstitutions;
    QString macroSubstitutionsOverride;

    QList<QString> variableNames;
};

#endif // VARIABLENAMEMANAGER_H
