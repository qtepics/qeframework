/*  PeriodicDialog.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2011-2018 Australian Synchrotron
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

#ifndef QE_PERIODIC_DIALOG_H
#define QE_PERIODIC_DIALOG_H

#include <QDialog>
#include <QEFrameworkLibraryGlobal.h>
#include <QEDialog.h>
#include <QEOneToOne.h>

namespace Ui {
    class PeriodicDialog;
}

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT PeriodicDialog : public QEDialog {
    Q_OBJECT
public:
    PeriodicDialog(QWidget *parent = 0);
    ~PeriodicDialog();

    void setElement( QString elementIn,
                     QList<bool>& enabledList );

    Q_DECL_DEPRECATED
    void setElement( QString elementIn,
                     QList<bool>& enabledList,
                     QList<QString>& elementList );

    QString getElement() const;     // selected element by symbol
    int getAtomicNumber() const;    // selected element by atomic number 1 .. 118

public slots:
    // Re-postion dialog to the centre of the specified widget.
    //
    int exec( QWidget* targetWidget );

protected:
    void changeEvent( QEvent *e );

private:
    Ui::PeriodicDialog *m_ui;
    int selectedAtomicNumber;
    QString elementSelected;

    typedef QEOneToOne <int, QPushButton*> ElementSlotButtonMap;
    ElementSlotButtonMap map;

private slots:
    void noteElementSelected();
};

#endif // QE_PERIODIC_DIALOG_H
