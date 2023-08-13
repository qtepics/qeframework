/*  QEMenuButtonArgumentsDialog.cpp
 *
 */

#include "QEMenuButtonArgumentsDialog.h"

#include <QDebug>
#include <QScrollBar>
#include <QTextCursor>
#include <QTimer>
#include <QECommon.h>
#include <ui_QEMenuButtonArgumentsDialog.h>

#define DEBUG  qDebug () << "QEMenuButtonArgumentsDialog" << __LINE__ << __FUNCTION__ << "  "

static const int noSelection = -1;

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
//
QEMenuButtonArgumentsDialog::QEMenuButtonArgumentsDialog (QWidget* parent) :
   QEDialog (parent),
   ui (new Ui::QEMenuButtonArgumentsDialog)
{
   this->ui->setupUi (this);
   this->listWidget = this->ui->argumentsTextEdit;
   this->argumentList.clear();

   // Setup connections.
   //
   QObject::connect (this->ui->newButton, SIGNAL (clicked        (bool)),
                     this,                SLOT   (onNewItemClick (bool)));

   QObject::connect (this->ui->deleteButton, SIGNAL (clicked           (bool)),
                     this,                   SLOT   (onDeleteItemClick (bool)));

   QObject::connect (this->ui->scrollUpButton, SIGNAL (clicked         (bool)),
                     this,                     SLOT   (onScrollUpClick (bool)));

   QObject::connect (this->ui->scrollDownButton, SIGNAL (clicked           (bool)),
                     this,                       SLOT   (onScrollDownClick (bool)));

   QObject::connect (this->listWidget, SIGNAL (itemSelectionChanged ()),
                     this,             SLOT   (rowSelectionChanged ()));

   QScrollBar* scrollBar = this->listWidget->verticalScrollBar();
   QObject::connect (scrollBar, SIGNAL (valueChanged       (int)),
                     this,      SLOT   (scrollValueChanged (int)));

   QObject::connect (this->ui->argumentEdit, SIGNAL (textEdited(const QString &)),
                     this,                   SLOT   (textEdited(const QString &)));
}

//------------------------------------------------------------------------------
//
QEMenuButtonArgumentsDialog::~QEMenuButtonArgumentsDialog ()
{
   this->argumentList.clear();
   delete this->ui;
}

//------------------------------------------------------------------------------
//
void QEMenuButtonArgumentsDialog::resizeEvent (QResizeEvent*)
{
   this->widgetsEnable ();
}

//------------------------------------------------------------------------------
//
void QEMenuButtonArgumentsDialog::setArgumentList (const QStringList& argumentListIn)
{
   this->argumentList = argumentListIn;

   this->loadArgumentList ();
   this->listWidget->setCurrentRow (noSelection);

   // We need a small delay for the initial scroll up to work as we would like.
   // Is this still true for the list widget
   //
   QTimer::singleShot (1, this, SLOT (setArgumentListPart2 ()));
}

//------------------------------------------------------------------------------
//
void QEMenuButtonArgumentsDialog::setArgumentListPart2 ()
{
   QScrollBar* scrollBar = this->listWidget->verticalScrollBar();
   scrollBar->setSliderPosition (0);
   this->widgetsEnable ();
}

//------------------------------------------------------------------------------
//
QStringList QEMenuButtonArgumentsDialog::getArgumentList () const
{
   return this->argumentList;
}

//------------------------------------------------------------------------------
//
void QEMenuButtonArgumentsDialog::widgetsEnable()
{
   QScrollBar* scrollBar = this->listWidget->verticalScrollBar();
   const int curPosn = scrollBar->sliderPosition();
   const int maxPosn = scrollBar->maximum();
   this->ui->scrollUpButton->setEnabled (curPosn > 0);
   this->ui->scrollDownButton->setEnabled (curPosn < maxPosn);

   this->ui->newButton->setEnabled (true);

   const int row = this->listWidget->currentRow();
   const int n = this->argumentList.count();

   bool weHaveASelectedRow = (row >= 0) && (row < n);
   this->ui->deleteButton->setEnabled (weHaveASelectedRow);
   this->ui->argumentEdit->setEnabled (weHaveASelectedRow);
   if (weHaveASelectedRow) {
      this->ui->argumentEdit->setFocus();
   }
}

//------------------------------------------------------------------------------
//
void QEMenuButtonArgumentsDialog::loadArgumentList ()
{
   this->ui->argumentEdit->setText ("");
   this->listWidget->setCurrentRow (noSelection);

   this->listWidget->clear();
   const int n = this->argumentList.count();
   for (int a = 0; a < n; a++) {
      QString arg = this->argumentList.value (a);

      QListWidgetItem* item;
      item = this->listWidget->item(a);
      if (!item) {
         item = new QListWidgetItem ();
         this->listWidget->insertItem(a, item);
      }
      item->setText (arg);
   }
}

//------------------------------------------------------------------------------
// slot
void QEMenuButtonArgumentsDialog::rowSelectionChanged()
{
   const int row = this->listWidget->currentRow();
   const int n = this->argumentList.count();
   if ((row < 0) || (row >= n)) return;   // sanity check

   QString arg = this->argumentList.value (row);
   this->ui->argumentEdit->setText (arg);
   this->widgetsEnable ();
}

//------------------------------------------------------------------------------
// slot
void QEMenuButtonArgumentsDialog::scrollValueChanged (int)
{
   this->widgetsEnable ();
}

//------------------------------------------------------------------------------
// slot
void QEMenuButtonArgumentsDialog::textEdited (const QString& text)
{
   const int row = this->listWidget->currentRow();
   const int n = this->argumentList.count();
   if ((row < 0) || (row >= n)) return;
   this->argumentList.replace (row, text);

   QListWidgetItem* item;
   item = this->listWidget->item (row);
   if (!item) {
      item = new QListWidgetItem ();
      this->listWidget->insertItem (row, item);
   }
   item->setText (text);
   this->widgetsEnable();
}

//------------------------------------------------------------------------------
// slot
void QEMenuButtonArgumentsDialog::onNewItemClick (bool)
{
   const int row = this->listWidget->currentRow();
   const int n = this->argumentList.count();
   int selectThis;

   if ((row >= 0) && (row < n)) {
      // Insert after currently selected row
      //
      this->argumentList.insert (row + 1, "");
      selectThis = row + 1;
   } else {
      this->argumentList.append ("");
      selectThis = n;
   }
   this->loadArgumentList ();
   this->listWidget->setCurrentRow (selectThis);

   this->widgetsEnable();
}

//------------------------------------------------------------------------------
// slot
void QEMenuButtonArgumentsDialog::onDeleteItemClick (bool)
{
   const int row = this->listWidget->currentRow();
   const int n = this->argumentList.count();
   if ((row < 0) || (row >= n)) return;

   int selectThis = MIN (row, n - 2);  // keep same or last row selected

   this->argumentList.removeAt (row);
   this->loadArgumentList ();
   this->listWidget->setCurrentRow (selectThis);
   this->widgetsEnable();
}

//------------------------------------------------------------------------------
// slot
void QEMenuButtonArgumentsDialog::onScrollUpClick (bool)
{
   QScrollBar* scrollBar = this->listWidget->verticalScrollBar();
   const int step = scrollBar->singleStep();
   const int oldPosn = scrollBar->sliderPosition();
   const int newPosn = MAX (0, oldPosn - step);
   scrollBar->setSliderPosition (newPosn);
   this->widgetsEnable();
}

//------------------------------------------------------------------------------
// slot
void QEMenuButtonArgumentsDialog::onScrollDownClick (bool)
{
   QScrollBar* scrollBar = this->listWidget->verticalScrollBar();
   const int step = scrollBar->singleStep();
   const int oldPosn = scrollBar->sliderPosition();
   const int maxPosn = scrollBar->maximum();
   const int newPosn = MIN (maxPosn, oldPosn + step);
   scrollBar->setSliderPosition (newPosn);
   this->widgetsEnable();
}

//------------------------------------------------------------------------------
// slot
void QEMenuButtonArgumentsDialog::on_buttonBox_rejected ()
{
   this->close ();
}

//------------------------------------------------------------------------------
// slot
void QEMenuButtonArgumentsDialog::on_buttonBox_accepted ()
{
   QDialog::accept();  // Always all okay
}

// end
