/*
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
 *  Copyright (c) 2009, 2010 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#ifndef QCADESIGNERPLUGIN_H
#define QCADESIGNERPLUGIN_H

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

#endif // QCADESIGNERPLUGIN_H
