/*  QEActionRequests.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2013-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_ACTION_REQUESTS_H
#define QE_ACTION_REQUESTS_H

#include <QDebug>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QEEnums.h>
#include <QEFrameworkLibraryGlobal.h>
#include <QAction>
#include <QEFormMapper.h>


// Forward class declarations
// TODO Rebadge to QEWindowCreationListItem and QEComponentHostListItem
//
class windowCreationListItem;
class componentHostListItem;

// Class defining an action an application should carry out on behalf of
// a QE Widget it has created.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEActionRequests {
public:

   // Type of request
   enum Kinds {
      KindNone,            // no action (default, not valid in any request)
      KindOpenFile,        // by file name, e.g. "detector_control.ui"
      KindOpenFiles,       // by file names, tahe a list of windowCreationListItems
      KindAction,          // inbuilt application action, e.g "PV Properties..."
      KindWidgetAction,    // inbuilt QE widget action

      // Application to host a widget on behalf of a QE widget. For example a
      // QEImage widget can create a profile plot in a QFrame and either display
      // it within itself or ask the application to host it is a dock window.
      KindHostComponents   // componentHostListItem or list thereof.
   };


   // How new windows created for the request are to be presented.
   // May not be relevent for all requests
   // enum QE::CreationOptions  replaces  enum Options

   // Predefined actions values for built in windows that consumer may provide.
   //
   static QString actionPvProperties ()   { return "PV Properties...";   }
   static QString actionStripChart ()     { return "Strip Chart...";     }
   static QString actionScratchPad ()     { return "Scratch Pad...";     }
   static QString actionPlotter ()        { return "Show in Plotter..."; }
   static QString actionTable ()          { return "Show in Table...";   }
   static QString actionShowInHisogram () { return "Show in Histogram..."; }
   static QString actionGeneralPvEdit ()  { return "General PV Edit..."; }

   // Constructors.
   //
   QEActionRequests ();

   // Action (for the application)
   QEActionRequests (const QString& action,
                     const QString& pvName);


   // .ui file name plus create option plus optional form handle.
   QEActionRequests (const QString &filename,
                     const QString &config,
                     const QE::CreationOptions optionIn,
                     const QEFormMapper::FormHandles& formHandle = QEFormMapper::nullHandle ());

   // a set of windows to create
   QEActionRequests (const QList<windowCreationListItem> windowsIn);

   // Action (for a QE widget)
   QEActionRequests (const QString& actionIn,
                     const QString& widgetNameIn,
                     const QStringList& argumentsIn,
                     const bool& initialiseIn,
                     QAction* originator);

   // single component to host for a QE widget
   QEActionRequests( const componentHostListItem& componentIn );

   // a set of components to host for a QE widget
   QEActionRequests( const QList<componentHostListItem>& componentsIn );

   // set and get functions
   //
   void setKind (const Kinds kindIn);
   Kinds getKind () const;

   void setArguments (const QStringList & argumentsIn);
   QStringList getArguments () const;

   void setOption (const QE::CreationOptions optionIn);
   QE::CreationOptions getOption () const;

   void setAction (const QString actionIn);
   QString getAction() const;

   void setWidgetName (const QString widgetNameIn);
   QString getWidgetName() const;

   void setInitialise (const bool initialiseIn);
   bool getInitialise() const;

   void setOriginator (QAction* originatorIn);
   QAction* getOriginator() const;

   void setFormHandle (const QEFormMapper::FormHandles& formHandle);
   QEFormMapper::FormHandles getFormHandle () const;

   QString getCustomisation() const;

   QList<windowCreationListItem> getWindows() const;
   QList<componentHostListItem> getComponents() const;

   static bool isDockCreationOption (const QE::CreationOptions createOption);          // Return true if creation option creates a dock
   static bool isTabbedDockCreationOption (const QE::CreationOptions createOption);    // Return true if creation option creates a tabbed dock

private:
   Kinds kind;
   QString action;
   QStringList arguments;
   QE::CreationOptions option;
   QString customisation;  // Window configuration (menus, buttons, etc)
   QEFormMapper::FormHandles formHandle;  // allows requestor to nominate a handle for the created QEForm.
   QList<windowCreationListItem> windows;
   QList<componentHostListItem> components;
   QString widgetName;
   bool initialise;     // If true, initial preperation to do this action, don't actually do it.
   // For example, set initial checked state of menu item
   QAction* originator; // A copy would be safer???
};

// allows qDebug() << QEActionRequests object.
//
QDebug QE_FRAMEWORK_LIBRARY_SHARED_EXPORT
operator<<(QDebug dbg, const QEActionRequests& actionRequest);


//------------------------------------------------------------------------------
// Class to hold window creation instructions
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT windowCreationListItem
{
public:
   windowCreationListItem ();
   windowCreationListItem (windowCreationListItem* item);
   ~windowCreationListItem();

   QString                   uiFile;               // .UI file to open when this item is actioned
   QString                   macroSubstitutions;   // Macro substitutions to apply when this item is actioned
   QString                   customisationName;    // Customisation name to apply to any main windows created when this item is actioned
   QEFormMapper::FormHandles formHandle;           // Allows requestor to nominate a handle for the created QEForm.
   QE::CreationOptions creationOption;             // Creation option defining how the UI file is presented (in a new window, a tabbed dock, etc)
   bool                      hidden;               // If true, any new dock is created hidden
   QString                   title;                // Title of this menu item
};

QDebug QE_FRAMEWORK_LIBRARY_SHARED_EXPORT
operator<<(QDebug dbg, const windowCreationListItem& item);

//------------------------------------------------------------------------------
// Class to hold component hosting instructions.
// (an application can host a widget on behalf of a QE widget.
// For example a QEImage widget can create a profile plot in a QFrame and either
// display it within itself or ask the application to host it is a dock window.)
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT componentHostListItem
{
public:
   componentHostListItem ();
   componentHostListItem (QWidget*                  widgetIn,
                          QE::CreationOptions       creationOptionIn,
                          bool                      hiddenIn,
                          QString                   titleIn );
   componentHostListItem (componentHostListItem* item);
   ~componentHostListItem ();

   QWidget*                  widget;
   QE::CreationOptions       creationOption;
   bool                      hidden;
   QString                   title;
};

QDebug QE_FRAMEWORK_LIBRARY_SHARED_EXPORT
operator<<(QDebug dbg, const componentHostListItem& item);


Q_DECLARE_METATYPE (QEActionRequests)

# endif // QE_ACTION_REQUESTS_H
