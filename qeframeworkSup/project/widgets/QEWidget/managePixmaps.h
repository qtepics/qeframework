/*  managePixmaps.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2011-2021 Australian Synchrotron
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
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#ifndef QE_MANAGE_PIXMAP_H
#define QE_MANAGE_PIXMAP_H

#include <QList>
#include <QPixmap>
#include <QEFrameworkLibraryGlobal.h>

#define NUM_PIXMAPS_MANAGED 8

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT managePixmaps {

public:
   explicit managePixmaps();
   virtual ~managePixmaps();

    // Property convenience functions

    // Pixmaps
    void setDataPixmap( const QPixmap& Pixmap, const int index );
    QPixmap getDataPixmap( const int index ) const;

    // Treat string as testual representation of numeric index value.
    //
    QPixmap getDataPixmap( const QString& value ) const;

    // Access function for pixmap properties.
    // Currently used by QELabel, QEPushButton etc. and QEFrame.
    //
#define PIXMAP_ACCESS(n)                                                                   \
    void setPixmap##n##Property( const QPixmap& pixmap ) { setDataPixmap( pixmap, n ); }   \
    QPixmap getPixmap##n##Property() const { return getDataPixmap( n ); }


    PIXMAP_ACCESS (0)
    PIXMAP_ACCESS (1)
    PIXMAP_ACCESS (2)
    PIXMAP_ACCESS (3)
    PIXMAP_ACCESS (4)
    PIXMAP_ACCESS (5)
    PIXMAP_ACCESS (6)
    PIXMAP_ACCESS (7)

#undef PIXMAP_ACCESS

protected:
    // Called when a specific pix map is updated.
    virtual void pixmapUpdated( const int index );

private:
    QList<QPixmap> pixmaps;
};

#endif // QE_MANAGE_PIXMAP_H
