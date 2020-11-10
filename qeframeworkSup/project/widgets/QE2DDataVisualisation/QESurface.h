/*  QESurface.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2020 Australian Synchrotron
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
 *    andrews@ansto.gov.au
 */

#ifndef QE_SURFACE_H
#define QE_SURFACE_H

#include <QString>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QEFrameworkLibraryGlobal.h>
#include <QEAbstract2DData.h>

// Differed declarations - these are only available Qt5 and later.
//
namespace QtDataVisualization {
   class Q3DSurface;
   class QSurfaceDataProxy;
}

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QESurface :
   public QEAbstract2DData
{
   Q_OBJECT

   Q_PROPERTY (QString text READ getText WRITE setText)

public:
   /// Create without a variable.
   /// Use setDataPvName () and setVariableNameSubstitutions() to define a
   /// variable and, optionally, macro substitutions later.
   ///
   explicit QESurface (QWidget* parent = 0);

   /// Create with a variable (s).
   /// A connection is automatically established.
   /// If macro substitutions are required, create without a variable and
   /// set the variable and macro substitutions after creation.
   ///
   explicit QESurface (const QString& dataVariableName,
                       QWidget* parent = 0);

   explicit QESurface (const QString& dataVariableName,
                       const QString& widthVariableName,
                       QWidget* parent = 0);

   /// Destruction
   virtual ~QESurface ();

   // Property access functions
   //
   void setText (const QString& text);
   QString getText () const;

protected:
   // Override parent virtual functions.
   //
   QSize sizeHint () const;
   void updateDataVisulation ();   // hook function

private:
   void commonSetup ();

   QPushButton* button;    // internal button widget
   QHBoxLayout* layout;    // holds the button widget - any layout type will do
   QtDataVisualization::Q3DSurface* surface;
   QtDataVisualization::QSurfaceDataProxy* surfaceProxy;

private slots:
   void initialise();
   void onClick (bool);
};

#endif  // QE_SURFACE_H
