/*  QEAbstractDynamicWidget.h
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
 *  Copyright (c) 2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_ABSTRACT_COMPLEX_WIDGET_H
#define QE_ABSTRACT_COMPLEX_WIDGET_H

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QEPluginLibrary_global.h>
#include <QEFrame.h>

/// Provides a common abstract base class for dynamic widgets, i.e. dynamic in the
/// sense that the user can add/removed and modifiy PVs used by the widget at run time.
/// It has been specifically designed to be a common base class for the QEStripChart,
/// QEScratchPad, QEPlotter and QETable widgets. This not only minimises maintainance,
/// but helps ensure we maintain a common look and feel user experiance.
///
class QEPLUGINLIBRARYSHARED_EXPORT QEAbstractDynamicWidget : public QEFrame
{
   Q_OBJECT

   typedef QEFrame ParentWidgetClass;

   /// Default directory used for loading/saving files. Default to null string
   /// which is interpreted as the current directory.
   ///
   Q_PROPERTY (QString defaultDir  READ getDefaultDir  WRITE setDefaultDir)

public:
   // Abstract Dynamic Widget Context Menu values
   //
   enum OwnContextMenuOptions { ADWCM_NONE = CM_SPECIFIC_WIDGETS_START_HERE,
                                ADWCM_LOAD_WIDGET_CONFIG,
                                ADWCM_SAVE_WIDGET_CONFIG,
                                ADWCM_SUB_CLASS_WIDGETS_START_HERE };

   explicit QEAbstractDynamicWidget (QWidget* parent = 0);
   ~QEAbstractDynamicWidget ();

   // Set/Get default configuration load/save directory.
   //
   void setDefaultDir (const QString& defaultDir);
   QString getDefaultDir () const;

   // Both add names to next available slot(s) if any.
   //.
   void paste (QVariant s);

   // Used by paste, but also made publically available.
   //
   void addPvNameList (const QStringList& pvNameList);
   void addPvNameSet (const QString& pvNameSet);

   // Add PV to next avialble slot (if any).
   // returns slot number 0 .. Max - 1 iff successful otherwise -1.
   //
   // Sub-class must provide an implementation for this function.
   //
   virtual int addPvName (const QString& pvName) = 0; // Add name to next available slot.

protected:
   QMenu* buildContextMenu ();                        // Build the specific context menu
   void contextMenuTriggered (int selectedItemNum);   // An action was selected from the context menu

   QString getPersistantRootName () const;
   QString getPersistantName () const;

protected slots:
   // Leverage off the persistant manager capability to load/save widget configurations.
   // This functions allow the configuration of a single widget to be saved and reloaded.
   //
   void loadNamedWidetConfiguration (const QString& filename);
   void saveNamedWidetConfiguration (const QString& filename);

   // Both these invoke a file dialog to select the filename.
   //
   void loadWidgetConfiguration ();
   void saveWidgetConfiguration ();

private:
   bool useOwnPersistantName;
   QString defaultDir;
};

#endif // QE_ABSTRACT_COMPLEX_WIDGET_H
