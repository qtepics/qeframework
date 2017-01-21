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
 *  Copyright (c) 2015 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

// This dialog is used to prompt a user for a new legend for a markup being shown over an
// image in a QEImage widget.
// This dialog is accessed from the markup context menu.

#ifndef IMAGEMARKUPLEGENDSETTEXT_H
#define IMAGEMARKUPLEGENDSETTEXT_H

#include <QDialog>

namespace Ui {
    class imageMarkupLegendSetText;
}

class imageMarkupLegendSetText : public QDialog
{
    Q_OBJECT

public:
    explicit imageMarkupLegendSetText( QString existingLegend, QWidget *parent = 0);
    ~imageMarkupLegendSetText();
    QString getLegend();

private:
    Ui::imageMarkupLegendSetText *ui;
};

#endif // IMAGEMARKUPLEGENDSETTEXT_H
