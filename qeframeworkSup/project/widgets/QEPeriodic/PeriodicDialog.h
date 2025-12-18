/*  PeriodicDialog.h
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

#ifndef QE_PERIODIC_DIALOG_H
#define QE_PERIODIC_DIALOG_H

#include <QDialog>
#include <QEFrameworkLibraryGlobal.h>
#include <QEDialog.h>
#include <QEOneToOne.h>

namespace Ui {
    class PeriodicDialog;
}

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT PeriodicDialog : public QEDialog 
{
    Q_OBJECT
public:
    PeriodicDialog( QWidget *parent = 0 );
    ~PeriodicDialog();

    // Colourise run-time element selection dialog
    void setColourised( const bool colouriseIn );
    bool isColourised() const;

    void setElement( QString elementIn,
                     QList<bool>& enabledList );

    QString getElementName() const; // selected element name
    QString getElement() const;     // selected element symbol
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
    QString selectedElementSymbol;
    QString selectedElementName;
    bool colourise;

    typedef QEOneToOne <int, QPushButton*> ElementSlotButtonMap;
    ElementSlotButtonMap map;

private slots:
    void noteElementSelected();
};

#endif // QE_PERIODIC_DIALOG_H
