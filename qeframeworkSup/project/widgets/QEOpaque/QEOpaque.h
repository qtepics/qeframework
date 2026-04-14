/*  QEOpaque.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_OPAQUE_H
#define QE_OPAQUE_H

#include <QLabel>
#include <QMenu>
#include <QPlainTextEdit>
#include <QString>
#include <QStringList>
#include <QSize>
#include <QVBoxLayout>

#include <QECommon.h>
#include <QEAbstractWidget.h>
#include <QEChannel.h>
#include <QEWidget.h>
#include <QCaVariableNamePropertyManager.h>
#include <QESingleVariableMethods.h>
#include <QEFrameworkLibraryGlobal.h>

/*!
   This class provides an EPICS aware table widget which is capable of displaying
   a PV Access Normative Type Table in tabular form. When in the default vertical
   orientation each column displays a consecutive element from an array EPICS variable.

   When in horizonal mode, the table and functionality is transposed.

   It is tighly integrated, via the QEAbstractWidget, with the base class QEWidget
   class which provides generic support such as macro substitutions, drag/drop,
   and standard properties. QEAbstractWidget provides all standard properties.
 */

class QEOpaqueData;  //  differed

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEOpaque :
      public QEAbstractWidget,
      public QESingleVariableMethods
{
   Q_OBJECT

   // Note, a property macro in the form 'Q_PROPERTY(QString variableName READ ...' doesn't work.
   // A property name ending with 'Name' results in some sort of string a variable being displayed,
   // but will only accept alphanumeric and won't generate callbacks on change.
public:
   /// EPICS variable name (PVA PV)
   ///
   Q_PROPERTY (QString variable READ getVariableNameProperty WRITE setVariableNameProperty)

   /// Macro substitutions. The default is no substitutions. The format is NAME1=VALUE1[,] NAME2=VALUE2...
   /// Values may be quoted strings. For example, 'PUMP=PMP3, NAME = "My Pump"'
   /// These substitutions are applied to variable names for all QE widgets.
   /// In some widgets are are also used for other purposes.
   ///
   Q_PROPERTY (QString variableSubstitutions READ getVariableNameSubstitutionsProperty WRITE setVariableNameSubstitutionsProperty)

   /// Controls if PV name displayed. Default is false.
   ///
   Q_PROPERTY (bool showPvName        READ getShowPvName       WRITE setShowPvName)

   //
   // End of QEOpaque specific properties =========================================

public:
   /// Create without a variable.
   /// Use setVariableName functions.
   //
   explicit QEOpaque (QWidget* parent = 0);

   /// Create with a variable.
   //
   explicit QEOpaque (const QString& variableName, QWidget* parent = 0);

   /// Destruction
   virtual ~QEOpaque ();

   void setShowPvName (const bool showName);
   bool getShowPvName () const;

signals:
   // Note, the following signals are common to many QE widgets,
   // if changing the doxygen comments, ensure relevent changes are migrated to all instances
   /// Sent when the widget is updated following a data change
   /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
   /// For example a QList widget could log updates from this widget.
   /// Note: this widget emits the numeric enumeration value as opposed to the associated text.
   ///
   void dbValueChanged (const QEOpaqueData& out);
   void dbValueChanged (const QVariant& out);

protected:
   QSize sizeHint () const;
   bool eventFilter (QObject* watched, QEvent* event);

   // Override QEWidget functions.
   //
   void establishConnection (unsigned int variableIndex);
   QEChannel* createQcaItem (unsigned int variableIndex);

   // Drag and Drop
   //
   void mousePressEvent (QMouseEvent *event)    { qcaMousePressEvent (event); }
   void dragEnterEvent (QDragEnterEvent *event) { qcaDragEnterEvent (event);  }
   void dropEvent (QDropEvent *event)           { qcaDropEvent (event, true); }

   // This widget uses the setDrop/getDrop defined in QEWidget.

   // Copy paste
   //
   QString copyVariable ();
   QVariant copyData ();
   void paste (QVariant s);

private:
   void commonConstruct ();     //

   QEOpaqueData* opaqueData;    // contains the PV data
   QLabel* pvNameLabel;         // displays the PV name
   QPlainTextEdit* plainText;   // internal widget
   QVBoxLayout* layout;         // lays out name label and internal widget
   bool showPvName;

private slots:
   void usePvNameProperties (const QEPvNameProperties&);
   void connectionUpdated (const QEConnectionUpdate&);
   void valueUpdated (const QEVariantUpdate&);
};

#endif // QE_TABLE_H
