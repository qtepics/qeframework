/*  QEFileBrowser.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2025 Australian Synchrotron
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
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
 *    Ricardo Fernandes
 *  Contact details:
 *    ricardo.fernandes@synchrotron.org.au
 */

#ifndef QE_FILE_BROWSER_H
#define QE_FILE_BROWSER_H

#include <QTableWidget>
#include <QPushButton>
#include <QEAbstractWidget.h>
#include <QEEnums.h>
#include <QEWidget.h>
#include <QELineEdit.h>

// =============================================================================
//  QEFILEBROWSER METHODS
// =============================================================================
/// The QEFileBrowser widget allows the user to browse existing files from a certain directory.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEFileBrowser:
      public QEAbstractWidget
{
   Q_OBJECT

   // QEFileBrowser specific properties
   //
   /// EPICS variable name (CA PV). This variable is used for both writing and
   /// reading the directory to be used by the widget.
   /// An internal QELineEdit widget is used to mange this PV.
   ///
   Q_PROPERTY(QString variable READ getVariableName WRITE setVariableName)

   /// Macro substitutions. The default is no substitutions.
   /// The format is NAME1=VALUE1[,] NAME2=VALUE2...
   /// Values may be quoted strings. For example, 'PUMP=PMP3, NAME = "My Pump"'
   /// These substitutions are applied to variable names for all QE widgets.
   /// In some widgets are are also used for other purposes.
   ///
   Q_PROPERTY(QString variableSubstitutions READ getVariableNameSubstitutions WRITE setVariableNameSubstitutions)

   /// Default directory where to browse files when QEFileBrowser is
   /// launched for the first time.
   ///
   Q_PROPERTY(QString directoryPath READ getDirectoryPath WRITE setDirectoryPath)

   /// Show/hide directory path line edit where the user can specify the
   /// directory to browse files.
   ///
   Q_PROPERTY(bool showDirectoryPath READ getShowDirectoryPath WRITE setShowDirectoryPath)

   /// Show/hide button to open the dialog window to browse for directories and files.
   ///
   Q_PROPERTY(bool showDirectoryBrowser READ getShowDirectoryBrowser WRITE setShowDirectoryBrowser)

   /// Show/hide button to refresh the table containing the list of files
   /// being browsed.
   ///
   Q_PROPERTY(bool showRefresh READ getShowRefresh WRITE setShowRefresh)

   /// Show/hide table containing the list of files being browsed.
   ///
   Q_PROPERTY(bool showTable READ getShowTable WRITE setShowTable)

   /// Show/hide column containing the time of creation of files
   Q_PROPERTY(bool showColumnTime READ getShowColumnTime WRITE setShowColumnTime)

   /// Show/hide column containing the size (in bytes) of files.
   ///
   Q_PROPERTY(bool showColumnSize READ getShowColumnSize WRITE setShowColumnSize)

   /// Show/hide column containing the name of files.
   ///
   Q_PROPERTY(bool showColumnFilename READ getShowColumnFilename WRITE setShowColumnFilename)

   /// Show/hide the extension of files.
   ///
   Q_PROPERTY(bool showFileExtension READ getShowFileExtension WRITE setShowFileExtension)

   /// Enable/disable the browsing of directories-only when opening the
   /// dialog window.
   ///
   Q_PROPERTY(bool fileDialogDirectoriesOnly READ getFileDialogDirectoriesOnly WRITE setFileDialogDirectoriesOnly)

   /// Specify which files to browse. To specify more than one filter,
   /// please separate them with a ";".
   /// Example: *.py;*.ui (this will only display files with an extension .py or .ui).
   ///
   Q_PROPERTY(QString fileFilter READ getFileFilter WRITE setFileFilter)

   /// Change the order of the widgets. Valid options are: Top, Bottom, Left and Right.
   /// The default is Top.
   ///
   Q_PROPERTY(QE::LayoutOptions optionsLayout READ getOptionsLayout WRITE setOptionsLayout)

   /// Margin between contebys and bounding container widgehet.
   ///
   Q_PROPERTY (int margin READ getMargin WRITE setMargin)

public:
   explicit QEFileBrowser(QWidget* parent = 0);
   virtual ~QEFileBrowser(){}

   void setVariableName(const QString& pvName);
   QString getVariableName() const;

   void setVariableNameSubstitutions(const QString& substitutions);
   QString getVariableNameSubstitutions() const;

   void setDirectoryPath(const QString& directoryPath);
   QString getDirectoryPath() const;

   void setShowDirectoryPath(bool showPath);
   bool getShowDirectoryPath() const;

   void setShowDirectoryBrowser(bool showBrowser);
   bool getShowDirectoryBrowser() const;

   void setShowRefresh(bool showRefresh);
   bool getShowRefresh() const;

   void setShowTable(bool showTable);
   bool getShowTable() const;

   void setShowColumnTime(bool showTime);
   bool getShowColumnTime() const;

   void setShowColumnSize(bool showSize);
   bool getShowColumnSize() const;

   void setShowColumnFilename(bool showFilename);
   bool getShowColumnFilename() const;

   void setShowFileExtension(bool showExtension);
   bool getShowFileExtension() const;

   void setFileFilter (const QString& fileFilter);
   QString getFileFilter() const;

   void setFileDialogDirectoriesOnly(bool directoriesOnly);
   bool getFileDialogDirectoriesOnly() const;

   void setOptionsLayout (QE::LayoutOptions layout);
   QE::LayoutOptions getOptionsLayout() const;

   void setMargin(const int margin);
   int getMargin() const;

   void updateTable();

signals:
   /// Signal that is generated every time the user double-clicks a certain file.
   /// This signals emits a string that contains the full path and the name of the selected file.
   /// This signal may be captured by other widgets that perform further operations (for instance,
   /// the QEImage displays the content of this file if it is a graphical one).
   //
   void selected(QString pFilename);

protected:
   class QEOwnTable;  // internal and private widget

   QELineEdit* directoryPathEdit;
   QPushButton* directoryBrowserButton;
   QPushButton* refreshButton;
   QEOwnTable* browserTable;
   QString m_fileFilter;
   bool m_showFileExtension;
   bool m_fileDialogDirectoriesOnly;
   int m_margin;
   QE::LayoutOptions m_optionsLayout;

   // Handle widget events.
   //
   bool event(QEvent* event);

private:

private slots:
   void lineEditDirectoryPathChanged (QString);
   void buttonDirectoryBrowserClicked();
   void buttonRefreshClicked();
   void itemActivated(QTableWidgetItem *);
};

#endif // QE_FILE_BROWSER_H
