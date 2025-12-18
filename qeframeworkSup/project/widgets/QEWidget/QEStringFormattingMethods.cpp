/*  QEStringFormattingMethods.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2009-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

/*
  This class is used by QEWidgets when setting properties managed by the QEStringformatting class.
  This class calls matching set and get methods from the QEStringformatting class but also calls back
  the QEWidget when setting properties so the widget can update itself based on the changes.
 */

#include <QEStringFormattingMethods.h>

// Place holders
//
QEStringFormattingMethods::QEStringFormattingMethods(){}

QEStringFormattingMethods::~QEStringFormattingMethods(){}

// String formatting properties

// precision
void QEStringFormattingMethods::setPrecision( int precision )
{
    this->stringFormatting.setPrecision( precision );
    this->stringFormattingChange();
}
int QEStringFormattingMethods::getPrecision() const
{
    return this->stringFormatting.getPrecision();
}

// useDbPrecision
void QEStringFormattingMethods::setUseDbPrecision( bool useDbPrecision )
{
    this->stringFormatting.setUseDbPrecision( useDbPrecision);
    this->stringFormattingChange();
}
bool QEStringFormattingMethods::getUseDbPrecision() const
{
    return this->stringFormatting.getUseDbPrecision();
}

// leadingZeros
void QEStringFormattingMethods::setLeadingZeros( const int leadingZeros )
{
   this->stringFormatting.setLeadingZeros( leadingZeros );
   this->stringFormattingChange();
}
int QEStringFormattingMethods::getLeadingZeros() const
{
   return this->stringFormatting.getLeadingZeros();
}

// leadingZero - deprecated
void QEStringFormattingMethods::setLeadingZero( bool )
{
}
bool QEStringFormattingMethods::getLeadingZero() const
{
    return true;
}

// trailingZeros - deprecated
void QEStringFormattingMethods::setTrailingZeros( bool )
{
}
bool QEStringFormattingMethods::getTrailingZeros() const
{
    return true;
}

// forceSign
void QEStringFormattingMethods::setForceSign( bool forceSign )
{
    this->stringFormatting.setForceSign( forceSign );
    this->stringFormattingChange();
}
bool QEStringFormattingMethods::getForceSign() const
{
    return this->stringFormatting.getForceSign();
}

// addUnits
void QEStringFormattingMethods::setAddUnits( bool addUnits )
{
    this->stringFormatting.setAddUnits( addUnits );
    this->stringFormattingChange();
}
bool QEStringFormattingMethods::getAddUnits() const
{
    return this->stringFormatting.getAddUnits();
}

// localEnumeration
void QEStringFormattingMethods::setLocalEnumeration( QString localEnumeration )
{
    this->stringFormatting.setLocalEnumeration( localEnumeration );
    this->stringFormattingChange();
}
QString QEStringFormattingMethods::getLocalEnumeration() const
{
    return this->stringFormatting.getLocalEnumeration();
}

// format
void QEStringFormattingMethods::setFormat( QE::Formats format )
{
    this->stringFormatting.setFormat( format );
    this->stringFormattingChange();
}
QE::Formats QEStringFormattingMethods::getFormat() const
{
    return this->stringFormatting.getFormat();
}

// separator
void QEStringFormattingMethods::setSeparator( const QE::Separators separator )
{
    this->stringFormatting.setSeparator( separator );
    this->stringFormattingChange();
}
QE::Separators QEStringFormattingMethods::getSeparator() const
{
   return this->stringFormatting.getSeparator();
}

// radix
void QEStringFormattingMethods::setRadix( const int radix )
{
    this->stringFormatting.setRadix( radix);
    this->stringFormattingChange();
}
unsigned int QEStringFormattingMethods::getRadix() const
{
    return this->stringFormatting.getRadix();
}

// notation
void QEStringFormattingMethods::setNotation( QE::Notations notation )
{
    this->stringFormatting.setNotation( notation );
    this->stringFormattingChange();
}
QE::Notations QEStringFormattingMethods::getNotation() const
{
    return this->stringFormatting.getNotation();
}

// arrayAction
void QEStringFormattingMethods::setArrayAction( QE::ArrayActions arrayAction )
{
    this->stringFormatting.setArrayAction( arrayAction );
    this->stringFormattingChange();
}
QE::ArrayActions QEStringFormattingMethods::getArrayAction() const
{
    return this->stringFormatting.getArrayAction();
}

// local enuerations
QELocalEnumeration QEStringFormattingMethods::getLocalEnumerationObject() const
{
    return this->stringFormatting.getLocalEnumerationObject();
}

// end
