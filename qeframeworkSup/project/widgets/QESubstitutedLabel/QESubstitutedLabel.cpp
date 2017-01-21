/*
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

/*
  This class is a label that uses the same macro substitution mechanisms as other CA aware label widgets.
  It is intended to enable unique titles and text in sub forms.
 */

#include <QESubstitutedLabel.h>

/*
    Constructor with no initialisation
*/
QESubstitutedLabel::QESubstitutedLabel( QWidget *parent ) : QLabel( parent ), QEWidget( this ) {
    setup();
}

/*
    Setup common to all constructors
*/
void QESubstitutedLabel::setup() {

    setVariableAsToolTip( false ); // Not used as this widget does not connect to any data source

    // Set up the number of variables managed by the variable name manager
    // NOTE: there is no data associated with this widget, but it uses the same substitution mechanism as other data widgets.
    variableNameManagerInitialise( 1 );


    // Set the initial state
    setText( "----" );

    // Use label signals
    // --Currently none--
}

//==============================================================================
// Property convenience functions


// label text (prior to substitution)
void QESubstitutedLabel::setLabelTextProperty( QString labelTextIn )
{
    bool wasBlank = labelText.isEmpty();
    labelText = labelTextIn;

    // Update the label's text.
    // But don't do it if the label was already displaying something and the
    // text-to-be-substituted is just being re-set to blank). This behaviour will
    // mean the normal label 'text' property can be used if text substitution is
    // not required. Without this the user would always have to use the labelText property.
    if (!( !text().isEmpty() && wasBlank && labelText.isEmpty() ))
    {
        setText( substituteThis( labelText ));
    }
}

QString QESubstitutedLabel::getLabelTextProperty()
{
    return labelText;
}

// Get the label text with line feeds replaced with C style line feed characters.
// This is to allow line feed to be entered in the property in designer, like the QLabel text property
QString QESubstitutedLabel::getLabelTextPropertyFormat()
{
    return getLabelTextProperty().replace( "\n", "\\n" );

}

// Set the label text with C style line feed characters replaced with line feeds.
// This is to allow line feed to be entered in the property in designer, like the QLabel text property
void QESubstitutedLabel::setLabelTextPropertyFormat( QString labelTextIn )
{
    setLabelTextProperty( labelTextIn.replace( "\\n", "\n" ));
}

// label text substitutions
void QESubstitutedLabel::setSubstitutionsProperty( QString macroSubstitutionsIn )
{
    // Set the substitutions
    setVariableNameSubstitutions( macroSubstitutionsIn );

    // Update the label's text to use the new substitutions.
    // But don't do it if the label was already displaying something and the
    // text-to-be-substituted is just being re-set to blank). This behaviour will
    // mean the normal label 'text' property can be used if text substitution is
    // not required. Without this the user would always have to use the labelText property.
    if (!( !text().isEmpty() && labelText.isEmpty() ))
    {
        setText( substituteThis( labelText ));
    }
}

QString QESubstitutedLabel::getSubstitutionsProperty()
{
    return getVariableNameSubstitutions();
}
