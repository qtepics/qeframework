/*  VariableNameManager.cpp
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

/*
 * Description:
 *
 * A class to manage changes to one or more variable names and the macro
 * substitutions applying to the variable names.
 *
 */

#include "VariableNameManager.h"
#include <ContainerProfile.h>
#include <QDebug>
#include <macroSubstitution.h>

#define DEBUG qDebug () << "VariableNameManager" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
VariableNameManager::VariableNameManager ()
{
   // Assume one variable name.
   this->variableNameManagerInitialise (1);
}

//------------------------------------------------------------------------------
//  place holder
VariableNameManager:: ~VariableNameManager () {}

//------------------------------------------------------------------------------
// Define the required number of variables to manage.
// The list of variable names is initially conatins a single variable name so this
// need only be called if more than one variable name is required.
//
void VariableNameManager::variableNameManagerInitialise (unsigned int numVariables)
{
   // Sanity check. List must contain at least one variable name.
   /// WHY?
   //
   if (numVariables < 1)
      return;

   // Clear out any existing variables.
   this->variableNames.clear ();

   // Create the required number of (empty) variables.
   //
   for (unsigned int i = 0; i < numVariables; i++) {
      this->variableNames.append (QString ());
   }
}

//------------------------------------------------------------------------------
// Get the number of variables that can be set up for this object.
//
int VariableNameManager::getNumberVariables () const
{
   return this->variableNames.size ();
}

//------------------------------------------------------------------------------
// Get the current variable name.
//
QString VariableNameManager::getOriginalVariableName (unsigned int variableIndex) const
{
   // Sanity check
   if (variableIndex >= (unsigned int) this->variableNames.size ())
      return "";

   // Return the original variable name or
   return this->variableNames.value (variableIndex, "");
}

//------------------------------------------------------------------------------
// Get the current variable name substitutions.
// Note the substitutions for the first variable are always returned as
// the same substitutions are used for every entry in the variableNames list.
//
QString VariableNameManager::getVariableNameSubstitutions () const
{
   return this->macroSubstitutions;
}

//------------------------------------------------------------------------------
// Get the current variable name with substitutions applied.
//
QString VariableNameManager::getSubstitutedVariableName (unsigned int variableIndex) const
{
   // Sanity check
   if (variableIndex >= (unsigned int) this->variableNames.size ())
      return "";

   // Perform the substitution
   return this->doSubstitution (variableIndex);
}

//------------------------------------------------------------------------------
// Override variable name substitutions.
// This is called when any macro substitutions set by default are overridden
// by the creator.
//
void VariableNameManager::
setVariableNameSubstitutionsOverride (const QString& macroSubstitutionsOverrideIn)
{
   this->macroSubstitutionsOverride = macroSubstitutionsOverrideIn;
}

//------------------------------------------------------------------------------
// Set the variable name.
// Macro substitution will be performed.
// A new connection is established.
//
void VariableNameManager::setVariableName (const QString& variableName,
                                           unsigned int variableIndex)
{
   // Sanity check
   if (variableIndex >= (unsigned int) this->variableNames.size ())
      return;

   // Save the variable name and request the variableName data if updates are required.
   //
   this->variableNames.replace(variableIndex, variableName);
}

//------------------------------------------------------------------------------
// Set the variable name substitutions.
// Note, if there is more than one variable name in the list, the same
// substitutions are used for every entry in the variableNames list.
// Macro substitution will be performed.
// A new connection is established.
//
void VariableNameManager::setVariableNameSubstitutions (const QString& macroSubstitutionsIn)
{
   this->macroSubstitutions = macroSubstitutionsIn;
}

//------------------------------------------------------------------------------
//  Perform a set of substitutions throughout a variable name.
//  Replace $MACRO1 with VALUE1, $MACRO2 with VALUE2, etc wherever they appear in the string.
//
QString VariableNameManager::doSubstitution (unsigned int variableIndex) const
{
   // Sanity check
   if (variableIndex >= (unsigned int) this->variableNames.size ())
      return "";

   // Start with the initial string
   QString result = this->variableNames.value (variableIndex, "");

   // Perform the required substitutions on the variable name.
   return this->substituteThis (result);
}

//------------------------------------------------------------------------------
// Perform a set of substitutions throughout a string.
// Replace $MACRO1 with VALUE1, $MACRO2 with VALUE2, etc wherever they appear in the string.
//
QString VariableNameManager::substituteThis (const QString& string) const
{
   // Generate a list where each item in the list is a single substitution
   // in the form MACRO1=VALUE1.
   //
   QString subs;
   subs.append (this->macroSubstitutionsOverride).append (",").append (this->macroSubstitutions);

   //!!! for efficiency, should this be done when substitutions are added or removed??
   //!!! Build a list of keys and values...
   // Parse the substitutions
   macroSubstitutionList parts = macroSubstitutionList (subs);

   // return the string with substitutions applied.
   return parts.substitute (string);
}

// end
