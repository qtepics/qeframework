/*  QEDistribution.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2019 Australian Synchrotron.
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
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrews@ansto.gov.au
 */

#ifndef QE_DISTRIBUTION_H
#define QE_DISTRIBUTION_H

#include <QObject>
#include <QAction>
#include <QCheckBox>
#include <QLabel>
#include <QMenu>
#include <QTimer>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QList>

#include <QEArchiveAccess.h>
#include <QECommon.h>
#include <QEAbstractDynamicWidget.h>
#include <QEFloatingFormatting.h>
#include <QEStringFormatting.h>
#include <QEAxisPainter.h>
#include <QEResizeableFrame.h>
#include <QESingleVariableMethods.h>
#include <QEPVNameSelectDialog.h>
#include <QEFrameworkLibraryGlobal.h>

/// \brief The QEDistribution class provides a histogram of the distribution of
/// the values of a scaler PV.
///
/// To a certain extent, this widget mimics the hehaviour of the histogram record
/// in that it bins values.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEDistribution :
      public QEAbstractDynamicWidget,
      public QESingleVariableMethods
{
   Q_OBJECT

   // BEGIN-SINGLE-VARIABLE-V2-PROPERTIES ===============================================
   // Single Variable properties
   // These properties should be identical for every widget using a single variable.
   // WHEN MAKING CHANGES: Use the update_widget_properties script in the resources
   // directory.
   //
   // Note, a property macro in the form 'Q_PROPERTY(QString variableName READ ...' doesn't work.
   // A property name ending with 'Name' results in some sort of string a variable being displayed,
   // but will only accept alphanumeric and won't generate callbacks on change.
public:
   /// EPICS variable name (CA PV)
   ///
   Q_PROPERTY (QString variable READ getVariableNameProperty WRITE setVariableNameProperty)

   /// Macro substitutions. The default is no substitutions. The format is NAME1=VALUE1[,] NAME2=VALUE2...
   /// Values may be quoted strings. For example, 'PUMP=PMP3, NAME = "My Pump"'
   /// These substitutions are applied to variable names for all QE widgets.
   /// In some widgets are are also used for other purposes.
   ///
   Q_PROPERTY (QString variableSubstitutions READ getVariableNameSubstitutionsProperty WRITE setVariableNameSubstitutionsProperty)

   /// The number of elements required to be subscribed for from the PV host (IOC).
   /// The default is 0 which means subscribed for all elements.
   /// Note: changing this value causes the unsubscribe/re-subscribe just as if the
   /// variable name changed.
   ///
   Q_PROPERTY (int elementsRequired READ getElementsRequired WRITE setElementsRequired)

   /// Index used to select a single item of data for processing. The default is 0.
   ///
   Q_PROPERTY (int arrayIndex READ getArrayIndex WRITE setArrayIndex)
   //
   // END-SINGLE-VARIABLE-V2-PROPERTIES =================================================

   // QEDistribution specific prperties
   //
   /// Number of standard deviations to plot: range 0.1 to 9.0, default 3.0
   ///
   Q_PROPERTY (double numberStdDevs    READ getNumberStdDevs    WRITE setNumberStdDevs)

   /// Selects plot mode - rectangular histogram (default) or smooth
   ///
   Q_PROPERTY (bool   isRectangular    READ getIsRectangular    WRITE setIsRectangular)

   /// histogram edge width : range 0 to 20, default 2
   Q_PROPERTY (int    edgeWidth        READ getEdgeWidth        WRITE setEdgeWidth)

   /// Selects display of gaussian curve - default yes.
   ///
   Q_PROPERTY (bool   showGaussian     READ getShowGaussian     WRITE setShowGaussian)

   /// gaussian plot width : range 1 to 20, default 2
   Q_PROPERTY (int    gaussianWidth    READ getGaussianWidth    WRITE setGaussianWidth)

   /// default: light blue
   Q_PROPERTY (QColor fillColour       READ getFillColour       WRITE setFillColour)

   /// default: dark blue
   Q_PROPERTY (QColor edgeColour       READ getEdgeColour       WRITE setEdgeColour)

   /// default: red
   Q_PROPERTY (QColor gaussianColour   READ getGaussianColour   WRITE setGaussianColour)

   /// default : light grey
   Q_PROPERTY (QColor backgroundColour READ getBackgroundColour WRITE setBackgroundColour)

public:
   /// Create without a variable.
   /// Use setVariableNameProperty() and setSubstitutionsProperty() to define a
   /// variable and, optionally, macro substitutions later.
   ///
   explicit QEDistribution (QWidget* parent = 0);

   /// Create with a variable.
   /// A connection is automatically established.
   /// If macro substitutions are required, create without a variable and set
   /// the variable and macro substitutions after creation.
   ///
   explicit QEDistribution (const QString& variableName, QWidget* parent = 0);

   /// Destruction
   ///
   ~QEDistribution ();

   // Property accesss functions.
   //
   double getNumberStdDevs () const;
   bool getIsRectangular () const;
   int getEdgeWidth () const;
   QColor getBackgroundColour () const;
   QColor getEdgeColour () const;
   QColor getFillColour () const;
   QColor getGaussianColour () const;
   bool getShowGaussian () const;
   int getGaussianWidth () const;

public slots:
   // Property set functions are also slots.
   //
   void setNumberStdDevs (const double numberStdDevs);
   void setIsRectangular (const bool isRectangular);
   void setEdgeWidth (const int edgeWidth);
   void setBackgroundColour (const QColor colour);
   void setEdgeColour (const QColor colour);
   void setFillColour (const QColor colour);
   void setGaussianColour (const QColor colour);
   void setShowGaussian (const bool showGaussian);
   void setGaussianWidth (const int gaussainWidth);

signals:
   // This signal is emitted using the QEEmitter::emitDbConnectionChanged function.
   /// Sent when the widget state updated following a channel connection change
   void dbConnectionChanged (const bool& isConnected);

   // Note, the following signals are common to many QE widgets,
   // if changing the doxygen comments, ensure relevent changes are migrated to all instances
   // These signals are emitted using the QEEmitter::emitDbValueChanged function.
   /// Sent when the widget is updated following a data change
   /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
   /// For example a QList widget could log updates from this widget.
   ///
   void dbValueChanged ();                     // signal event
   void dbValueChanged (const QString& out);   // signal as formatted text
   void dbValueChanged (const int& out);       // signal as int if applicable
   void dbValueChanged (const long& out);      // signal as long if applicable
   void dbValueChanged (const qlonglong& out); // signal as qlonglong if applicable
   void dbValueChanged (const double& out);    // signal as floating if applicable
   void dbValueChanged (const bool& out);      // signal as bool: value != 0 if applicable

public:
   // Override QEAbstractDynamicWidget functions - required.
   //
   int addPvName (const QString& pvName);
   void clearAllPvNames ();

protected:
    bool eventFilter (QObject* watched, QEvent* event);

    qcaobject::QCaObject* createQcaItem (unsigned int variableIndex);
    void establishConnection (unsigned int variableIndex);

    // Drag and Drop
    void dragEnterEvent (QDragEnterEvent *event) { this->qcaDragEnterEvent (event); }
    void dropEvent (QDropEvent *event)           { this->qcaDropEvent (event); }
    void mousePressEvent (QMouseEvent *event)    { this->qcaMousePressEvent (event); }

    // Use default parent setDrop and getDrop which are past and copy actions.

    // Copy paste
    QString copyVariable();
    QVariant copyData();
    void paste (QVariant v);

    // override other virtual functions
    //
    void saveConfiguration (PersistanceManager* pm);
    void restoreConfiguration (PersistanceManager* pm, restorePhases restorePhase);

    void setReadOut (const QString& text);

private:
   void setup();                 // common widget setup function
   void resizeDistribution ();
   void paintDistribution ();
   void mouseMoveDistribution (const QPoint& mousePosition);
   void createWidgets();         // creates internal widgets
   void setPvName (const QString& pvName);   // for paste, drag/drop
   void connectSignalsToSlots ();
   void resetDistibution ();
   bool updatePlotLimits ();
   void updateDistribution ();

   // utility functions
   double calcPlotDelta () const;
   static bool changed (const double a, const double b, const double e);

   static QTimer* tickTimer;
   QList<double> historicalData;
   int distributionData [128];
   int distributionCount;

   // X axis plot values
   //
   double currentXPlotMin;
   double currentXPlotMax;
   double currentXPlotMinor;
   int xChangePending;

   // Y axis plot values
   //
   double currentYPlotMin;   // always 0
   double currentYPlotMax;
   double currentYPlotMinor;
   int yChangePending;

   // property members
   //
   QColor mBackgroundColour;
   QColor mEdgeColour;
   QColor mGaussianColour;
   QColor mFillColour;
   double mNumberStdDevs;
   int mEdgeWidth;
   int mGaussianWidth;
   bool mIsRectangular;
   bool mShowGaussian;

   QEFloatingFormatting floatingFormatting;
   QEStringFormatting stringFormatting;
   int tickTimerCount;
   bool replotIsRequired;

   double valueCount;
   double valueSum;
   double valueMean;
   double valueMin;
   double valueMax;
   double valueSquaredSum;
   double valueStdDev;
   bool isFirstUpdate;

   QEArchiveAccess* archiveAccess;
   QEPVNameSelectDialog* pvNameSelectDialog;

   enum ContextMenuActions { maVoid = 0, maAddEdit, maPaste, maClear };
   QMenu* contextMenu;
   QAction* pasteAction;
   QAction* clearAction;

   QVBoxLayout* verticalLayout;
   QEResizeableFrame* resize_frame;
   QPushButton* pushButton;
   QLabel* valueLabel;
   QLabel* pvNameLabel;
   QLabel* label_1;
   QLabel* label_2;
   QLabel* label_4;
   QLabel* label_5;
   QLabel* label_6;
   QLabel* countValueLabel;
   QLabel* minValueLabel;
   QLabel* meanValueLabel;
   QLabel* maxValueLabel;
   QLabel* stdDevLabel;
   QPushButton* resetButton;
   QCheckBox* gaussianBox;

   QWidget* plotArea;
   QEAxisPainter* xAxis;
   QEAxisPainter* yAxis;

private slots:
   void newPvName (QString variableNameIn,
                   QString variableNameSubstitutionsIn,
                   unsigned int variableIndex);

   void connectionChanged (QCaConnectionInfo& connectionInfo,
                           const unsigned int& variableIndex);

   void setPvValue (const double& value, QCaAlarmInfo&,
                    QCaDateTime&, const unsigned int&);

   void runPVSelectDialog (bool);
   void resetButtonClicked (bool);
   void contextMenuRequested (const QPoint& pos);
   void contextMenuSelected (QAction* selectedItem);
   void tickTimeout ();
};

#ifdef EXP_DECLARE_METATYPE_IS_REQUIRED
// place holder
#endif

#endif   // QE_DISTRIBUTION_H
