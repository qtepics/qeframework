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
 *  Copyright (c) 2014 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
 This class manages the markup display menu for the QEImage widget.
 Functions are available to set the initial state of checkable actions.

 Note, if a markup is not used for a GUI,then the controls relating to the markup are not enabled.
 If a markup is used for a GUI, then the controls relating to the markup are enabled and the user
 can use those controls to display or hide the markup.
 */

#include "markupDisplayMenu.h"
#include "QEImage.h"

markupDisplayMenu::markupDisplayMenu( QWidget *parent) : QMenu(parent)
{
    // Macro to create the buttons
#define NEW_MARKUP_DISPLAY_MENU_BUTTON( TITLE, ID, ACTION ) \
    ACTION = new QAction( TITLE, this );                    \
    ACTION->setData( imageContextMenu::ID );                \
    ACTION->setCheckable( true );                           \
    ACTION->setChecked( true );                             \
    addAction( ACTION );

    // Create the buttons (with default titles - these will be overridden by changes to the markup legend properties)
    NEW_MARKUP_DISPLAY_MENU_BUTTON( "Horizontal Line 1", ICM_DISPLAY_HSLICE1,    hSlice1Action    )
    NEW_MARKUP_DISPLAY_MENU_BUTTON( "Horizontal Line 2", ICM_DISPLAY_HSLICE2,    hSlice2Action    )
    NEW_MARKUP_DISPLAY_MENU_BUTTON( "Horizontal Line 3", ICM_DISPLAY_HSLICE3,    hSlice3Action    )
    NEW_MARKUP_DISPLAY_MENU_BUTTON( "Horizontal Line 4", ICM_DISPLAY_HSLICE4,    hSlice4Action    )
    NEW_MARKUP_DISPLAY_MENU_BUTTON( "Horizontal Line 5", ICM_DISPLAY_HSLICE5,    hSlice5Action    )
    NEW_MARKUP_DISPLAY_MENU_BUTTON( "Vertical Line 1",   ICM_DISPLAY_VSLICE1,    vSlice1Action    )
    NEW_MARKUP_DISPLAY_MENU_BUTTON( "Vertical Line 2",   ICM_DISPLAY_VSLICE2,    vSlice2Action    )
    NEW_MARKUP_DISPLAY_MENU_BUTTON( "Vertical Line 3",   ICM_DISPLAY_VSLICE3,    vSlice3Action    )
    NEW_MARKUP_DISPLAY_MENU_BUTTON( "Vertical Line 4",   ICM_DISPLAY_VSLICE4,    vSlice4Action    )
    NEW_MARKUP_DISPLAY_MENU_BUTTON( "Vertical Line 5",   ICM_DISPLAY_VSLICE5,    vSlice5Action    )
    NEW_MARKUP_DISPLAY_MENU_BUTTON( "Area 1",          ICM_DISPLAY_AREA1,     area1Action     )
    NEW_MARKUP_DISPLAY_MENU_BUTTON( "Area 2",          ICM_DISPLAY_AREA2,     area2Action     )
    NEW_MARKUP_DISPLAY_MENU_BUTTON( "Area 3",          ICM_DISPLAY_AREA3,     area3Action     )
    NEW_MARKUP_DISPLAY_MENU_BUTTON( "Area 4",          ICM_DISPLAY_AREA4,     area4Action     )
    NEW_MARKUP_DISPLAY_MENU_BUTTON( "Arbitrary Line",  ICM_DISPLAY_PROFILE,   profileAction   )
    NEW_MARKUP_DISPLAY_MENU_BUTTON( "Target",          ICM_DISPLAY_TARGET,    targetAction    )
    NEW_MARKUP_DISPLAY_MENU_BUTTON( "Beam",            ICM_DISPLAY_BEAM,      beamAction      )
    NEW_MARKUP_DISPLAY_MENU_BUTTON( "Timestamp",       ICM_DISPLAY_TIMESTAMP, timestampAction )
    NEW_MARKUP_DISPLAY_MENU_BUTTON( "Ellipse",         ICM_DISPLAY_ELLIPSE,   ellipseAction   )

    // Set the title
    setTitle( "Markup display" );

}

// Set the availability of the menu items.
// This function presents or hides the markup controls, not the markups themselves.
// Not all markups make sense in all GUIs. If a markup is enabled for use in a GUI the
// associated check boxes are made visible here so the user can show or hide the markup.
// If a markup is not enabled for use in a GUI the associated check boxes are hidden here
// so the user sees no sign of them.
void markupDisplayMenu::enable( imageContextMenu::imageContextMenuOptions option, bool state )
{
    QAction* action = getAction( option );
    if( action )
    {
        action->setVisible( state );
    }
}

// Set the state of the menu items.
// This function presents or hides the markups themselves, not the markups controls.
// Not all markups make sense in all GUIs. If a markup is enabled for use in a GUI the
// associated check boxes are visible allowing the user to display or hide the markup
// through this functino.
void markupDisplayMenu::setDisplayed( imageContextMenu::imageContextMenuOptions option, bool state )
{
    QAction* action = getAction( option );
    if( action )
    {
        action->setChecked( state );
    }
}

// Set the text of the menu items
void markupDisplayMenu::setItemText( imageContextMenu::imageContextMenuOptions option, QString title )
{
    QAction* action = getAction( option );
    if( action )
    {
        action->setText( title );
    }
}

// Return true if the markup is to be displayed.
bool markupDisplayMenu::isDisplayed( imageContextMenu::imageContextMenuOptions option )
{
    QAction* action = getAction( option );
    if( action )
    {
        // If the action is visible, return its state
        if( action->isVisible() )
        {
            return action->isChecked();
        }

        // If the action is not visiible, ignore its state and return false as the markup is not even enabled (it is not intended to be used)
        else
        {
            return false;
        }
    }

    // Sanity check. Action should always be available
    else
    {
        return false;
    }
}


QAction* markupDisplayMenu::getAction( imageContextMenu::imageContextMenuOptions option )
{
    switch( option )
    {
        case imageContextMenu::ICM_DISPLAY_HSLICE1:   return hSlice1Action;
        case imageContextMenu::ICM_DISPLAY_HSLICE2:   return hSlice2Action;
        case imageContextMenu::ICM_DISPLAY_HSLICE3:   return hSlice3Action;
        case imageContextMenu::ICM_DISPLAY_HSLICE4:   return hSlice4Action;
        case imageContextMenu::ICM_DISPLAY_HSLICE5:   return hSlice5Action;
        case imageContextMenu::ICM_DISPLAY_VSLICE1:   return vSlice1Action;
        case imageContextMenu::ICM_DISPLAY_VSLICE2:   return vSlice2Action;
        case imageContextMenu::ICM_DISPLAY_VSLICE3:   return vSlice3Action;
        case imageContextMenu::ICM_DISPLAY_VSLICE4:   return vSlice4Action;
        case imageContextMenu::ICM_DISPLAY_VSLICE5:   return vSlice5Action;
        case imageContextMenu::ICM_DISPLAY_AREA1:     return area1Action;
        case imageContextMenu::ICM_DISPLAY_AREA2:     return area2Action;
        case imageContextMenu::ICM_DISPLAY_AREA3:     return area3Action;
        case imageContextMenu::ICM_DISPLAY_AREA4:     return area4Action;
        case imageContextMenu::ICM_DISPLAY_PROFILE:   return profileAction;
        case imageContextMenu::ICM_DISPLAY_TARGET:    return targetAction;
        case imageContextMenu::ICM_DISPLAY_BEAM:      return beamAction;
        case imageContextMenu::ICM_DISPLAY_TIMESTAMP: return timestampAction;
        case imageContextMenu::ICM_DISPLAY_ELLIPSE:   return ellipseAction;
        default:                                      return NULL;
    }
}

//// Act on a selection from the markup display menu
//void imageMarkup::markupDisplayMenuTriggered( QAction* selectedItem )
//{
//    switch( (imageContextMenu::imageContextMenuOptions)(selectedItem->data().toInt()) )
//    {
//        default:
//        case imageContextMenu::ICM_NONE: break;

//        case imageContextMenu::ICM_DISPLAY_VSLICE:     displayMarkup( imageMarkup::MARKUP_ID_V_SLICE,   selectedItem->isChecked() ); break;
//        case imageContextMenu::ICM_DISPLAY_HSLICE:     displayMarkup( imageMarkup::MARKUP_ID_H_SLICE,   selectedItem->isChecked() ); break;
//        case imageContextMenu::ICM_DISPLAY_AREA1:      displayMarkup( imageMarkup::MARKUP_ID_REGION1,   selectedItem->isChecked() ); break;
//        case imageContextMenu::ICM_DISPLAY_AREA2:      displayMarkup( imageMarkup::MARKUP_ID_REGION2,   selectedItem->isChecked() ); break;
//        case imageContextMenu::ICM_DISPLAY_AREA3:      displayMarkup( imageMarkup::MARKUP_ID_REGION3,   selectedItem->isChecked() ); break;
//        case imageContextMenu::ICM_DISPLAY_AREA4:      displayMarkup( imageMarkup::MARKUP_ID_REGION4,   selectedItem->isChecked() ); break;
//        case imageContextMenu::ICM_DISPLAY_PROFILE:    displayMarkup( imageMarkup::MARKUP_ID_LINE,      selectedItem->isChecked() ); break;
//        case imageContextMenu::ICM_DISPLAY_TARGET:     displayMarkup( imageMarkup::MARKUP_ID_TARGET,    selectedItem->isChecked() ); break;
//        case imageContextMenu::ICM_DISPLAY_BEAM:       displayMarkup( imageMarkup::MARKUP_ID_BEAM,      selectedItem->isChecked() ); break;
//        case imageContextMenu::ICM_DISPLAY_TIMESTAMP:  displayMarkup( imageMarkup::MARKUP_ID_TIMESTAMP, selectedItem->isChecked() ); break;
//        case imageContextMenu::ICM_DISPLAY_ELLIPSE:    displayMarkup( imageMarkup::MARKUP_ID_ELLIPSE,   selectedItem->isChecked() ); break;
//    }
//}
