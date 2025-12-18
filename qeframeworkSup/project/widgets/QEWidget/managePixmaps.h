/*  managePixmaps.h
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

#ifndef QE_MANAGE_PIXMAP_H
#define QE_MANAGE_PIXMAP_H

#include <QList>
#include <QPixmap>
#include <QEFrameworkLibraryGlobal.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEManagePixmaps {
public:
   enum Constants {
      // A bit arbitary, however is in line with mbbi/mbbo records.
      NUM_PIXMAPS_MANAGED = 16
   };

   explicit QEManagePixmaps();
   virtual ~QEManagePixmaps();

    // Property convenience functions

    // Pixmaps
    void setDataPixmap (const QPixmap& Pixmap, const int index);
    QPixmap getDataPixmap (const int index ) const;

    // Treat string as textual representation of numeric index value.
    //
    QPixmap getDataPixmap (const QString& value) const;

    // Access function for pixmap properties.
    // Currently used by QELabel, QEPushButton etc. and QEFrame.
    //
#define PIXMAP_ACCESS(n)                                                                 \
    void setPixmap##n##Property (const QPixmap& pixmap) { setDataPixmap (pixmap, n); }   \
    QPixmap getPixmap##n##Property() const { return getDataPixmap (n); }


    PIXMAP_ACCESS (0)
    PIXMAP_ACCESS (1)
    PIXMAP_ACCESS (2)
    PIXMAP_ACCESS (3)
    PIXMAP_ACCESS (4)
    PIXMAP_ACCESS (5)
    PIXMAP_ACCESS (6)
    PIXMAP_ACCESS (7)
    PIXMAP_ACCESS (8)
    PIXMAP_ACCESS (9)
    PIXMAP_ACCESS (10)
    PIXMAP_ACCESS (11)
    PIXMAP_ACCESS (12)
    PIXMAP_ACCESS (13)
    PIXMAP_ACCESS (14)
    PIXMAP_ACCESS (15)

#undef PIXMAP_ACCESS

protected:
    // Called when a specific pix map is updated.
    virtual void pixmapUpdated (const int index);

private:
    QList<QPixmap> pixmaps;
};

#endif // QE_MANAGE_PIXMAP_H
