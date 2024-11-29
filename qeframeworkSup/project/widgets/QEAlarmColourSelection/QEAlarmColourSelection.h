/* qecolourselection.h
 */

#ifndef QE_COLOUR_SELECTION_H
#define QE_COLOUR_SELECTION_H

#include <QObject>
#include <QWidget>
#include <QFrame>
#include <QColorDialog>
#include <QPushButton>
#include <QEFrameworkLibraryGlobal.h>

namespace Ui {
   class QEAlarmColourSelection;
}

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEAlarmColourSelection : public QFrame
{
   Q_OBJECT
public:
   explicit QEAlarmColourSelection (QWidget* parent = 0);
   ~QEAlarmColourSelection ();

private:
   Ui::QEAlarmColourSelection* ui;
   QColorDialog* colourDialog;
   QPushButton* buttons [2][5];

   void setAllButtonStyles ();

private slots:
   void postConstruction ();
   void onSelectionClicked (bool);
};

#endif  // QE_COLOUR_SELECTION_H
