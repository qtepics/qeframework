/*  VariableNameManager.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2009-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_VARIABLE_NAME_MANAGER_H
#define QE_VARIABLE_NAME_MANAGER_H

#include <QString>
#include <QStringList>
#include <QEFrameworkLibraryGlobal.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT VariableNameManager {
public:
   explicit VariableNameManager ();
   virtual ~VariableNameManager ();

   // Used to set up the number of variables required (defaults to 1 if this is not called).
   void variableNameManagerInitialise (unsigned int numVariables);

   // Returns number of variables that can be set up for this object.
   int getNumberVariables () const;

   // Return variable name prior to any macro substitutions (eg, SR$SECTOR$V ).
   QString getOriginalVariableName (unsigned int variableIndex) const;

   // Return macro substitutions used for variable names (eg, SECTOR=01,V=PRESURE).
   QString getVariableNameSubstitutions () const;

   // Return variable name after macro substitutions.
   QString getSubstitutedVariableName (unsigned int variableIndex) const;

   void setVariableNameSubstitutionsOverride (const QString& substitutions);

   // Accept a new variable name which may include substitution keys preceeded by $.
   void setVariableName (const QString& variableName, unsigned int variableIndex);

   // Accept a new set of macro substitutions in the form KEY1=VALUE1,KEY2=VALUE2.
   void setVariableNameSubstitutions (const QString& substitutions);

   // Perform the macro substitutions on a string. Used internaly for
   // variable names, but can be used for any string.
   QString substituteThis (const QString& string) const;

private:
   QString doSubstitution (unsigned int variableIndex) const;

   QString macroSubstitutions;
   QString macroSubstitutionsOverride;
   QStringList variableNames;
};

#endif  // QE_VARIABLE_NAME_MANAGER_H
