/*  QESubstitutedLabel.cpp
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
  This class is a label that uses the same macro substitution mechanisms as other CA aware label widgets.
  It is intended to enable unique titles and text in sub forms.
 */

#include <QESubstitutedLabel.h>
#include <QECommon.h>

/*
    Constructor with no initialisation
*/
QESubstitutedLabel::QESubstitutedLabel( QWidget *parent ) :
   QLabel( parent ),
   QEWidget( this )
{
    setup();
}

/*
    Setup common to all constructors
*/
void QESubstitutedLabel::setup()
{
    // Not used as this widget does not connect to any data source
    setVariableAsToolTip( false );

    // Set up the number of variables managed by the variable name manager
    // NOTE: there is no data associated with this widget, but it uses the same
    // substitution mechanism as other data widgets.
    variableNameManagerInitialise( 1 );

    // Set the initial state
    setText( "----" );
    setIndent( 6 );

    setStyleSheet( QEUtilities::offBackgroundStyle() );

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

// end
