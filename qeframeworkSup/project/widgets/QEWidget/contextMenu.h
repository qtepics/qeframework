/*  contextMenu.h
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
 *  Copyright (c) 2011,2016,2017 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#ifndef QE_CONTEXT_MENU_H
#define QE_CONTEXT_MENU_H

#include <QMenu>
#include <QSet>
#include <QEActionRequests.h>

class contextMenu;
class QEWidget;

// QObject based context menu
// Instance of this created and owned by contextMenu which itself cannot be based on a QObject
class QEContextMenuObject : public QObject
{
    Q_OBJECT
public:
    QEContextMenuObject( contextMenu* menuIn,  QObject* parent ); // Construction
    ~QEContextMenuObject();                                       // Destruction
    void sendRequestAction( const QEActionRequests& request);   // Emit a GUI launch request

signals:
    void requestAction( const QEActionRequests& );                      // Signal 'launch a GUI'

public slots:
    void contextMenuTriggeredSlot( QAction* selectedItem );             // Slot - an item has been selected from the context menu
    void showContextMenuSlot( const QPoint& pos );                      // Slot - a widget has requested the QE sutom context menu be shown

private:
    contextMenu* menu;                                                  // contextMenu class owning this class
};

// Manage QE widget context menu
class QEPLUGINLIBRARYSHARED_EXPORT contextMenu
{
public:
    friend class QEContextMenuObject;

    enum contextMenuOptions{ CM_NOOPTION,
                             CM_COPY_VARIABLE, CM_COPY_DATA, CM_PASTE,
                             CM_DRAG_VARIABLE, CM_DRAG_DATA,
                             CM_SHOW_PV_PROPERTIES,
                             CM_ADD_TO_STRIPCHART,
                             CM_ADD_TO_SCRATCH_PAD,
                             CM_ADD_TO_PLOTTER,
                             CM_ADD_TO_TABLE,
                             CM_SHOW_AS_HISTOGRAM,
                             CM_GENERAL_PV_EDIT,
                             CM_SPECIFIC_WIDGETS_START_HERE };

    typedef QSet<contextMenuOptions> ContextMenuOptionSets;

    static ContextMenuOptionSets defaultMenuSet ();     // All menu items are in the default set.

    explicit contextMenu( QEWidget* qewIn, QWidget* ownerIn );
    virtual ~contextMenu();

    // Set up the standard QE context menu for a QE widget (conextMenu class is a base class for
    // all QE widgets, but a menu is only available to users if this is called)
    void setupContextMenu( const ContextMenuOptionSets& menuSet = contextMenu::defaultMenuSet ());

    void clearContextMenuRequestHandling();             // Clears internal signal connection to contextMenuObject

    void setContextMenuOptions( const ContextMenuOptionSets& menuSet );     // Update conext menu options
    void setNumberOfContextMenuItems( const int numberOfItems );            // Set number of PV items - used to pluralise conext menu captions

    bool isDraggingVariable();                          // Return the global 'is dragging variable' flag (Dragging variable is true, draging data if false)

    virtual QMenu* buildContextMenu();                        // Build the QE generic context menu
    virtual void contextMenuTriggered( int selectedItemNum ); // An action was selected from the context menu

    virtual QString copyVariable(){ return ""; }        // Function a widget may implement to perform a 'copy variable' operation
    virtual QVariant copyData(){ return ""; }           // Function a widget may implement to perform a 'copy data' operation
    virtual void paste( QVariant ){}                    // Function a widget may implement to perform a 'paste' operation
    QAction* showContextMenuGlobal( const QPoint& globalPos );              // Create and present a context menu given a global co-ordinate
    QAction* showContextMenu( const QPoint& pos );                          // Create and present a context menu given a co-ordinate relative to the QE widget
    QAction* showContextMenuGlobal( QMenu* menu, const QPoint& globalPos ); // Present an existing context menu given a global co-ordinate
    QAction* showContextMenu( QMenu* menu, const QPoint& pos );             // Present an existing context menu given a co-ordinate relative to the QE widget

    void addMenuItem( QMenu* menu, const QString& title, const bool checkable, const bool checked, const int option );

protected:
    void setConsumer (QObject *consumer);               // Set the consumer of the signal generted by this object

private:
    QEContextMenuObject* object;                          // Our own QObject based class to managing signals and slots
    void doCopyVariable();                              // 'Copy Variable' was selected from the menu
    void doCopyData();                                  // 'Copy Data' was selected from the menu
    void doPaste();                                     // 'Paste' was selected from the menu
    void doShowPvProperties();                          // 'Show Properties' was selected from the menu
    void doAddToStripChart();                           // 'Add to strip chart' was selected from the menu
    void doAddToScratchPad();                           // 'Add to scratch pad' was selected from the menu
    void doAddToPlotter();                              // 'Add to plotter' was selected from the menu
    void doAddToTable();                                // 'Add to table' was selected from the menu
    void doShowAsHistogram();                           // 'Show as histogram' was selected from the menu
    void doGeneralPVEdit();                             // 'Add to scratch pad' was selected from the menu
    bool isArrayVariable () const;                      // Tests is primary PV is an array variable
    static bool draggingVariable;                       // Global 'dragging variable' flag (dragging data if false)
    QEWidget* qew;                                      // QEWidget associated with this instance
    bool hasConsumer;                                   // A launch consumer has been set (it is ok to present menu options that require application support to receive signals to, for example, start a strip chart
    ContextMenuOptionSets menuSet;                      // Defines required set of menu items.
    int numberOfItems;                                  // Number PV names to be copied/dragged
};

#endif // QE_CONTEXT_MENU_H
