/*  QEFileBrowser.h
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

   /// Default macro substitutions. The default is no substitutions.
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
   /// dialog window. The default is true.
   ///
   Q_PROPERTY(bool fileDialogDirectoriesOnly READ getFileDialogDirectoriesOnly WRITE setFileDialogDirectoriesOnly)

   /// Specifies if the selected can be created.
   /// When false, this is the equivilent to open and is the default.
   /// When true, this is the equivilent to save.
   /// Only applicable when fileDialogDirectoriesOnly is set true.
   ///
   Q_PROPERTY(bool fileCreationAllowed READ getFileCreationAllowed WRITE setFileCreationAllowed )

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

   void setFileDialogDirectoriesOnly (const bool directoriesOnly);
   bool getFileDialogDirectoriesOnly () const;

   void setFileCreationAllowed (const bool fileCreationAllowed);
   bool getFileCreationAllowed () const;

   void setOptionsLayout (const QE::LayoutOptions layout);
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
   bool m_fileCreationAllowed;
   int m_margin;
   QE::LayoutOptions m_optionsLayout;

   // Handle widget events.
   //
   bool event(QEvent* event);

private:

private slots:
   void onDirectoryPathChanged (const QString&);
   void onDirectoryBrowseClicked ();
   void onRefreshClicked ();
   void itemActivated (QTableWidgetItem*);
};

#endif // QE_FILE_BROWSER_H
