/*  QEDesignerPlugin.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2009-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_DESIGNER_PLUGIN_H
#define QE_DESIGNER_PLUGIN_H

#include <QtPlugin>
#include <QtGlobal>
#if QT_VERSION >= 0x050500
    #include <QtUiPlugin/QDesignerCustomWidgetCollectionInterface>
#else
    #include <QDesignerCustomWidgetCollectionInterface>
#endif

class QEWidgets: public QObject, public QDesignerCustomWidgetCollectionInterface {
    Q_OBJECT
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    Q_PLUGIN_METADATA(IID "au.org.synchrotron.qeframework" FILE "qeplugin.json")
#endif
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)

  public:
    QEWidgets(QObject *parent = 0);
    virtual ~QEWidgets(){}

    virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const;

  private:
    QList<QDesignerCustomWidgetInterface*> widgets;
};

#endif // QE_DESIGNER_PLUGIN_H
