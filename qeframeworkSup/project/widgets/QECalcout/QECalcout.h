/*  QECalcout.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2018-2024 Australian Synchrotron
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
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_CALCOUT_H
#define QE_CALCOUT_H

#include <QLabel>
#include <QEWidget.h>
#include <QString>
#include <QVariant>
#include <QEFrameworkLibraryGlobal.h>

/// This widget provides a calcout-like widget. So much so that where applicable
/// the property names have been choosen to match the calcout record.
/// This widget can be used instead of and/or to complement QELink.
///
/// This widget has no PV variables of its own. The inputs, A to L, must be provided
/// either by signals from other (QE) widgets or preset as 'constants' using the
/// a to l properties.
///
/// Under the covers, this widget uses the same calculation engine as the
/// calc/calcout record.
///
/// Traditionally, the type of GUI functionality the QECalcout widget supports
/// has been effected by using EPICS database variables (often CALC records) to
/// determine the state of GUI items. Where the variable is primarily a part of
/// the control system this is appropriate. Where the variable is only present
/// to support the GUI, then this functionality should be embedded in the GUI.
///
/// Conversely, while it is tempting to use this widget to quickly and easily do
/// GUI-side calculations, you should ask your self whether this should really be
/// done in an IOC. It can then be archived, alarmed, plotted, and be available
/// to any other EPICS client.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QECalcout : public QLabel, public QEWidget
{
   Q_OBJECT

public:
   // Apart from spaces mapping to underscores - these enumeration types match
   // the calcout record.
   //
   enum DataOptions {
      Use_CALC,
      Use_OCAL
   };
   Q_ENUM (DataOptions)

   enum OutputOptions {
      Every_Time,
      On_Change,
      When_Zero,
      When_Non_zero,
      Transition_To_Zero,
      Transition_To_Non_zero,
      Never   // not part of calcout, but is in swait and a useful addition.
   };
   Q_ENUM (OutputOptions)

   /// Specifies the calculation to be performed. The syntax is identical to that
   /// used by the calc/calcout records. However it is not limited to 80 characters.
   /// The default is "0"
   ///
   Q_PROPERTY (QString calc       READ getCalc       WRITE setCalc)

   /// Shows the state of the calculation expression.
   ///
   Q_PROPERTY (QString calcStatus READ getCalcStatus WRITE setCalcStatus)

   /// Specifies the ouput calc to be performed. The syntax is identical to that
   /// used by the calc/calcout records. However it is not limited to 80 characters.
   /// The default is "0"
   ///
   Q_PROPERTY (QString ocal       READ getOcal       WRITE setOcal)

   /// Shows the state of the output calculation expression.
   ///
   Q_PROPERTY (QString ocalStatus READ getOcalStatus WRITE setOcalStatus)

   Q_PROPERTY (DataOptions dopt   READ getDopt       WRITE setDopt)
   Q_PROPERTY (OutputOptions oopt READ getOopt       WRITE setOopt)

   /// Controls the string format for both the out (QString) signal
   /// and the widhets own text.
   ///
   Q_PROPERTY (QString egu        READ getEgu        WRITE setEgu)
   Q_PROPERTY (int prec           READ getPrec       WRITE setPrec)
   Q_PROPERTY (QChar format       READ getFormat     WRITE setFormat)

   /// Sets the 'constant' input values.
   ///
   Q_PROPERTY (double  a          READ geta          WRITE inpa)
   Q_PROPERTY (double  b          READ getb          WRITE inpb)
   Q_PROPERTY (double  c          READ getc          WRITE inpc)
   Q_PROPERTY (double  d          READ getd          WRITE inpd)
   Q_PROPERTY (double  e          READ gete          WRITE inpe)
   Q_PROPERTY (double  f          READ getf          WRITE inpf)
   Q_PROPERTY (double  g          READ getg          WRITE inpg)
   Q_PROPERTY (double  h          READ geth          WRITE inph)
   Q_PROPERTY (double  i          READ geti          WRITE inpi)
   Q_PROPERTY (double  j          READ getj          WRITE inpj)
   Q_PROPERTY (double  k          READ getk          WRITE inpk)
   Q_PROPERTY (double  l          READ getl          WRITE inpl)

   /// Allows the widget to be visible ar run time. Default is false.
   /// Note: always visible in designer.
   ///
   Q_PROPERTY (bool runVisible    READ getRunVisible WRITE setRunVisible)

public:
   explicit QECalcout (QWidget *parent = 0);
   ~QECalcout ();

   // Property functions
   //
   // The calc expression and error if any.
   //
   void setCalc (const QString& calc);
   QString getCalc () const;

   // The calc expression status - essentially read only.
   //
   void setCalcStatus (const QString&);
   QString getCalcStatus () const;

   void setOcal (const QString& ocal);
   QString getOcal () const;

   // The calc expression status - essentially read only.
   //
   void setOcalStatus (const QString&);
   QString getOcalStatus () const;

   void setDopt (const DataOptions dopt);
   DataOptions getDopt () const;

   void setOopt (const OutputOptions oopt);
   OutputOptions getOopt () const;

   void setEgu (const QString& egu);
   QString getEgu () const;

   void setPrec (const int prec);
   int getPrec () const;

   void setFormat (const QChar format);
   QChar getFormat () const;

public slots:
   void inpa (const double a);
   void inpb (const double b);
   void inpc (const double c);
   void inpd (const double d);
   void inpe (const double e);
   void inpf (const double f);
   void inpg (const double g);
   void inph (const double h);
   void inpi (const double i);
   void inpj (const double j);
   void inpk (const double k);
   void inpl (const double l);

   // Replicated for integer inputs.
   //
   void inpa (const int a);
   void inpb (const int b);
   void inpc (const int c);
   void inpd (const int d);
   void inpe (const int e);
   void inpf (const int f);
   void inpg (const int g);
   void inph (const int h);
   void inpi (const int i);
   void inpj (const int j);
   void inpk (const int k);
   void inpl (const int l);

   // Replicated for boolean inputs.
   //
   void inpa (const bool a);
   void inpb (const bool b);
   void inpc (const bool c);
   void inpd (const bool d);
   void inpe (const bool e);
   void inpf (const bool f);
   void inpg (const bool g);
   void inph (const bool h);
   void inpi (const bool i);
   void inpj (const bool j);
   void inpk (const bool k);
   void inpl (const bool l);

public:
   double geta () const;
   double getb () const;
   double getc () const;
   double getd () const;
   double gete () const;
   double getf () const;
   double getg () const;
   double geth () const;
   double geti () const;
   double getj () const;
   double getk () const;
   double getl () const;

   // This will cause the expressions to be re-evaluated and the emission
   // of the output signals.
   //
   void process ();

signals:
   // The output value in various formats.
   //
   void out (const bool& out);
   void out (const int& out);
   void out (const long& out);
   void out (const qlonglong& out);
   void out (const double& out);
   void out (const QString& out);

private:
   // Utility functions.
   //
   void emitValue (const QVariant& value);
   void processInner ();

   QString calc;
   QString ocal;
   DataOptions dopt;
   OutputOptions oopt;
   QString egu;
   int prec;
   QChar format;

   QString calcExpressionStatus;
   bool calcExpressionIsValid;

   QString ocalExpressionStatus;
   bool ocalExpressionIsValid;

   bool isProcessing;
   double lastCalcValue;

   // This is the value from the INFIX_TO_POSTFIX_SIZE macro from postfix.h
   // plus a 20 just in case.
   //
   static const int MAX_POSTFIX_SIZE = 524 + 20;
   char calcPostFix [MAX_POSTFIX_SIZE];
   char ocalPostFix [MAX_POSTFIX_SIZE];
   double args [12];  // CALCPERFORM_NARGS
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QECalcout::DataOptions)
Q_DECLARE_METATYPE (QECalcout::OutputOptions)
#endif

#endif   // QE_CALCOUT_H
