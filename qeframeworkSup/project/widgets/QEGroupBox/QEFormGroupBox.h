/*  QEFormGroupBox.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2021-2022 Australian Synchrotron
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
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_FORM_GROUP_BOX_H
#define QE_FORM_GROUP_BOX_H

#include <QGroupBox>
#include <QHBoxLayout>
#include <QWidget>
#include <QFrame>
#include <QECommon.h>
#include <persistanceManager.h>
#include <UserMessage.h>
#include <QEGroupBox.h>
#include <QEForm.h>
#include <QEFrameworkLibraryGlobal.h>

/// This widget is a group box that contains a QEForm widget, which allows
/// arbitary ui files to be loaded - see QEForm doco and comments for details.
/// The group box is set checkable, and when un checked the group box's height
/// is set to all is tilte to be seen, but non of the contents. When checked
/// the group box will expand to accomodate the loaded ui file.
///
/// Note: this widget takes control/management of it's minimum and maximum
/// width and heights, so setting in designer will have effectively no impact.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEFormGroupBox : public QEGroupBox
{
   Q_OBJECT
public:
   // The setCheckable is set and should be left true for this widget.
   //
   Q_PROPERTY (bool checkable READ isCheckable WRITE setCheckable DESIGNABLE false)

   /// The following propeties are directly copied form QEFrom - see QEForm
   /// docoumentation and code comments for details, the the exception of the
   /// messageSourceId porperty which has been rebagded to formMessageSourceId
   /// to avoid conflict with QEFormGroupBox own messageSourceId porperty.
   // Please keep these aligned with QEFrom.
   //
   Q_PROPERTY (QString uiFile
               READ getUiFileNameProperty         WRITE setUiFileNameProperty)
   Q_PROPERTY (QString variableSubstitutions
               READ getVariableNameSubstitutionsProperty
               WRITE setVariableNameSubstitutionsProperty)
   Q_PROPERTY (bool handleGuiLaunchRequests
               READ getHandleGuiLaunchRequests    WRITE setHandleGuiLaunchRequests)
   Q_PROPERTY (bool resizeContents
               READ getResizeContents             WRITE setResizeContents)
   Q_PROPERTY (unsigned int formMessageSourceId
               READ getMessageSourceId            WRITE setMessageSourceId)
   Q_PROPERTY (QEForm::MessageFilterOptions messageFormFilter
               READ getMessageFormFilter          WRITE setMessageFormFilter)
   Q_PROPERTY (QEForm::MessageFilterOptions messageSourceFilter
               READ getMessageSourceFilter        WRITE setMessageSourceFilter)

   // QEFormGroupBox specific properties.

   /// When true, the frame shape of the inner frame widget is set to NoShape
   /// provided a unique QFrame (or QEFrame/other sub classes) is found at the
   /// top level of the loaded form.
   //
   Q_PROPERTY (bool hideInnerFrameShape
               READ getHideInnerFrameShape        WRITE setHideInnerFrameShape)

   /// When true, the form title is extracted from the loaded form and
   /// used to set the group box title.
   //
   Q_PROPERTY (bool useFormTitle
               READ getUseFormTitle               WRITE setUseFormTitle)

public:
   explicit QEFormGroupBox (QWidget* parent = 0);
   virtual ~QEFormGroupBox ();

   QSize sizeHint () const;

   // Expose access to the internal QEForm widget's set/get functions.
   //
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (form, QString, getUiFileNameProperty, setUiFileNameProperty);
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (form, QString, getVariableNameSubstitutionsProperty,  setVariableNameSubstitutionsProperty);
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (form, bool, getHandleGuiLaunchRequests, setHandleGuiLaunchRequests);
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (form, bool, getResizeContents ,  setResizeContents);
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (form, unsigned int, getMessageSourceId,  setMessageSourceId);
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (form, QEForm::MessageFilterOptions, getMessageFormFilter,  setMessageFormFilter);
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (form, QEForm::MessageFilterOptions, getMessageSourceFilter, setMessageSourceFilter);

   void setHideInnerFrameShape (const bool useFormTitle);
   bool getHideInnerFrameShape () const;

   void setUseFormTitle (const bool useFormTitle);
   bool getUseFormTitle () const;

public slots:
   // Slot function equivilents of setUiFileNameProperty and setVariableNameSubstitutionsProperty
   //
   void setUiFileName (const QString& uiFileName);
   void setUiFileSubstitutions (const QString& uiFileNameSubstitutions);

protected:
   // Override parents virtual functions.
   //
   void showEvent (QShowEvent* event);
   void saveConfiguration (PersistanceManager* pm);
   void restoreConfiguration (PersistanceManager* pm, restorePhases restorePhase);

private:
   void updateBoxTitle();
   void updateInnerFrameShape();
   void updateBoxMinMaxSizes();
   QFrame* findUniqueFrame ();

   bool hideInnerFrameShape;
   QFrame::Shape loadedFrameShape;

   bool useFormTitle;
   QString loadedFormTitle;

   // Internal widgets
   //
   QHBoxLayout* layout;
   QEForm* form;
   QWidget* spacer;

private slots:
   void onFormLoaded (bool);
   void onGroupBoxClick (bool);
};

#endif     // QE_FORM_GROUP_BOX_H
