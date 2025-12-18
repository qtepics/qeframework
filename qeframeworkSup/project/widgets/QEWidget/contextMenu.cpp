/*  contextMenu.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2011-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

/* Description:
 *
 * A class to manage the QEGui context menu.
 *
 * This class creates a menu that can be added as the right click menu for QE widgets.
 * The contextMenu class cannot be based on QObject so it creates and owns an instance
 * of a small class (contextMenyObject) that is based on QObject that can manage
 * signals and slots.
 * The contextMenu class can't be based on QObject as only one base class of an object
 * can be based on QObjects and for all QE widgets that class is typically a standard Qt widget.
 *
 */

#include "contextMenu.h"
#include <QApplication>
#include <QDebug>
#include <QClipboard>
#include <QMetaType>
#include <QEPlatform.h>
#include <QEWidget.h>
#include <QEScaling.h>
#include <QAction>

#define DEBUG qDebug() << "contextMenu" << __LINE__ << __FUNCTION__ << "  "

// Flag common to all context menus.
// true if 'dragging the variable
// false if dragging the data
bool contextMenu::draggingVariable = true;

//==============================================================================
// Methods for QObject based contextMenuObject class
//==============================================================================
//
QEContextMenuObject::QEContextMenuObject( contextMenu* menuIn,  QObject* parent ) :
   QObject (parent)
{
   menu = menuIn;
}

//------------------------------------------------------------------------------
// place holder
QEContextMenuObject::~QEContextMenuObject() { }

//------------------------------------------------------------------------------
//
void QEContextMenuObject::sendRequestAction( const QEActionRequests& request)
{
   emit requestAction( request );
}

//------------------------------------------------------------------------------
//
void QEContextMenuObject::contextMenuTriggeredSlot( QAction* selectedItem )
{
   menu->contextMenuTriggered( selectedItem->data().toInt() );
}

//------------------------------------------------------------------------------
//
void QEContextMenuObject::showContextMenuSlot( const QPoint& pos )
{
   menu->showContextMenu( pos );
}

//==============================================================================
// Methods for contextMenu class
//==============================================================================
// Create the default menu set, i.e. the lot.
//
contextMenu::ContextMenuOptionSets  contextMenu::defaultMenuSet ()
{
   ContextMenuOptionSets result;

   result.clear();

   for( int j = CM_NOOPTION; j < CM_SPECIFIC_WIDGETS_START_HERE; j++ )
   {
      contextMenuOptions e = (contextMenuOptions) j;
      result.insert( e );
   }
   return result;
}

//------------------------------------------------------------------------------
// Create a class to manage the QE context menu
//
contextMenu::contextMenu( QEWidget* qewIn, QWidget* ownerIn )
{
   hasConsumer = false;
   qew = qewIn;
   numberOfItems = 1;
   editPvUserLevel = QE::Engineer;
   menuSet = defaultMenuSet();

   // Create the signaller object - note: owned and deleted the widget
   object = new QEContextMenuObject( this, ownerIn );
}

//------------------------------------------------------------------------------
//
contextMenu::~contextMenu() { }   // place holder

//------------------------------------------------------------------------------
// Tests is primary PV is an array variable
//
bool contextMenu::isArrayVariable () const
{
   bool result = false;
   qcaobject::QCaObject* qca = qew->getQcaItem( 0 );
   if( qca ){
      result = (qca->getHostElementCount() >= 2);
   }
   return result;
}

//------------------------------------------------------------------------------
// static
bool contextMenu::insertBefore( QMenu* menu, QAction* action, const int option )
{
   if (!menu || !action) return false;

   QList<QAction*> actionList = menu->actions ();
   for( int j = 0; j < actionList.count(); j++ ) {
      QAction* aoi = actionList.value( j, NULL );
      if( !aoi )continue;
      int ad =  aoi->data().toInt( NULL );
      if( ad == option ){
         // We have found a match
         menu->insertAction( aoi, action );
         return true;
      }
   }
   return false;
}

//------------------------------------------------------------------------------
// static
bool contextMenu::insertSeparatorBefore( QMenu* menu, const int option )
{
   if (!menu) return false;

   QList<QAction*> actionList = menu->actions ();
   for( int j = 0; j < actionList.count(); j++ ) {
      QAction* aoi = actionList.value( j, NULL );
      if( !aoi )continue;
      int ad =  aoi->data().toInt( NULL );
      if( ad == option ){
         // We have found a match
         menu->insertSeparator(aoi);
         return true;
      }
   }
   return false;
}

//------------------------------------------------------------------------------
// static
bool contextMenu::insertAfter(  QMenu* menu, QAction* action, const int option )
{
   if (!menu || !action) return false;

   QList<QAction*> actionList = menu->actions ();
   for( int j = 0; j < actionList.count(); j++ ) {
      QAction* aoi = actionList.value( j, NULL );
      if( !aoi )continue;
      int ad =  aoi->data().toInt( NULL );
      if( ad == option ){
         // We have found a match
         // The insertAction function is insert before, so we have to be sneaky.
         menu->insertAction( aoi, action );
         menu->removeAction( aoi );
         menu->insertAction( action, aoi );
         return true;
      }
   }
   return false;
}

//------------------------------------------------------------------------------
// Build the QE generic context menu
//
QMenu* contextMenu::buildContextMenu()
{
   bool addSeparator;

   // Create the menu
   QMenu* menu = new QMenu( );//qew->getQWidget() );
   menu->setStyle( QApplication::style() );

   // Get Qt widget standard menu if any
   QMenu* defaultMenu = qew->getDefaultContextMenu();

   if( defaultMenu )
   {
      defaultMenu->setStyle( QApplication::style() );

      // Apply current scaling if any to new default menu.
      //
      QEScaling::applyToWidget( defaultMenu );
      menu->addMenu( defaultMenu );
      menu->addSeparator();
   }

   // Add QE context menu
   QAction* a;
   QString names = (numberOfItems >= 2) ? "names " : "name ";

   // Add menu options that require the application to provide support such as launch a strip chart.
   if( hasConsumer )
   {
      addSeparator = false;

      if( menuSet.contains( CM_SHOW_PV_PROPERTIES ))
      {
         a = new QAction( "Examine Properties",     menu ); a->setCheckable( false ); a->setData( CM_SHOW_PV_PROPERTIES ); menu->addAction( a );
         addSeparator = true;
      }

      if( menuSet.contains( CM_ADD_TO_STRIPCHART ))
      {
         a = new QAction( "Plot in StripChart",     menu ); a->setCheckable( false ); a->setData( CM_ADD_TO_STRIPCHART );  menu->addAction( a );
         addSeparator = true;
      }

      if( menuSet.contains( CM_ADD_TO_SCRATCH_PAD ))
      {
         a = new QAction( "Show in Scratch Pad",    menu ); a->setCheckable( false ); a->setData( CM_ADD_TO_SCRATCH_PAD ); menu->addAction( a );
         addSeparator = true;
      }

      // This menu items are really only sensible of array PVs, i.e. PVs with at least two elements.
      if( isArrayVariable() ) {

         if( menuSet.contains( CM_ADD_TO_PLOTTER ))
         {
            a = new QAction( "Show in Plotter",    menu ); a->setCheckable( false ); a->setData( CM_ADD_TO_PLOTTER ); menu->addAction( a );
            addSeparator = true;
         }

         if( menuSet.contains( CM_SHOW_AS_HISTOGRAM ))
         {
            a = new QAction( "Show as Historgram", menu ); a->setCheckable( false ); a->setData( CM_SHOW_AS_HISTOGRAM ); menu->addAction( a );
            addSeparator = true;
         }

         if( menuSet.contains( CM_ADD_TO_TABLE ))
         {
            a = new QAction( "Show in Table",      menu ); a->setCheckable( false ); a->setData( CM_ADD_TO_TABLE ); menu->addAction( a );
            addSeparator = true;
         }
      }

      if( addSeparator ) menu->addSeparator();
   }

   // Add menu options that don't require the application to provide support such as launch a strip chart.
   addSeparator = false;

   if( menuSet.contains( CM_COPY_VARIABLE ))
   {
      a = new QAction( "Copy variable " + names, menu ); a->setCheckable( false ); a->setData( CM_COPY_VARIABLE );      menu->addAction( a );
      addSeparator = true;
   }

   if( menuSet.contains( CM_COPY_DATA ))
   {
      a = new QAction( "Copy data",              menu ); a->setCheckable( false ); a->setData( CM_COPY_DATA );          menu->addAction( a );
      addSeparator = true;
   }

   if( menuSet.contains( CM_PASTE ))
   {
      a = new QAction( "Paste to variable " + names, menu ); a->setCheckable( false ); a->setData( CM_PASTE );          menu->addAction( a );

      QClipboard *cb = QApplication::clipboard();
      a->setEnabled( qew->getAllowDrop() && !cb->text().isEmpty() );
      addSeparator = true;
   }

   if( addSeparator )  menu->addSeparator();

   addSeparator = false;

   if( menuSet.contains( CM_DRAG_VARIABLE ))
   {
      a = new QAction( "Drag variable " + names, menu ); a->setCheckable( true );  a->setData( CM_DRAG_VARIABLE );      menu->addAction( a );
      a->setChecked( draggingVariable );
      addSeparator = true;
   }

   if( menuSet.contains( CM_DRAG_DATA ))
   {
      a = new QAction( "Drag data",              menu ); a->setCheckable( true );  a->setData( CM_DRAG_DATA );          menu->addAction( a );
      a->setChecked( !draggingVariable );
      addSeparator = true;
   }

   // Add edit PV menu if and only if we are in the approriate level.
   if( ( qew->getUserLevel() >= editPvUserLevel ) && menuSet.contains( CM_GENERAL_PV_EDIT ))
   {
      if( addSeparator ) menu->addSeparator();
      a = new QAction( "Edit PV", menu );
      a->setCheckable( false );
      a->setData( CM_GENERAL_PV_EDIT );
      menu->addAction( a );
   }

   menu->setTitle( "Use..." );

   QObject::connect( menu, SIGNAL( triggered ( QAction* ) ),
                     object, SLOT( contextMenuTriggeredSlot( QAction* )) );

   // This object is created dynamically as opposed to at overall contruction time,
   // so need to apply current scalling, if any to the new menu.
   QEScaling::applyToWidget( menu );

   return menu;
}

//------------------------------------------------------------------------------
// Create and present a context menu given a global co-ordinate
//
QAction* contextMenu::showContextMenuGlobal( const QPoint& globalPos )
{
   QMenu* menu = buildContextMenu();
   QAction* action = showContextMenuGlobal( menu, globalPos );
   delete menu;
   return action;
}

//------------------------------------------------------------------------------
// Create and present a context menu given a co-ordinate relative to the QE widget
//
QAction* contextMenu::showContextMenu( const QPoint& pos )
{
   QMenu* menu = buildContextMenu();
   QAction* action = showContextMenu( menu, pos );
   delete menu;
   return action;
}

//------------------------------------------------------------------------------
// Present an existing context menu given a global co-ordinate
//
QAction* contextMenu::showContextMenuGlobal( QMenu* menu, const QPoint& globalPos )
{
   return menu->exec( globalPos );
}

//------------------------------------------------------------------------------
// Present an existing context menu given a co-ordinate relative to the QE widget
//
QAction* contextMenu::showContextMenu( QMenu* menu, const QPoint& pos )
{
   QPoint globalPos = qew->getQWidget()->mapToGlobal( pos );
   return menu->exec( globalPos );
}

//------------------------------------------------------------------------------
// Return the global 'is dragging variable' flag.
// (Dragging variable is true, draging data if false)
//
bool contextMenu::isDraggingVariable()
{
   return draggingVariable;
}

//------------------------------------------------------------------------------
// Set the consumer of the signal generated by this object
// (send via the associated contextMenuObject object).
// Only one consumer allowed - last set is the consumer.
//
void contextMenu::setConsumer (QObject* consumer)
{
   if (consumer)
   {
      QObject::disconnect (object, SIGNAL (requestAction( const QEActionRequests& )), 0,0);
      hasConsumer =  QObject::connect (object, SIGNAL (requestAction( const QEActionRequests& )),
                                       consumer, SLOT (requestAction( const QEActionRequests& )));
   }
}

//------------------------------------------------------------------------------
// Connect the supplied QE widget to a slot that will present out own context menu when requested
//
void contextMenu::setupContextMenu( const ContextMenuOptionSets& menuSetIn )
{
   menuSet = menuSetIn;   // save required menu items.
   QWidget* qw = qew->getQWidget();
   qw->setContextMenuPolicy( Qt::CustomContextMenu );
   QObject::connect( qw, SIGNAL( customContextMenuRequested( const QPoint& )),
                     object, SLOT( showContextMenuSlot( const QPoint& )));
}

//------------------------------------------------------------------------------
// Set minimum user level required for EditPV context menu entry available.
//
void contextMenu::setEditPvUserLevel( const QE::UserLevels level )
{
   editPvUserLevel = level;
}

//------------------------------------------------------------------------------
// Get minimum user level required for EditPV context menu entry available.
//
bool contextMenu::getEditPvUserLevel() const
{
   return editPvUserLevel;
}

//------------------------------------------------------------------------------
// Disconnect the supplied QE widget to a slot that will present the context menu.
// This allows a "complex" widget that contains this widget to capture the custom
// context menu request signal.
//
void contextMenu::clearContextMenuRequestHandling()
{
   QWidget* qw = qew->getQWidget();
   QObject::disconnect( qw, SIGNAL( customContextMenuRequested( const QPoint& )),
                        object, SLOT( showContextMenuSlot( const QPoint& )));
}

//------------------------------------------------------------------------------
// Update  the conext menu items that will be presented.
//
void contextMenu::setContextMenuOptions( const ContextMenuOptionSets& menuSetIn )
{
   menuSet = menuSetIn;   // save required menu items.
}

//------------------------------------------------------------------------------
// Update the number of items that will be copied/dragged etc.
//
void contextMenu::setNumberOfContextMenuItems ( const int numberOfItemsIn )
{
   numberOfItems = numberOfItemsIn;
}

//------------------------------------------------------------------------------
// An action was selected from the context menu
//
void contextMenu::contextMenuTriggered( int optionNum )
{
   switch( (contextMenuOptions)(optionNum) )
   {
      default:
      case contextMenu::CM_NOOPTION:
         break;

      case contextMenu::CM_COPY_VARIABLE:
         doCopyVariable();
         break;

      case contextMenu::CM_COPY_DATA:
         doCopyData();
         break;

      case contextMenu::CM_PASTE:
         doPaste();
         break;

      case contextMenu::CM_DRAG_VARIABLE:
         draggingVariable = true;
         break;

      case contextMenu::CM_DRAG_DATA:
         draggingVariable = false;
         break;

      case contextMenu::CM_SHOW_PV_PROPERTIES:
         doShowPvProperties();
         break;

      case contextMenu::CM_ADD_TO_STRIPCHART:
         doAddToStripChart();
         break;

      case contextMenu::CM_ADD_TO_SCRATCH_PAD:
         doAddToScratchPad();
         break;

      case contextMenu::CM_ADD_TO_PLOTTER:
         doAddToPlotter();
         break;

      case contextMenu::CM_ADD_TO_TABLE:
         doAddToTable();
         break;

      case contextMenu::CM_SHOW_AS_HISTOGRAM:
         doShowAsHistogram();
         break;

      case contextMenu::CM_GENERAL_PV_EDIT:
         doGeneralPVEdit();
         break;
   }
}

//------------------------------------------------------------------------------
// 'Copy Variable' was selected from the menu
//
void contextMenu::doCopyVariable()
{
   QString s = copyVariable();
   QClipboard *cb = QApplication::clipboard();
   cb->setText( s );
}

//------------------------------------------------------------------------------
// 'Copy Data' was selected from the menu
//
void contextMenu::doCopyData()
{
   QClipboard* cb = QApplication::clipboard();
   QVariant v = this->copyData();
   const QMetaType::Type mtype = QEPlatform::metaType (v);
   switch( mtype )
   {
      default:
      case QMetaType::QString:
         cb->setText( v.toString() );
         break;

      case QMetaType::QImage:
         cb->setImage( v.value<QImage>() );
         break;
   }
}

//------------------------------------------------------------------------------
// 'Paste' was selected from the menu
//
void contextMenu::doPaste()
{
   QVariant v;
   QClipboard *cb = QApplication::clipboard();
   if( !cb->text().isEmpty() )
   {
      v = QVariant( cb->text() );
   }
   else if( !cb->image().isNull() )
   {
      v = QVariant( cb->image() );
   }
   paste( v );
}

//------------------------------------------------------------------------------
// 'Show Properties' was selected from the menu
//
void contextMenu::doShowPvProperties ()
{
   QString pvName = copyVariable().trimmed();
   QEActionRequests request( QEActionRequests::actionPvProperties(), pvName );
   if( !pvName.isEmpty() ) object->sendRequestAction( request );
}

//------------------------------------------------------------------------------
// 'Add to strip chart' wasselected from the menu
//
void contextMenu::doAddToStripChart ()
{
   QString pvName = copyVariable().trimmed();
   QEActionRequests request( QEActionRequests::actionStripChart(), pvName );
   if( !pvName.isEmpty() ) object->sendRequestAction( request );
}

//------------------------------------------------------------------------------
// 'Add to scratch pad' was selected from the menu
//
void contextMenu::doAddToScratchPad()
{
   QString pvName = copyVariable().trimmed();
   QEActionRequests request( QEActionRequests::actionScratchPad(), pvName );
   if( !pvName.isEmpty() ) object->sendRequestAction( request );
}

//------------------------------------------------------------------------------
// 'Show in Plotter' was selected from the menu
//
void contextMenu::doAddToPlotter()
{
   QString pvName = copyVariable().trimmed();
   QEActionRequests request( QEActionRequests::actionPlotter(), pvName );
   if( !pvName.isEmpty() ) object->sendRequestAction( request );
}

//------------------------------------------------------------------------------
// 'Show in Table' was selected from the menu
//
void contextMenu::doAddToTable()
{
   QString pvName = copyVariable().trimmed();
   QEActionRequests request( QEActionRequests::actionTable(), pvName );
   if( !pvName.isEmpty() ) object->sendRequestAction( request );
}

//------------------------------------------------------------------------------
// 'Show as Histogram' was selected from the menu
//
void contextMenu::doShowAsHistogram()
{
   QString pvName = copyVariable().trimmed();
   QEActionRequests request( QEActionRequests::actionShowInHisogram(), pvName );
   if( !pvName.isEmpty() ) object->sendRequestAction( request );
}

//------------------------------------------------------------------------------
// Request mini general PV edit form.
//
void contextMenu::doGeneralPVEdit()
{
   QString pvName = copyVariable().trimmed();
   QEActionRequests request( QEActionRequests::actionGeneralPvEdit(), pvName );
   if( !pvName.isEmpty() ) object->sendRequestAction( request );
}

//------------------------------------------------------------------------------
// Add a menu item to the either the context menu, or one of its sub menus
//
void contextMenu::addMenuItem( QMenu* menu, const QString& title,
                               const bool checkable, const bool checked,
                               const int option )
{
   QAction* a = new QAction( title, menu );
   a->setCheckable( checkable );
   if( checkable )
   {
      a->setChecked( checked );
   }
   a->setData( option );
   menu->addAction( a );
}

// end
