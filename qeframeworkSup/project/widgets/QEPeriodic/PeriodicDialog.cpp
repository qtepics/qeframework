/*  PeriodicDialog.cpp
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
 *  Copyright (c) 2011 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
  This class manages the presenation of a periodic table and the selection of an
  element in the table by a user.
  It is used by the QEPeriodic widget.
  The QEPeriodic widget appears as a push button displaying an element name.
  When the QEPeriodic widget is pressed this dialog is presented with the focus on the
  button in the periodic table for the current element.
  When a button for an element in the periodic table is pressed the element symbol is
  saved ready for recall by the QEPeriodic widget.
 */

#include "PeriodicDialog.h"
#include "ui_PeriodicDialog.h"
#include <QtDebug>


// Create the periodic selection dialog
PeriodicDialog::PeriodicDialog(QWidget *parent) :
    QEDialog(parent),
    m_ui(new Ui::PeriodicDialog)
{
    m_ui->setupUi(this);
}

// Delete the periodic selection dialog
PeriodicDialog::~PeriodicDialog()
{
    delete m_ui;
}

// ???
void PeriodicDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

// Return the symbolic name of the element last selected
// Used to determine what element was selected by the user.
QString PeriodicDialog::getElement()
{
    return elementSelected;
}

// Set the focus to the push button for a an element with the specified symbol
// This is used when presenting the dialog so the dialog has focus on the element currently selected.
void PeriodicDialog::setElement( QString elementIn, QList<bool>& enabledList, QList<QString>& elementList )
{
    QList<QPushButton *> allPButtons = this->findChildren<QPushButton *>();
    for( int i = 0; i < allPButtons.size(); i++ )
    {
        // Enable or disable the button as directed by the 'enabled' list
        QString element = allPButtons[i]->text();
        for( int j = 0; j < elementList.size(); j++ )
        {
            if( element.compare( elementList[j]) == 0 )
            {
                allPButtons[i]->setEnabled( enabledList[j] );
                break;
            }
        }

        // Set the button focus if it is enabled and matching the provided symbol
        if( allPButtons[i]->isEnabled() && allPButtons[i]->text() == elementIn )
            allPButtons[i]->setFocus();
    }
}

// Save the element symbol for the widget with focus (if it is a push button)
// This is used by all the 'clicked' slots for all the element buttons
void PeriodicDialog::noteElementSelected()
{
    if( this->focusWidget()->inherits("QPushButton") )
    {
        QPushButton* btn;
        btn = (QPushButton*)(this->focusWidget());
        elementSelected = btn->text();
    }
    this->close();
}
