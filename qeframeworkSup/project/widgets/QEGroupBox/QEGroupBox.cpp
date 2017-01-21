/*  QEGroupBox.cpp
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
 *  Copyright (c) 2012 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QEGroupBox.h>

//------------------------------------------------------------------------------
//
QEGroupBox::QEGroupBox (QWidget *parent) : QGroupBox (parent), QEWidget (this)
{
   this->setTitle (" QEGroupBox ");

   // This is not an EPICS aware widget.
   //
   this->setVariableAsToolTip (false);
   this->setAllowDrop (false);
   this->setNumVariables (0);
}

//------------------------------------------------------------------------------
//
QEGroupBox::QEGroupBox (const QString& title, QWidget* parent) : QGroupBox (title, parent), QEWidget (this)
{
   // This is not an EPICS aware widget.
   //
   this->setVariableAsToolTip (false);
   this->setAllowDrop (false);
   this->setNumVariables (0);
   }

//------------------------------------------------------------------------------
//
QEGroupBox::~QEGroupBox () {
}

//------------------------------------------------------------------------------
//
QSize QEGroupBox::sizeHint () const {
    return QSize (120, 80);
}

// end

//==============================================================================
// Property convenience functions


// label text (prior to substitution)
void QEGroupBox::setSubstitutedTitleProperty( QString substitutedTitleIn )
{
    bool wasBlank = substitutedTitle.isEmpty();
    substitutedTitle = substitutedTitleIn;

    // Update the group box'x title.
    // But don't do it if the title was already displaying something and the
    // text-to-be-substituted is just being re-set to blank). This behaviour will
    // mean the normal label 'title' property can be used if text substitution is
    // not required. Without this the user would always have to use the substitutedTitle property.
    if (!( !title().isEmpty() && wasBlank && substitutedTitleIn.isEmpty() ))
    {
        setTitle( substituteThis( substitutedTitleIn ));
    }
}

QString QEGroupBox::getSubstitutedTitleProperty()
{
    return substitutedTitle;
}

// title text substitutions
void QEGroupBox::setSubstitutionsProperty( QString macroSubstitutionsIn )
{
    // Set the substitutions
    setVariableNameSubstitutions( macroSubstitutionsIn );

    // Update the group box's title to use the new substitutions.
    // But don't do it if the title was already displaying something and the
    // text-to-be-substituted is just being re-set to blank). This behaviour will
    // mean the normal label 'title' property can be used if text substitution is
    // not required. Without this the user would always have to use the substitutedTitle property.
    if (!( !title().isEmpty() && substitutedTitle.isEmpty() ))
    {
        setTitle( substituteThis( substitutedTitle ));
    }
}

QString QEGroupBox::getSubstitutionsProperty()
{
    return getVariableNameSubstitutions();
}
