/*  zoomMenu.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2012-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

/*
 This class manages the zoom menu for the QEImage widget
 */

#ifndef ZOOMMENU_H
#define ZOOMMENU_H

#include <QMenu>
#include <imageContextMenu.h>

class zoomMenu : public QMenu
{
    Q_OBJECT
public:

    explicit zoomMenu( QWidget *parent = 0 );

    void enableAreaSelected( bool enable );
    imageContextMenu::imageContextMenuOptions getZoom( const QPoint& pos );

signals:

public slots:

private:
    QAction* areaSelectedAction;

};

#endif // ZOOMMENU_H
