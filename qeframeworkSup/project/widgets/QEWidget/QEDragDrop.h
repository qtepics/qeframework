/*  QEDragDrop.h
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
    // Note: the allowSelfDrop parameter to allow dropping onto source
    //       has been removed as never used.
    //
    void qcaDragEnterEvent(QDragEnterEvent *event);

    // Handles drop event. When the drop data is textual, only the first part of the text
    // is dropped unless allText parameter is set true in which case all the available
    // text is dropped.
    //
    void qcaDropEvent(QDropEvent *event, const bool allText = false);

    // left button: initiates drag-drop
    // middle botton: performs copy variable name to paste buffer.
    // Note: while the middle button processing is not part of drag/drop per se,
    // this is the location of the standard qcaMousePressEvent function.
    //
    void qcaMousePressEvent(QMouseEvent* event);

    // Virtual functions to allow this class to get and set the QE widgets drag/drop text
    // They are not defined as pure virtual as the QE widgets does not have to use this class's drag drop.
    //
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
