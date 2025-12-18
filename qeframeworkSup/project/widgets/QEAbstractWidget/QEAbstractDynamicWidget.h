/*  QEAbstractDynamicWidget.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2016-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_ABSTRACT_DYNAMIC_WIDGET_H
#define QE_ABSTRACT_DYNAMIC_WIDGET_H

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QEEnums.h>
#include <QEFrameworkLibraryGlobal.h>
#include <ContainerProfile.h>
#include <QEAbstractWidget.h>

class QELabel;    // differed - need avoid some weird meta data compiler error.

/// Provides a common abstract base class for dynamic widgets, i.e. dynamic in the
/// sense that the user can add/removed and modifiy PVs used by the widget at run time.
/// It has been specifically designed to be a common base class for the QEStripChart,
/// QEScratchPad, QEPlotter and QETable widgets. This not only minimises maintainance,
/// but helps ensure we maintain a common look and feel user experiance.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEAbstractDynamicWidget :
      public QEAbstractWidget
{
   Q_OBJECT

   typedef QEAbstractWidget ParentWidgetClass;

public:
   /// Default directory used for loading/saving files. Default to null string
   /// which is interpreted as the current directory.
   ///
   Q_PROPERTY (QString defaultDir    READ getDefaultDir    WRITE setDefaultDir)

   /// By default, the contexMenu class only adds the "Edit PV" menu entry if and only
   /// if we are using the engineer user level (provided it  has also been included in
   /// the widget menu set). When enableEditPv set true, the user level required is set
   /// to user level user, i.e. always available.
   /// Note: this may apply to this widget itself, but is more likely to be applied to
   /// the contained dynamic widgets.
   /// The default value for this property is false.
   ///
   Q_PROPERTY (bool    enableEditPv  READ getEnableEditPv  WRITE setEnableEditPv)

   /// The default is useAliasName, however the default aliasName are undefined,
   /// so the effective default is usePvName.
   ///
   Q_PROPERTY (QE::PVLabelMode pvLabelMode READ getPVLabelMode  WRITE setPVLabelMode)

public:
   // Abstract Dynamic Widget Context Menu values
   //
   enum OwnContextMenuOptions {
      ADWCM_NONE = CM_SPECIFIC_WIDGETS_START_HERE,
      ADWCM_LOAD_WIDGET_CONFIG,
      ADWCM_SAVE_WIDGET_CONFIG,
      ADWCM_SELECT_USE_PV_NAME,
      ADWCM_SELECT_USE_ALIAS_NAME,
      ADWCM_SELECT_USE_DESCRIPTION,
      ADWCM_SUB_CLASS_WIDGETS_START_HERE
   };

   explicit QEAbstractDynamicWidget (QWidget* parent = 0);
   ~QEAbstractDynamicWidget ();

   // Set/Get default configuration load/save directory.
   //
   void setDefaultDir (const QString& defaultDir);
   QString getDefaultDir () const;

   // Set/Get enableEditPv.
   //
   void setEnableEditPv (const bool isEnabled);
   bool getEnableEditPv () const;

   // Set/Get pvLabelMode.
   //
   void setPVLabelMode (const QE::PVLabelMode pvLabelMode);
   QE::PVLabelMode getPVLabelMode () const;

   // Override paste. This functions adds PVs names to the next
   // available slot(s) if any.
   //
   void paste (QVariant s);

   // Used by paste and addPvNameSet, but also made publically available.
   // May be overridden. See QEPvProperties for a concerete example of this.
   //
   virtual void addPvNameList (const QStringList& pvNameList);

   // Split input string using white space as delimiter.
   //
   void addPvNameSet (const QString& pvNameSet);

   // Add PV to next available slot (if any).
   // returns slot number 0 .. Max - 1 iff successful otherwise -1.
   //
   // Sub-class MUST provide an implementation for this function.
   //
   virtual int addPvName (const QString& pvName) = 0; // Add name to next available slot.

   // Remove and clear all PVs.
   // Sub-class MUST provide an implementation for this function.
   //
   virtual void clearAllPvNames () = 0;

protected:
   QMenu* buildContextMenu ();                        // Build the specific context menu
   void addPVLabelModeContextMenu (QMenu* menu);      // Add in optional PV label mode selection
   void contextMenuTriggered (int selectedItemNum);   // An action was selected from the context menu

   QString getPersistantName () const;                // Sub classes use this in lieu of QEWidget::persistantName

   // Sub-class may provide an implementation for these functions.
   //
   virtual void pvLabelModeChanged ();                // Used for sub-class notification
   virtual void enableEditPvChanged ();               // Used for sub-class notification

   // Sub-class convienence utility functions.
   //
   QE::UserLevels minimumEditPvUserLevel () const;
   void setStandardFormat (QELabel* dataLabel);

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
   QString getPersistantRootName () const;

   bool useOwnPersistantName;
   QString defaultDir;
   bool enableEditPv;
   QE::PVLabelMode pvLabelMode;
};

#endif // QE_ABSTRACT_DYNAMIC_WIDGET_H
