/*  QEFileBrowser.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2012-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Ricardo Fernandes
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEFileBrowser.h"
#include <QDebug>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QHeaderView>
#include <QECommon.h>
#include <QELineEdit.h>

#define DEBUG qDebug () << "QEFileBrowser" << __LINE__ << __FUNCTION__ << "  "


// =============================================================================
// QEOwnTable - internal class
// =============================================================================
//
class QEFileBrowser::QEOwnTable: public QTableWidget
{
public:
   explicit QEOwnTable (QWidget* parent = 0);
   ~QEOwnTable () { }

   void refreshSize();
   void resizeEvent(QResizeEvent*);
private:
   bool initialized;
};


// =============================================================================
// QEFileBrowser
// =============================================================================
//
QEFileBrowser::QEFileBrowser (QWidget* parent):
   QEAbstractWidget (parent)
{
   // Set default non-property values.
   //
   this->setVariableAsToolTip(false);
   this->setAllowDrop(false);
   this->setDisplayAlarmStateOption(QE::Never);

   // Create interbnal widgets
   //
   this->directoryPathEdit = new QELineEdit(this);
   this->directoryBrowserButton = new QPushButton(this);
   this->refreshButton = new QPushButton(this);
   this->browserTable = new QEFileBrowser::QEOwnTable (this);

   this->directoryPathEdit->setToolTip("Specify the directory where to browse for files");
   QObject::connect(this->directoryPathEdit, SIGNAL(textChanged(const QString&)),
                    this,           SLOT(onDirectoryPathChanged(const QString&)));

   this->directoryBrowserButton->setText("...");
   this->directoryBrowserButton->setToolTip("Browse for a directory/file");
   QObject::connect(this->directoryBrowserButton, SIGNAL(clicked()),
                    this, SLOT (onDirectoryBrowseClicked()));
   this->directoryBrowserButton->setFixedWidth (44);

   this->refreshButton->setText("Refresh");
   this->refreshButton->setToolTip("Refresh file browse result");
   QObject::connect(this->refreshButton, SIGNAL(clicked()),
                    this, SLOT(onRefreshClicked()));

   this->browserTable->setColumnCount(3);
   this->browserTable->setHorizontalHeaderItem(0, new QTableWidgetItem("Time"));
   this->browserTable->setHorizontalHeaderItem(1, new QTableWidgetItem("Size"));
   this->browserTable->setHorizontalHeaderItem(2, new QTableWidgetItem("Filename"));
   this->browserTable->setToolTip("Files contained in the specified directory");
   this->browserTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
   this->browserTable->setSelectionBehavior(QAbstractItemView::SelectRows);
   this->browserTable->setSelectionMode(QAbstractItemView::SingleSelection);
   this->browserTable->verticalHeader()->hide();
   QObject::connect(this->browserTable, SIGNAL(itemActivated(QTableWidgetItem*)),
                    this,               SLOT  (itemActivated(QTableWidgetItem*)));

   this->setShowFileExtension(true);
   this->setFileFilter("");
   this->setFileDialogDirectoriesOnly(true);
   this->setFileCreationAllowed(false);
   this->setOptionsLayout(QE::Top);
   this->setMargin(2);
}


//------------------------------------------------------------------------------
//
bool QEFileBrowser::event(QEvent* event)
{
   QPaintEvent* paintEvent;
   const QEvent::Type type = event->type ();
   bool result = false;

   switch (type) {
      case QEvent::FontChange:
         // Pass through font change to internal widgets
         this->directoryPathEdit->setFont (this->font ());
         this->directoryBrowserButton->setFont (this->font ());
         this->refreshButton->setFont (this->font ());
         this->browserTable->setFont (this->font ());
         result = true;
         break;

      case QEvent::Paint:
         // We need to handle the parents paint evenet.
         //
         paintEvent = reinterpret_cast<QPaintEvent*> (event);
         QEAbstractWidget::paintEvent (paintEvent);
         result = true;
         break;

      default:
         result = false;
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QEFileBrowser::setVariableName(const QString& pValue)
{
   this->directoryPathEdit->setVariableNameProperty(pValue);
}

//------------------------------------------------------------------------------
//
QString QEFileBrowser::getVariableName() const
{
   return this->directoryPathEdit->getVariableNameProperty();
}

//------------------------------------------------------------------------------
//
void QEFileBrowser::setVariableNameSubstitutions(const QString& pValue)
{
   this->directoryPathEdit->setVariableNameSubstitutionsProperty(pValue);
}

//------------------------------------------------------------------------------
//
QString QEFileBrowser::getVariableNameSubstitutions() const
{
   return this->directoryPathEdit->getVariableNameSubstitutionsProperty();
}

//------------------------------------------------------------------------------
//
void QEFileBrowser::setDirectoryPath(const QString& directoryPath)
{
   this->directoryPathEdit->setText (directoryPath);
}

//------------------------------------------------------------------------------
//
QString QEFileBrowser::getDirectoryPath() const
{
   return this->directoryPathEdit->text();
}

//------------------------------------------------------------------------------
//
void QEFileBrowser::setShowDirectoryPath(bool showPath)
{
   this->directoryPathEdit->setVisible(showPath);
}

//------------------------------------------------------------------------------
//
bool QEFileBrowser::getShowDirectoryPath() const
{
   return this->directoryPathEdit->isVisible();
}

//------------------------------------------------------------------------------
//
void QEFileBrowser::setShowDirectoryBrowser(bool showBrowser)
{
   this->directoryBrowserButton->setVisible(showBrowser);
}

//------------------------------------------------------------------------------
//
bool QEFileBrowser::getShowDirectoryBrowser() const
{
   return this->directoryBrowserButton->isVisible();
}

//------------------------------------------------------------------------------
//
void QEFileBrowser::setShowRefresh(bool showRefresh)
{
   this->refreshButton->setVisible(showRefresh);
}

//------------------------------------------------------------------------------
//
bool QEFileBrowser::getShowRefresh() const
{
   return this->refreshButton->isVisible();
}

//------------------------------------------------------------------------------
//
void QEFileBrowser::setFileFilter(const QString& fileFilter)
{
   this->m_fileFilter = fileFilter;
   this->updateTable();
}

//------------------------------------------------------------------------------
//
QString QEFileBrowser::getFileFilter() const
{
   return this->m_fileFilter;
}

//------------------------------------------------------------------------------
//
void QEFileBrowser::setShowTable(bool showTable)
{
   this->browserTable->setVisible(showTable);
}

//------------------------------------------------------------------------------
//
bool QEFileBrowser::getShowTable() const
{
   return this->browserTable->isVisible();
}

//------------------------------------------------------------------------------
//
void QEFileBrowser::setShowColumnTime(bool showTime)
{
   this->browserTable->setColumnHidden(0, showTime == false);
   this->browserTable->refreshSize();
}

//------------------------------------------------------------------------------
//
bool QEFileBrowser::getShowColumnTime() const
{
   return (this->browserTable->isColumnHidden(0) == false);
}

//------------------------------------------------------------------------------
//
void QEFileBrowser::setShowColumnSize(bool showSize)
{
   this->browserTable->setColumnHidden(1, showSize == false);
   this->browserTable->refreshSize();
}

//------------------------------------------------------------------------------
//
bool QEFileBrowser::getShowColumnSize() const
{
   return (this->browserTable->isColumnHidden(1) == false);
}

//------------------------------------------------------------------------------
//
void QEFileBrowser::setShowColumnFilename(bool showFilename)
{
   this->browserTable->setColumnHidden(2, showFilename == false);
   this->browserTable->refreshSize();
}

//------------------------------------------------------------------------------
//
bool QEFileBrowser::getShowColumnFilename() const
{
   return (this->browserTable->isColumnHidden(2) == false);
}

//------------------------------------------------------------------------------
//
void QEFileBrowser::setShowFileExtension(bool showExtension)
{
   this->m_showFileExtension = showExtension;
   this->updateTable();
}

//------------------------------------------------------------------------------
//
bool QEFileBrowser::getShowFileExtension() const
{
   return this->m_showFileExtension;
}

//------------------------------------------------------------------------------
//
void QEFileBrowser::setFileDialogDirectoriesOnly(const bool directoriesOnly)
{
   this->m_fileDialogDirectoriesOnly = directoriesOnly;
}

//------------------------------------------------------------------------------
//
bool QEFileBrowser::getFileDialogDirectoriesOnly() const
{
   return this->m_fileDialogDirectoriesOnly;
}

//------------------------------------------------------------------------------
//
void QEFileBrowser::setFileCreationAllowed (const bool fileCreationAllowed)
{
   this->m_fileCreationAllowed = fileCreationAllowed;
}

//------------------------------------------------------------------------------
//
bool QEFileBrowser::getFileCreationAllowed () const
{
   return this->m_fileCreationAllowed;
}

//------------------------------------------------------------------------------
//
void QEFileBrowser::setOptionsLayout (const QE::LayoutOptions layoutIn)
{
   const int m = this->getMargin();
   QLayout *qLayoutMain;
   QLayout *qLayoutChild;

   delete this->layout();

   switch (layoutIn) {
      case QE::Top:
         this->m_optionsLayout = QE::Top;
         qLayoutMain = new QVBoxLayout(this);
         qLayoutMain->setContentsMargins(m, m, m, m);
         qLayoutChild = new QHBoxLayout();
         qLayoutChild->setContentsMargins(0, 0, 0, 0);
         qLayoutChild->addWidget(directoryPathEdit);
         qLayoutChild->addWidget(directoryBrowserButton);
         qLayoutChild->addWidget(refreshButton);
         qLayoutMain->addItem(qLayoutChild);
         qLayoutMain->addWidget(browserTable);
         break;

      case QE::Bottom:
         this->m_optionsLayout = QE::Bottom;
         qLayoutMain = new QVBoxLayout(this);
         qLayoutMain->setContentsMargins(m, m, m, m);
         qLayoutMain->addWidget(browserTable);
         qLayoutChild = new QHBoxLayout();
         qLayoutChild->setContentsMargins(0, 0, 0, 0);
         qLayoutChild->addWidget(directoryPathEdit);
         qLayoutChild->addWidget(directoryBrowserButton);
         qLayoutChild->addWidget(refreshButton);
         qLayoutMain->addItem(qLayoutChild);
         break;

      case QE::Left:
         this->m_optionsLayout = QE::Left;
         qLayoutMain = new QHBoxLayout(this);
         qLayoutMain->setContentsMargins(m, m, m, m);
         qLayoutChild = new QVBoxLayout();
         qLayoutChild->setContentsMargins(0, 0, 0, 0);
         qLayoutChild->addWidget(directoryPathEdit);
         qLayoutChild->addWidget(directoryBrowserButton);
         qLayoutChild->addWidget(refreshButton);
         qLayoutMain->addItem(qLayoutChild);
         qLayoutMain->addWidget(browserTable);
         break;

      case QE::Right:
         this->m_optionsLayout = QE::Right;
         qLayoutMain = new QHBoxLayout(this);
         qLayoutMain->setContentsMargins(m, m, m, m);
         qLayoutChild = new QVBoxLayout();
         qLayoutChild->setContentsMargins(0, 0, 0, 0);
         qLayoutChild->addWidget(directoryPathEdit);
         qLayoutChild->addWidget(directoryBrowserButton);
         qLayoutChild->addWidget(refreshButton);
         qLayoutMain->addWidget(browserTable);
         qLayoutMain->addItem(qLayoutChild);
   }
}

//------------------------------------------------------------------------------
//
QE::LayoutOptions QEFileBrowser::getOptionsLayout() const
{
   return this->m_optionsLayout;
}

//------------------------------------------------------------------------------
//
void QEFileBrowser::setMargin(const int margin)
{
   this->m_margin = LIMIT (margin, 0, 100);
   // re-layout - the layout widget is ephemeral
   this->setOptionsLayout (this->m_optionsLayout);
   this->update();
}

//------------------------------------------------------------------------------
//
int QEFileBrowser::getMargin () const
{
   return this->m_margin;
}


//------------------------------------------------------------------------------
//
void QEFileBrowser::onDirectoryPathChanged(const QString&)
{
   this->updateTable();
}

//------------------------------------------------------------------------------
//
void QEFileBrowser::onDirectoryBrowseClicked()
{
   QString result;

   if (this->m_fileDialogDirectoriesOnly) {
      result = QFileDialog::getExistingDirectory (this, "Select directory",
                                                  this->directoryPathEdit->text(),
                                                  QFileDialog::ShowDirsOnly);
   } else {
      // this->m_fileFilter
      if (this->m_fileCreationAllowed) {
         result = QFileDialog::getSaveFileName (this, "Nominate file",
                                                this->directoryPathEdit->text(),
                                                this->m_fileFilter);
      } else {
         result = QFileDialog::getOpenFileName (this, "Select file",
                                                this->directoryPathEdit->text(),
                                                this->m_fileFilter);
      }
   }

   if (!result.isEmpty()) {
      this->directoryPathEdit->setText(result);
      this->directoryPathEdit->writeNow();
   }
}

//------------------------------------------------------------------------------
//
void QEFileBrowser::onRefreshClicked()
{
   this->updateTable();
}

//------------------------------------------------------------------------------
//
void QEFileBrowser::itemActivated(QTableWidgetItem *)
{
   QModelIndexList selectedRows;
   QString filename;
   QString data;

   selectedRows = this->browserTable->selectionModel()->selectedRows();
   data = this->browserTable->item(selectedRows.at(0).row(), 2)->text();

   if (this->directoryPathEdit->text().endsWith(QDir::separator()))
   {
      filename = this->directoryPathEdit->text() + data;
   }
   else
   {
      filename = this->directoryPathEdit->text() + QDir::separator() + data;
   }

   emit selected(filename);
}

//------------------------------------------------------------------------------
//
void QEFileBrowser::updateTable()
{
   QTableWidgetItem *qTableWidgetItem;
   QDir directory;
   QFileInfoList fileList;
   int i;
   int j;

   this->browserTable->setRowCount(0);
   directory.setPath(directoryPathEdit->text());
   directory.setFilter(QDir::Files);
   if (this->m_fileFilter.isEmpty() == false)
   {
      directory.setNameFilters(this->m_fileFilter.split(";"));
   }
   fileList = directory.entryInfoList();

   for(i = 0; i < fileList.size(); i++)
   {
      j = this->browserTable->rowCount();
      this->browserTable->insertRow(j);

      qTableWidgetItem = new QTableWidgetItem(fileList.at(i).lastModified().toString("yyyy/MM/dd - hh:mm:ss"));
      this->browserTable->setItem(j, 0, qTableWidgetItem);

      qTableWidgetItem = new QTableWidgetItem(QString::number(fileList.at(i).size()) + " bytes");
      this->browserTable->setItem(j, 1, qTableWidgetItem);

      if (this->m_showFileExtension)
      {
         qTableWidgetItem = new QTableWidgetItem(fileList.at(i).fileName());
      }
      else
      {
         qTableWidgetItem = new QTableWidgetItem(fileList.at(i).baseName());
      }
      this->browserTable->setItem(j, 2, qTableWidgetItem);
   }
}


// =============================================================================
// QEOwnTable methods
// =============================================================================
//
QEFileBrowser::QEOwnTable::QEOwnTable (QWidget* parent):
   QTableWidget(parent)
{
   this->initialized = false;
}

//------------------------------------------------------------------------------
//
void QEFileBrowser::QEOwnTable::refreshSize()
{
   int sizeColumn0;
   int sizeColumn1;
   int sizeColumn2;

   if (this->isColumnHidden(0)) {
      if (this->isColumnHidden(1)) {
         if (this->isColumnHidden(2)) {
            sizeColumn0 = 0;
            sizeColumn1 = 0;
            sizeColumn2 = 0;
         } else {
            sizeColumn0 = 0;
            sizeColumn1 = 0;
            sizeColumn2 = this->width();
         }
      } else {
         if (this->isColumnHidden(2)) {
            sizeColumn0 = 0;
            sizeColumn1 = this->width();
            sizeColumn2 = 0;
         } else {
            sizeColumn0 = 0;
            sizeColumn1 = 1 * this->width() / 5;
            sizeColumn2 = 4 * this->width() / 5 - 1;
         }
      }
   } else {
      if (this->isColumnHidden(1)) {
         if (this->isColumnHidden(2)) {
            sizeColumn0 = this->width();
            sizeColumn1 = 0;
            sizeColumn2 = 0;
         } else {
            sizeColumn0 = 1 * this->width() / 5;
            sizeColumn1 = 0;
            sizeColumn2 = 4 * this->width() / 5 - 1;
         }
      } else {
         if (this->isColumnHidden(2)) {
            sizeColumn0 = this->width() / 2;
            sizeColumn1 = this->width() / 2 - 1;
            sizeColumn2 = 0;
         } else {
            sizeColumn0 = 1 * this->width() / 5;
            sizeColumn1 = 1 * this->width() / 5;
            sizeColumn2 = 3 * this->width() / 5 - 1;
         }
      }
   }

   this->setColumnWidth(0, sizeColumn0);
   this->setColumnWidth(1, sizeColumn1);
   this->setColumnWidth(2, sizeColumn2);
}

//------------------------------------------------------------------------------
//
void QEFileBrowser::QEOwnTable::resizeEvent (QResizeEvent *)
{

   // TODO: this condition should always be execute when inside Qt Designer.
   //
   if (this->initialized == false) {
      this->refreshSize();
      this->initialized = true;
   }
}

// end
