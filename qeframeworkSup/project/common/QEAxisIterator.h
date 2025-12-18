/*  QEAxisIterator.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2015-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_AXIS_ITERATOR_H
#define QE_AXIS_ITERATOR_H

#include <QEFrameworkLibraryGlobal.h>

/// This class provides the means to iterate over a range of values such that
/// the returned values x, are minimum <= x <= maximum together an indication
/// of whether the interval is a minor ot major interval. It primarily intented
/// to support the painting of axies, but could be use for other purposes.
///
/// For a linear iteration, the set of values are always an integer multiple of
/// the given minorInterval. The major interval indication occures when the value
/// an interger multiple of the major interval (minorInterval * majorMinorRatio).
///
/// The class constructor will contstrain the minorInterval to be >= 1.0e-20 and
/// the majorMinorRatio to be >= 1 if needs be.
//
/// This is illustrates below - the minor values are indicated by a '+' character
/// and the major intervals by an '|' character. The minimum value is -0.14, the
/// maximum value is 1.0, the minorInterval is 0.1, and the majorMinorRatio is 4.
///
///  min                                                                 max
///   v                                                                   v
///   --+-----|-----+-----+-----+-----|-----+-----+-----+-----|-----+-----+
///   -0.1   0.0   0.1   0.2   0.3   0.4   0.5   0.6   0.7   0.8   0.9   1.0
///
///
/// For a logarithmic iteration, the set of values are of the form:
///
///  1.0eN, 2.0eN, 3.0eN, 4.0eN, 5.0eN, 6.0eN, 7.0eN, 8.0eN, 9.0eN, 1.0e(N+1)
///
/// Major values are of the form 1.0eM where M is a multiple of majorMinorRatio.
/// The minorInterval parameter is not applicable when the iteration is logarithmic.
//
// This is used by and has been refactoed out of QEAxisPainter.h.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEAxisIterator {
public:
   // Iterator constructor.
   explicit QEAxisIterator (const double minimum,
                            const double maximum,
                            const double minorInterval,
                            const int majorMinorRatio,
                            const bool isLogarithmic);
   virtual ~QEAxisIterator ();

   /// Re-initialise iterator parameters. If the iterator parameters are updated,
   /// the next call to  nextValue () will always return false.
   //
   void reInitialise (const double minimum,
                      const double maximum,
                      const double minorInterval,
                      const int majorMinorRatio,
                      const bool isLogarithmic);

   /// Resets the iterator and returns first iteration value if available (return value is true).
   /// The maxIterations parameter is a belts and braces safety check to avoid infinite looping.
   ///
   bool firstValue (double& value, bool& isMajor, const int maxIterations = 10000);

   /// Returns next value if available indicated by return value being true.
   //
   bool nextValue  (double& value, bool& isMajor);

   double getMinimum () const;          /// Extracts iterator's minimum value
   double getMaximum () const;          /// Extracts iterator's maximum value
   double getMinorInterval () const;    /// Extracts iterator's minor interval
   int    getMajorMinorRatio () const;  /// Extracts iterator's major/minor ratio
   bool   getIsLogarithmic () const;    /// Extracts iterator's is logarithmic indicator

private:
   double minimum;
   double maximum;
   double minorInterval;
   int    majorMinorRatio;
   bool   isLogarithmic;

   double origin;             // choosen such that iteratorControl does not overflow.
   int    iteratorControl;    // value = (iteratorControl * minorInterval) + origin
   double minTolerance;       // accomodate rounding errors at lower limit
   double maxTolerance;       // accomodate rounding errors at upper limit
   int    iterationCount;     // iteration count
   int    maxIterations;      // max allowed iteration
};

#endif  // QE_AXIS_ITERATOR_H
