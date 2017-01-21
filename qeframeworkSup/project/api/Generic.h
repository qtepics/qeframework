/*  Generic.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
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
 *  Copyright (c) 2009, 2010 Australian Synchrotron
 *
 *  Author:
 *    Anthony Owen
 *  Contact details:
 *    anthony.owen@gmail.com
 */

#ifndef GENERIC_H_
#define GENERIC_H_

#include <string>
#include <QtGlobal>

namespace generic {

  enum generic_types { GENERIC_STRING,
                       GENERIC_SHORT, GENERIC_UNSIGNED_SHORT,
                       GENERIC_UNSIGNED_CHAR,
                       GENERIC_LONG, GENERIC_UNSIGNED_LONG,
                       GENERIC_FLOAT, GENERIC_DOUBLE,
                       GENERIC_UNKNOWN };

  class Generic {
    public:
      Generic();
      Generic( std::string newValue );
      Generic( short newValue );
      Generic( short* newValueArray, unsigned long countIn );
      Generic( unsigned short newValue );
      Generic( unsigned short* newValueArray, unsigned long countIn );
      Generic( unsigned char newValue );
      Generic( unsigned char* newValueArray, unsigned long countIn );
      Generic( qint32 newValue );
      Generic( qint32* newValueArray, unsigned long countIn );
      Generic( quint32 newValue );
      Generic( quint32* newValueArray, unsigned long countIn );
      Generic( float newValue );
      Generic( float* newValueArray, unsigned long countIn );
      Generic( double newValue );
      Generic( double* newValueArray, unsigned long countIn );
      virtual ~Generic();

      Generic( Generic &param );
      Generic& operator= ( Generic &param );

      void setString( std::string newValue );
      void setShort( short newValue );
      void setShort( short* newValueArray, unsigned long countIn );
      void updateShort( short newValue, unsigned long arrayIndex );
      void setUnsignedShort( unsigned short newValue );
      void setUnsignedShort( unsigned short* newValueArray, unsigned long countIn );
      void updateUnsignedShort( unsigned short newValue, unsigned long arrayIndex );
      void setUnsignedChar( unsigned char newValue );
      void setUnsignedChar( unsigned char* newValueArray, unsigned long countIn );
      void updateUnsignedChar( unsigned char newValue, unsigned long arrayIndex );
      void setLong( qint32 newValue );
      void setLong( qint32* newValueArray, unsigned long countIn );
      void updateLong( qint32 newValue, unsigned long arrayIndex );
      void setUnsignedLong( quint32 newValue );
      void setUnsignedLong( quint32* newValueArray, unsigned long countIn );
      void updateUnsignedLong( quint32 newValue, unsigned long arrayIndex );
      void setFloat( float newValue );
      void setFloat( float* newValueArray, unsigned long countIn );
      void updateFloat( float newValue, unsigned long arrayIndex );
      void setDouble( double newValue );
      void setDouble( double* newValueArray, unsigned long countIn );
      void updateDouble( double newValue, unsigned long arrayIndex );

      std::string getString();
      void getString( char** valueArray, unsigned long* countOut = NULL );
      short getShort();
      void  getShort( short** valueArray, unsigned long* countOut = NULL );
      unsigned short getUnsignedShort();
      void           getUnsignedShort( unsigned short** valueArray, unsigned long* countOut = NULL );
      unsigned char getUnsignedChar();
      void          getUnsignedChar( unsigned char** valueArray, unsigned long* countOut = NULL );
      qint32 getLong();
      void getLong( qint32** valueArray, unsigned long* countOut = NULL );
      quint32 getUnsignedLong();
      void          getUnsignedLong( quint32** valueArray, unsigned long* countOut = NULL );
      float getFloat();
      void  getFloat( float** valueArray, unsigned long* countOut = NULL );
      double getDouble();
      void   getDouble( double** valueArray, unsigned long* countOut = NULL );

      unsigned long getArrayCount();

      generic_types getType();
      void setType( generic_types newType );

    protected:
      void cloneValue( Generic *param );

    private:
      unsigned long arrayCount;
      generic_types type;
      void* value;

      void deleteValue();
      int getSize( generic_types type );
  };

}

#endif  // GENERIC_H_
