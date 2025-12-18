/*  imageDataFormats.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2014-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_IMAGE_DATA_FORMATS_H
#define QE_IMAGE_DATA_FORMATS_H

#include <QString>
#include <QEEnums.h>

/// The formats are now defined in QEEnums
///
class imageDataFormats
{
public:
   // Returns false if given text does not match one of the expected formats
   //
   static bool convertToFormatOption (const QString& text,
                                      QE::ImageFormatOptions& format);

   static QString getFormatInformation (const QE::ImageFormatOptions format);
};

#endif // QE_IMAGE_DATA_FORMATS_H
