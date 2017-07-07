/*  QEDragDrop.h
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
 *  Copyright (c) 2009,2010,2014,2017 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#ifndef QE_DRAG_DROP_H
#define QE_DRAG_DROP_H

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QObject>
#include <QEActionRequests.h>

#include <QEFrameworkLibraryGlobal.h>

class QEWidget;

// Instance of this created and owned by QEDragDrop which itself cannot be based on a QObject
class QEDragDropObject : public QObject
{
    Q_OBJECT
public:
    QEDragDropObject( QObject* parent );      // Construction
    ~QEDragDropObject();                      // Destruction
    void sendRequestAction( const QEActionRequests& request );
signals:
    void requestAction( const QEActionRequests& );
};


class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEDragDrop {

public:
    QEDragDrop( QEWidget* qewIn, QWidget* ownerIn );
    virtual ~QEDragDrop(){}

    void setAllowDrop( bool allowDropIn );
    bool getAllowDrop() const;
protected:

    // Drag and Drop

    // Called by the QE widgets in the QE widgets's drag/drop implementation.
    // Set allowSelfDrop to allow dropping onto source.
    void qcaDragEnterEvent(QDragEnterEvent *event, const bool allowSelfDrop = false);

    // Handles drop event. When the drop data is textual, only the first part of the text
    // is dropped unless allText parameter is set true in which case all the available
    // text is dropped.
    void qcaDropEvent(QDropEvent *event, const bool allText = false);

    // left button: initiates drag-drop
    // middle botton: performs copy variable name to paste buffer.
    // Note: while the middle button processing is not part of drag/drop per se,
    // this is the location of the standard qcaMousePressEvent function.
    //
    void qcaMousePressEvent(QMouseEvent* event);

    // Virtual functions to allow this class to get and set the QE widgets drag/drop text
    // They are not defined as pure virtual as the QE widgets does not have to use this class's drag drop.
    virtual void setDrop( QVariant ) {}
    virtual QVariant getDrop() { return QVariant(); }

protected:
    void setDragDropConsumer (QObject* consumer);  // Set the consumer of the signal generted by this object

private:
    void initiateDragDrop(QMouseEvent* event);     // Initiates drag-drop
    void postPvInformation();                      // Use use message with PV name
    void examinePVProperties();                    // Request Examine PV Properties
    void plotInStripChart();                       // Request Plot in StripChart

    QEDragDropObject* object;
    QWidget* owner;
    QEWidget* qew;                                 // QEWidget associated with this instance
    bool allowDrop;
};

#endif // QE_DRAG_DROP_H
