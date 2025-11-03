/*  VariableNameManager.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2025 Australian Synchrotron
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

#include <VariableNameManager.h>
#include <ContainerProfile.h>
#include <macroSubstitution.h>
#include <QStringList>
#include <QtDebug>

/*
    Assume one variable name.
*/
VariableNameManager::VariableNameManager() {

   // Assume one variable name.
   variableNameManagerInitialise( 1 );
}

/*
    Define the required number of variables to manage.
    The list of variable names is initially conatins a single variable name so this
    need only be called if more than one variable name is required.
*/
void VariableNameManager::variableNameManagerInitialise( unsigned int numVariables ) {

   // Sanity check. List must contain at least one variable name
   if( numVariables < 1 )
      return;

   // Clear out any existing variables
   variableNames.clear();

   // Create the required number of variables
   for( unsigned int i = 0; i < numVariables; i++ ) {
      variableNames.append( QString() );
   }
}

/*
    Get the number of variables that can be set up for this object.
 */
int VariableNameManager::getNumberVariables () const {
   return variableNames.size();
}

/*
    Get the current variable name.
    Supply a variable index if this class is managing more than one variable
    name.
*/
QString VariableNameManager::getOriginalVariableName( unsigned int variableIndex ) const {

   // Sanity check
   if( variableIndex >= (unsigned int )variableNames.size() )
      return "";

   // Return the original variable name
   return variableNames[variableIndex];
}

/*
    Get the current variable name substitutions.
    Note the substitutions for the first variable are always returned as
    the same substitutions are used for every entry in the variableNames list.
*/
QString VariableNameManager::getVariableNameSubstitutions() const {

   return macroSubstitutions;
}

/*
    Get the current variable name with substitutions applied.
*/
QString VariableNameManager::getSubstitutedVariableName( unsigned int variableIndex ) const {

   // Sanity check
   if( variableIndex >= (unsigned int )variableNames.size() )
      return "";

   // Perform the substitution
   return doSubstitution( variableIndex );
}

/*
    Override variable name substitutions.
    This is called when any macro substitutions set by default are overridden by the creator.
*/
void VariableNameManager::setVariableNameSubstitutionsOverride( const QString& macroSubstitutionsOverrideIn ) {

   macroSubstitutionsOverride = macroSubstitutionsOverrideIn;
}

/*
    Set the variable name.
    Macro substitution will be performed.
    A new connection is established.
*/
void VariableNameManager::setVariableName( const QString& variableNameIn, unsigned int variableIndex ) {

   // Sanity check
   if( variableIndex >= (unsigned int )variableNames.size() )
      return;

   // Save the variable name and request the variableName data if updates are required
   variableNames[variableIndex] = variableNameIn;
}

/*
    Set the variable name substitutions.
    Note, if there is more than one variable name in the list, the same
    substitutions are used for every entry in the variableNames list.
    Macro substitution will be performed.
    A new connection is established.
*/
void VariableNameManager::setVariableNameSubstitutions( const QString& macroSubstitutionsIn ) {

   macroSubstitutions = macroSubstitutionsIn;
}

/*
    Perform a set of substitutions throughout a variable name.
    Replace $MACRO1 with VALUE1, $MACRO2 with VALUE2, etc wherever they appear in the string.
*/
QString VariableNameManager::doSubstitution( unsigned int variableIndex ) const {

   // Sanity check
   if( variableIndex >= (unsigned int )variableNames.size() )
      return "";

   // Start with the initial string
   QString result = variableNames[variableIndex];

   // Perform the required substitutions on the variable name
   return substituteThis( result );
}

/*
    Perform a set of substitutions throughout a string.
    Replace $MACRO1 with VALUE1, $MACRO2 with VALUE2, etc wherever they appear in the string.
*/
QString VariableNameManager::substituteThis( const QString string ) const {

   // Generate a list where each item in the list is a single substitution in the form MACRO1=VALUE1
   QString subs;
   subs.append( macroSubstitutionsOverride ).append( "," ).append( macroSubstitutions );

   //!!! for efficiency, should this be done when substitutions are added or removed?? Build a list of keys and values...
   // Parse the substitutions
   macroSubstitutionList parts = macroSubstitutionList( subs );

   //return the string with substitutions applied
   return parts.substitute( string );
}
