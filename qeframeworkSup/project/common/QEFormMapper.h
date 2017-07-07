/*  QEFormMapper.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
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
 *  Copyright (c) 2015,2017 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_FORM_MAPPER_H
#define QE_FORM_MAPPER_H

#include <QtCore/qglobal.h>
#include <QHash>
#include <QObject>
#include <QWidget>
#include <QString>

#include <QEFrameworkLibraryGlobal.h>

/// The QEFormMapper mapper class is a singleton class that allows a user allocated
/// form handle to be associated with an QEForm, specifically the form handle is
/// included in the action request (along with the ui filename, macros etc.) emitted
/// and then received up by the QEGui (or other) display manager. When the QEForm object
/// is created the user alllocated handle is associated with the QEForm object.
/// This allows the originator of the request to find the QEForm that was created
/// as a result of the request.
///
// This is a singleton class - the single instance is declared in the .cpp file.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEFormMapper
{
public:
   // Handle type used to identify a QEForm. The type should be considered private.
   //
   typedef quint64 FormHandles;

   static FormHandles nullHandle ();        // provides the null handle identifier value
   static FormHandles allocateHandle ();    // allocate unique handle identifier

   // Find the widget with the specified handle - handle specified in the action request.
   // These functions return NULL if the widget is not found.
   //
   // NOTE: Forms and the widgets they contain may disapper at any time,
   // i.e. when a main window form is closed. Therefore the QEForm/QWidget
   // references returnd by these MUST ONLY BE CONSIDERED VALID within the scope
   // of the function calling these procedures. Therefore, DO NOT store these
   // references for laters use. Only store as local variable.
   //
   // OKAY:
   //   void goodBehaviour () {
   //      QWidget* w;
   //      w = findWidget (handle, "QEPlotter");
   //      w->callSomeFunction();
   //   }  // w goes out of scope.
   //
   // BAD:
   //   void askingForTrouble () {
   //      this->w = findWidget (handle, "QEPlotter"); // save ref for later use.
   //   }
   //
   //   void someTimeLater () {
   //      this->w->callSomeFunction();
   //   }
   //
   static QWidget* findWidget (const FormHandles& handle);       // find form via unique handle.
   static QWidget* findWidget (const FormHandles& handle,        // find widget using handle and class name.
                               const QString& className);

private:
   // We make the constructor private to ensure only a single instance created.
   //
   explicit QEFormMapper ();
   ~QEFormMapper ();

   typedef QHash <FormHandles, QWidget*> FormMaps;

   FormMaps map;
   FormHandles nextId;

   // Ensure the singleton object is created.
   //
   static bool initialise ();
   static bool initialised;

   // Returns true if successfull, i.e. handle must not aleady be in use.
   //
   bool associateHandleWithWidget (const FormHandles& handle, QWidget* widget);

   void disasociatedHandleAndWidget (const QEFormMapper::FormHandles& handle);

   friend class QEMapable;
};


//------------------------------------------------------------------------------
// This class enables a QEForm (or any other widget) to be mapable, i.e.
// allows a unique identifer to be associated with the QEForm.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEMapable {
public:
   explicit QEMapable (QWidget* ownerIn);
   ~QEMapable ();

   // Attempt to associate the specified handle with this object.
   // Will fail if handle not unique (unless set null which always okay)
   //
   bool setFormHandle (const QEFormMapper::FormHandles& formHandle);

   QEFormMapper::FormHandles getFormHandle () const;

private:
   QWidget* owner;
   QEFormMapper::FormHandles formHandle;
};

#endif // QE_FORM_MAPPER_H
