/* QEAlarmColourSelection.cpp
 *
 */

#include "QEAlarmColourSelection.h"
#include <QDebug>
#include <QTimer>
#include <QECommon.h>
#include <QCaAlarmInfo.h>
#include <ui_QEAlarmColourSelection.h>

#define DEBUG  qDebug () << "QEAlarmColourSelection" << __LINE__ <<  __FUNCTION__  << "  "

// Alias for brevity
typedef QCaAlarmInfoColorNamesManager  cnm;

//------------------------------------------------------------------------------
//
QEAlarmColourSelection::QEAlarmColourSelection (QWidget* parent) :
   QFrame (parent),
   ui (new Ui::QEAlarmColourSelection)
{
   this->ui->setupUi (this);
   this->colourDialog = new QColorDialog (this);

   for (int s = 0; s < ARRAY_LENGTH (this->buttons); s++) {
      for (int a = 0; a < ARRAY_LENGTH (this->buttons); a++) {
         this->buttons[s][a] = NULL;
      }
   }

   QTimer::singleShot (5, this, SLOT (postConstruction ()));
}

//------------------------------------------------------------------------------
//
QEAlarmColourSelection::~QEAlarmColourSelection ()
{ }


//------------------------------------------------------------------------------
//
void QEAlarmColourSelection::postConstruction ()
{
   // Form an array of buttons.
   //
   this->buttons[0][0] = this->ui->pushButton_R10;
   this->buttons[0][1] = this->ui->pushButton_R11;
   this->buttons[0][2] = this->ui->pushButton_R12;
   this->buttons[0][3] = this->ui->pushButton_R13;
   this->buttons[0][4] = this->ui->pushButton_R14;

   this->buttons[1][0] = this->ui->pushButton_R20;
   this->buttons[1][1] = this->ui->pushButton_R21;
   this->buttons[1][2] = this->ui->pushButton_R22;
   this->buttons[1][3] = this->ui->pushButton_R23;
   this->buttons[1][4] = this->ui->pushButton_R24;

   this->setAllButtonStyles ();

   for (int s = 0; s < ARRAY_LENGTH (this->buttons); s++) {
      for (int a = 0; a < ARRAY_LENGTH (this->buttons[s]); a++) {
         QPushButton* button = this->buttons[s][a];
         if (!button) continue;

         QObject::connect (button, SIGNAL (clicked (bool)),
                           this,   SLOT (onSelectionClicked (bool)));

         QEUtilities::tagObject (button, 10*s + a);
      }
   }
}

//------------------------------------------------------------------------------
//
void QEAlarmColourSelection::setAllButtonStyles ()
{
   for (int s = 0; s < ARRAY_LENGTH (this->buttons); s++) {
      QStringList names = (s == 0) ?  cnm::getInUseStyleColorNames() :
                                      cnm::getInUseColorNames();

      for (int a = 0; a < ARRAY_LENGTH (this->buttons[s]); a++) {
         QPushButton* button = this->buttons[s][a];
         if (!button) continue;

         QColor colour (names.value(a));

         // colourToStyle sets the font color to white or black as appropriate.
         //
         QString style = QEUtilities::colourToStyle (colour);
         button->setStyleSheet (style);
      }
   }
}

//------------------------------------------------------------------------------
//
void QEAlarmColourSelection::onSelectionClicked (bool)
{
   QPushButton* button = qobject_cast<QPushButton*>(this->sender());
   if (!button) return;   // safety check

   const int tag = QEUtilities::objectTag (button);
   if (tag < 0) return;   // safety check

   const int s = tag / 10;
   const int a = tag % 10;
   if (s >=2 || a >= 5) return;   // safety check

   // Get the unuse names
   //
   QStringList names = (s == 0) ?  cnm::getInUseStyleColorNames() :
                                   cnm::getInUseColorNames();
   QColor colour (names.value(a));

   this->colourDialog->setCurrentColor (colour);
   int status = this->colourDialog->exec();
   if (status == 1) {
      colour = this->colourDialog->currentColor ();

      QString name = colour.name();
      names.replace(a, name);

      // Update the priority names
      //
      if (s == 0) {
         cnm::setStyleColorNames (cnm::cnkOverride, names);
      } else {
         cnm::setColorNames (cnm::cnkOverride, names);
      }

      this->setAllButtonStyles ();
   }
}

// end
