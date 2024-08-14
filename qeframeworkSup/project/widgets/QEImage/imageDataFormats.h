/*  imageDataFormats.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2014-2022 Australian Synchrotron
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
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
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
