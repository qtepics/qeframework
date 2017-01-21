/*  QEWidgetProperties.h
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

#ifndef QE_WIDGET_PROPERTIES_H
#define QE_WIDGET_PROPERTIES_H

#include <QObject>
#include <QEPluginLibrary_global.h>

/// This class defines common widget property types.
///
/// This class is not intended to be inherited by any other class.
/// The use of the class is to provide a name space and allow moc to
/// generate the asociated meta data.
///
class QEPLUGINLIBRARYSHARED_EXPORT QEWidgetProperties : public QObject {
    Q_OBJECT
public:

   /// Specifies how a (control) widget should behave if/when the underlying
   /// record becomes disabled (i.e. DISA and DISV fields become equal).
   enum DisabledRecordPolicy {
      ignore = 0,     ///< No change in appearance - the default
      grayout         ///< Grayed out apperance, i.e. same as when the widgest PV is disconnected
                      // Maybe as disable at a later date.
   };
   Q_ENUMS (DisabledRecordPolicy)

   // TODO relocate other common property types here.

   QEWidgetProperties (QObject* parent = 0);
   ~QEWidgetProperties ();
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEWidgetProperties::DisabledRecordPolicy)
#endif

#endif  // QE_WIDGET_PROPERTIES_H
