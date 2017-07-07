/*  PeriodicDialog.h
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
 *  Copyright (c) 2011 Australian Synchrotron
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

namespace Ui {
    class PeriodicDialog;
}

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT PeriodicDialog : public QEDialog {
    Q_OBJECT
public:
    PeriodicDialog(QWidget *parent = 0);
    ~PeriodicDialog();

    QString getElement();
    void setElement( QString elementIn, QList<bool>& enabledList, QList<QString>& elementList );


protected:
    void changeEvent(QEvent *e);

private:
    Ui::PeriodicDialog *m_ui;
    QString elementSelected;
    void noteElementSelected();

private slots:
    void on_pushButton_001_clicked(){ noteElementSelected(); }
    void on_pushButton_002_clicked(){ noteElementSelected(); }
    void on_pushButton_003_clicked(){ noteElementSelected(); }
    void on_pushButton_004_clicked(){ noteElementSelected(); }
    void on_pushButton_005_clicked(){ noteElementSelected(); }
    void on_pushButton_006_clicked(){ noteElementSelected(); }
    void on_pushButton_007_clicked(){ noteElementSelected(); }
    void on_pushButton_008_clicked(){ noteElementSelected(); }
    void on_pushButton_009_clicked(){ noteElementSelected(); }
    void on_pushButton_010_clicked(){ noteElementSelected(); }
    void on_pushButton_011_clicked(){ noteElementSelected(); }
    void on_pushButton_012_clicked(){ noteElementSelected(); }
    void on_pushButton_013_clicked(){ noteElementSelected(); }
    void on_pushButton_014_clicked(){ noteElementSelected(); }
    void on_pushButton_015_clicked(){ noteElementSelected(); }
    void on_pushButton_016_clicked(){ noteElementSelected(); }
    void on_pushButton_017_clicked(){ noteElementSelected(); }
    void on_pushButton_018_clicked(){ noteElementSelected(); }
    void on_pushButton_019_clicked(){ noteElementSelected(); }
    void on_pushButton_020_clicked(){ noteElementSelected(); }
    void on_pushButton_021_clicked(){ noteElementSelected(); }
    void on_pushButton_022_clicked(){ noteElementSelected(); }
    void on_pushButton_023_clicked(){ noteElementSelected(); }
    void on_pushButton_024_clicked(){ noteElementSelected(); }
    void on_pushButton_025_clicked(){ noteElementSelected(); }
    void on_pushButton_026_clicked(){ noteElementSelected(); }
    void on_pushButton_027_clicked(){ noteElementSelected(); }
    void on_pushButton_028_clicked(){ noteElementSelected(); }
    void on_pushButton_029_clicked(){ noteElementSelected(); }
    void on_pushButton_030_clicked(){ noteElementSelected(); }
    void on_pushButton_031_clicked(){ noteElementSelected(); }
    void on_pushButton_032_clicked(){ noteElementSelected(); }
    void on_pushButton_033_clicked(){ noteElementSelected(); }
    void on_pushButton_034_clicked(){ noteElementSelected(); }
    void on_pushButton_035_clicked(){ noteElementSelected(); }
    void on_pushButton_036_clicked(){ noteElementSelected(); }
    void on_pushButton_037_clicked(){ noteElementSelected(); }
    void on_pushButton_038_clicked(){ noteElementSelected(); }
    void on_pushButton_039_clicked(){ noteElementSelected(); }
    void on_pushButton_040_clicked(){ noteElementSelected(); }
    void on_pushButton_041_clicked(){ noteElementSelected(); }
    void on_pushButton_042_clicked(){ noteElementSelected(); }
    void on_pushButton_043_clicked(){ noteElementSelected(); }
    void on_pushButton_044_clicked(){ noteElementSelected(); }
    void on_pushButton_045_clicked(){ noteElementSelected(); }
    void on_pushButton_046_clicked(){ noteElementSelected(); }
    void on_pushButton_047_clicked(){ noteElementSelected(); }
    void on_pushButton_048_clicked(){ noteElementSelected(); }
    void on_pushButton_049_clicked(){ noteElementSelected(); }
    void on_pushButton_050_clicked(){ noteElementSelected(); }
    void on_pushButton_051_clicked(){ noteElementSelected(); }
    void on_pushButton_052_clicked(){ noteElementSelected(); }
    void on_pushButton_053_clicked(){ noteElementSelected(); }
    void on_pushButton_054_clicked(){ noteElementSelected(); }
    void on_pushButton_055_clicked(){ noteElementSelected(); }
    void on_pushButton_056_clicked(){ noteElementSelected(); }
    void on_pushButton_057_clicked(){ noteElementSelected(); }
    void on_pushButton_058_clicked(){ noteElementSelected(); }
    void on_pushButton_059_clicked(){ noteElementSelected(); }
    void on_pushButton_060_clicked(){ noteElementSelected(); }
    void on_pushButton_061_clicked(){ noteElementSelected(); }
    void on_pushButton_062_clicked(){ noteElementSelected(); }
    void on_pushButton_063_clicked(){ noteElementSelected(); }
    void on_pushButton_064_clicked(){ noteElementSelected(); }
    void on_pushButton_065_clicked(){ noteElementSelected(); }
    void on_pushButton_066_clicked(){ noteElementSelected(); }
    void on_pushButton_067_clicked(){ noteElementSelected(); }
    void on_pushButton_068_clicked(){ noteElementSelected(); }
    void on_pushButton_069_clicked(){ noteElementSelected(); }
    void on_pushButton_070_clicked(){ noteElementSelected(); }
    void on_pushButton_071_clicked(){ noteElementSelected(); }
    void on_pushButton_072_clicked(){ noteElementSelected(); }
    void on_pushButton_073_clicked(){ noteElementSelected(); }
    void on_pushButton_074_clicked(){ noteElementSelected(); }
    void on_pushButton_075_clicked(){ noteElementSelected(); }
    void on_pushButton_076_clicked(){ noteElementSelected(); }
    void on_pushButton_077_clicked(){ noteElementSelected(); }
    void on_pushButton_078_clicked(){ noteElementSelected(); }
    void on_pushButton_079_clicked(){ noteElementSelected(); }
    void on_pushButton_080_clicked(){ noteElementSelected(); }
    void on_pushButton_081_clicked(){ noteElementSelected(); }
    void on_pushButton_082_clicked(){ noteElementSelected(); }
    void on_pushButton_083_clicked(){ noteElementSelected(); }
    void on_pushButton_084_clicked(){ noteElementSelected(); }
    void on_pushButton_085_clicked(){ noteElementSelected(); }
    void on_pushButton_086_clicked(){ noteElementSelected(); }
    void on_pushButton_087_clicked(){ noteElementSelected(); }
    void on_pushButton_088_clicked(){ noteElementSelected(); }
    void on_pushButton_089_clicked(){ noteElementSelected(); }
    void on_pushButton_090_clicked(){ noteElementSelected(); }
    void on_pushButton_091_clicked(){ noteElementSelected(); }
    void on_pushButton_092_clicked(){ noteElementSelected(); }
    void on_pushButton_093_clicked(){ noteElementSelected(); }
    void on_pushButton_094_clicked(){ noteElementSelected(); }
    void on_pushButton_095_clicked(){ noteElementSelected(); }
    void on_pushButton_096_clicked(){ noteElementSelected(); }
    void on_pushButton_097_clicked(){ noteElementSelected(); }
    void on_pushButton_098_clicked(){ noteElementSelected(); }
    void on_pushButton_099_clicked(){ noteElementSelected(); }
    void on_pushButton_100_clicked(){ noteElementSelected(); }
    void on_pushButton_101_clicked(){ noteElementSelected(); }
    void on_pushButton_102_clicked(){ noteElementSelected(); }
    void on_pushButton_103_clicked(){ noteElementSelected(); }
    void on_pushButton_104_clicked(){ noteElementSelected(); }
    void on_pushButton_105_clicked(){ noteElementSelected(); }
    void on_pushButton_106_clicked(){ noteElementSelected(); }
    void on_pushButton_107_clicked(){ noteElementSelected(); }
    void on_pushButton_108_clicked(){ noteElementSelected(); }
    void on_pushButton_109_clicked(){ noteElementSelected(); }
    void on_pushButton_110_clicked(){ noteElementSelected(); }
    void on_pushButton_111_clicked(){ noteElementSelected(); }
    void on_pushButton_112_clicked(){ noteElementSelected(); }
    void on_pushButton_113_clicked(){ noteElementSelected(); }
    void on_pushButton_114_clicked(){ noteElementSelected(); }
    void on_pushButton_115_clicked(){ noteElementSelected(); }
    void on_pushButton_116_clicked(){ noteElementSelected(); }
    void on_pushButton_117_clicked(){ noteElementSelected(); }
    void on_pushButton_118_clicked(){ noteElementSelected(); }

};

#endif // QE_PERIODIC_DIALOG_H
