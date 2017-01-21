/*
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

// Provides a data holder for EPICS types

#ifndef CARECORD_H_
#define CARECORD_H_

#include <vector>
#include <string>
#include <cadef.h>
#include <Generic.h>


namespace carecord {

  enum dbr_translation_type { BASIC, STATUS, TIME, GRAPHIC, CONTROL };
  enum process_state { NO_UPDATE, FIRST_UPDATE, UPDATE };

  // Total number of different basic EPICS types
  const short TYPE_COUNT = 7;

  // CA property for bound limits
  struct ca_limit {
      double upper;
      double lower;
  };
    
  class CaRecord : public generic::Generic {
    public:
      CaRecord();
      ~CaRecord();

      CaRecord( CaRecord &param );
      CaRecord& operator= ( CaRecord &param );

      void setName( std::string nameIn );
      void setDbrType( short dbrType );
      void setValid( bool newValid );
      void updateProcessState();

      void setStatus( short newStatus );
      void setAlarmSeverity( short newSeverity );
      void setPrecision( short newPrecision );
      void setUnits( std::string newUnits );

      void setTimeStamp( unsigned long timeStampSecondsIn, unsigned long timeStampNanosecondsIn );
      void clearEnumState();
      void addEnumState( std::string newEnumStates );

      void setDisplayLimit( double newUpper, double newLower );
      void setAlarmLimit( double newUpper, double newLower );
      void setWarningLimit( double newUpper, double newLower );
      void setControlLimit( double newUpper, double newLower );

      std::string getName() { return name; }
      short getDbrType( const dbr_translation_type type );
      short getDbrTranslation( const short translationMatrix[TYPE_COUNT][2], short type );
      bool isValid() { return valid; }
      process_state getProcessState() { return processState; }
      bool isFirstUpdate();

      short getStatus() { return status; }
      short getAlarmSeverity() { return alarmSeverity; }
      short getPrecision() { return precision; }
      std::string getUnits() { return units; }

      unsigned long getTimeStampSeconds() { return timeStampSeconds; }
      unsigned long getTimeStampNanoseconds() { return timeStampNanoseconds; }
      std::string getEnumState( int position ) { return enumStates[position]; }
      int getEnumStateCount() { return (int) enumStates.size(); }

      ca_limit getDisplayLimit() { return display; }
      ca_limit getAlarmLimit() { return alarm; }
      ca_limit getWarningLimit() { return warning; }
      ca_limit getControlLimit() { return control; }

    private:
      std::string name;
      short dbrType;     // essentially the field type
      bool valid;
      process_state processState;

      short status;
      short alarmSeverity;
      short precision;
      std::string units;
      unsigned long timeStampSeconds;
      unsigned long timeStampNanoseconds;
      std::vector<std::string> enumStates;
      ca_limit display;
      ca_limit alarm;
      ca_limit warning;
      ca_limit control;

      void reset();
  };
  
  // Translation matrix for EPICS basic to EPICS status type
  const short statusTranslationMatrix[TYPE_COUNT][2] = {
      { DBR_STRING, DBR_STS_STRING },
      { DBR_INT, DBR_STS_INT },
      { DBR_FLOAT, DBR_STS_FLOAT },
      { DBR_ENUM, DBR_STS_ENUM },
      { DBR_CHAR, DBR_STS_CHAR },
      { DBR_LONG, DBR_STS_LONG },
      { DBR_DOUBLE, DBR_STS_DOUBLE }
  };

  // Translation matrix for EPICS basic to EPICS control type
  const short controlTranslationMatrix[TYPE_COUNT][2] = {
      { DBR_STRING, DBR_CTRL_STRING },
      { DBR_INT, DBR_CTRL_INT },
      { DBR_FLOAT, DBR_CTRL_FLOAT },
      { DBR_ENUM, DBR_CTRL_ENUM },
      { DBR_CHAR, DBR_CTRL_CHAR },
      { DBR_LONG, DBR_CTRL_LONG },
      { DBR_DOUBLE, DBR_CTRL_DOUBLE }
  };

  // Translation matrix for EPICS basic to EPICS time type
  const short timeTranslationMatrix[TYPE_COUNT][2] = {
      { DBR_STRING, DBR_TIME_STRING },
      { DBR_INT, DBR_TIME_INT },
      { DBR_FLOAT, DBR_TIME_FLOAT },
      { DBR_ENUM, DBR_TIME_ENUM },
      { DBR_CHAR, DBR_TIME_CHAR },
      { DBR_LONG, DBR_TIME_LONG },
      { DBR_DOUBLE, DBR_TIME_DOUBLE }
  };

  // Translation matrix for EPICS basic to EPICS graphic type
  const short graphicTranslationMatrix[TYPE_COUNT][2] = {
      { DBR_STRING, DBR_GR_STRING },
      { DBR_INT, DBR_GR_INT },
      { DBR_FLOAT, DBR_GR_FLOAT },
      { DBR_ENUM, DBR_GR_ENUM },
      { DBR_CHAR, DBR_GR_CHAR },
      { DBR_LONG, DBR_GR_LONG },
      { DBR_DOUBLE, DBR_GR_DOUBLE }
  };

}

#endif  // CARECORD_H_
