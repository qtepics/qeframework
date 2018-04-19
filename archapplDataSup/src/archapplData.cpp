/*  archapplData.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
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
 *  Copyright (c) 2018 Australian Synchrotron
 *
 *  Author:
 *    Andraz Pozar
 *  Contact details:
 *    andraz.pozar@synchrotron.org.au
 */

#include <cfloat>
#include "archapplProto.pb.h"

#define epicsExportSharedSymbols
#include "archapplData.h"
#include <stdio.h>

namespace ArchapplData {
   // A generic template funciton to get all data for one point regardless of
   // a point value's data type
   //
   template <class T> void processValue(const void* vptr, int size,  ArchapplData::PBData& data) {
      T point;
      point.ParseFromArray(vptr, size);
      data.value = static_cast<double>(point.val());
      data.seconds = point.secondsintoyear();
      data.nanos = point.nano();
      data.severity = point.severity();
      data.status = point.status();

      for (int i = 0; i < point.fieldvalues_size(); i++) {
         const ArchapplPB::FieldValue& fieldValue = point.fieldvalues(i);
         data.fieldValues.insert(std::make_pair(fieldValue.name(), fieldValue.val()));
      }
   }

   void unescapePBData(std::vector<char> *pbData, std::vector< std::vector<char> > &unescapedLines)
   {
      // As serialized PB messages are binary data; after serialization, newline characters are escaped
      // to maintain a "sample per line" constraint:
      // 1. The ASCII escape character 0x1B is escaped to the following two characters 0x1B 0x01
      // 2. The ASCII newline character \n or 0x0A is escaped to the following two characters 0x1B 0x02
      // 3. The ASCII carriage return character 0x0D is escaped to the following two characters 0x1B 0x03
      //
      // To successfully parse deserialize the data we have to remove the escaping. Every time we find 0x1B
      // we know that this is an exaped character and is should be replaced by the character that follows.
      //
      const char ESCAPE_CHAR = 0x1B;
      const char ESCAPE_ESCAPE_CHAR = 0x01;
      const char NEWLINE_CHAR = 0x0A;
      const char NEWLINE_ESCAPE_CHAR = 0x02;
      const char CARRIAGERETURN_CHAR = 0x0D;
      const char CARRIAGERETURN_ESCAPE_CHAR = 0x03;

      std::vector<char> unescapedLineBuffer;

      for (size_t i = 0; i < pbData->size(); i++) {
         if (pbData->at(i) == NEWLINE_CHAR) {
            std::vector<char> unescapedLine = unescapedLineBuffer;
            unescapedLines.push_back(unescapedLine);
            unescapedLineBuffer = std::vector<char>();
         } else {
            char b = pbData->at(i);
            if (b == ESCAPE_CHAR) {
               b = pbData->at(++i);
               switch(b) {
               case ESCAPE_ESCAPE_CHAR:
                  unescapedLineBuffer.push_back(ESCAPE_CHAR);
                  break;
               case NEWLINE_ESCAPE_CHAR:
                  unescapedLineBuffer.push_back(NEWLINE_CHAR);
                  break;
               case CARRIAGERETURN_ESCAPE_CHAR:
                  unescapedLineBuffer.push_back(CARRIAGERETURN_CHAR);
                  break;
               default:
                  unescapedLineBuffer.push_back(b);
                  break;
               }
            } else {
               unescapedLineBuffer.push_back(b);
            }
         }
      }
   }


   void processProtoBuffers(std::vector<char> *pbData,
                            int &precision,
                            std::string &pvName,
                            std::string &units,
                            double &displayHigh,
                            double &displayLow,
                            std::vector<ArchapplData::PBData> &pvData)
   {
      ArchapplPB::PayloadInfo payloadInfo;
      std::vector< std::vector<char> > unescapedLines;

      // Archiver Appliance escapes special characters so that after serialization
      // each data point still falls in one line. To successfully parse the data
      // we first have to unsecape special characters.
      //
      unescapePBData(pbData, unescapedLines);

      bool headerComming = true;
      ArchapplPB::PayloadType type = ArchapplPB::SCALAR_STRING;
      int year = 0;
      bool eguAndPrecSet = false;
      displayHigh = DBL_MIN;
      displayLow = DBL_MAX;

      std::vector< std::vector<char> >::iterator it = unescapedLines.begin();
      while (it != unescapedLines.end()) {
         int lineLength = (*it).size();
         const void* line = &((*(it++))[0]);
         if (lineLength == 0) {
            // We're at an empty line
            //
            headerComming = true;
         } else if (headerComming && payloadInfo.ParseFromArray(line, lineLength)) {
            // We're at a header line containing PV name, year, data type and possibly but not
            // necessarily extra PV field values like EGU and PREC
            //
            pvName = payloadInfo.pvname();
            type = payloadInfo.type();
            year = payloadInfo.year();

            // We only set engineering units and precission once as they are the same
            // for the same PV
            //
            if (!eguAndPrecSet) {
               for (int i = 0; i < payloadInfo.headers_size(); i++) {
                  const ArchapplPB::FieldValue& value = payloadInfo.headers(i);
                  std::string name = value.name();
                  std::string fieldValue = value.val();
                  if (name == "EGU") {
                     units = fieldValue;
                  } else if (name == "PREC") {
                     //precision = std::stoi(fieldValue);
                     precision = atoi( fieldValue.c_str() );
                  }
               }
               eguAndPrecSet = true;
            }
            headerComming = false;
         } else {
            // We're at a line containing one PV data point along with timestamp,
            // severity and status
            //
            ArchapplData::PBData onePointData;
            switch (type) {
            case ArchapplPB::SCALAR_SHORT:
               processValue<ArchapplPB::ScalarShort>(line, lineLength, onePointData);
               break;
            case ArchapplPB::SCALAR_ENUM:
               processValue<ArchapplPB::ScalarEnum>(line, lineLength, onePointData);
               break;
            case ArchapplPB::SCALAR_FLOAT:
               processValue<ArchapplPB::ScalarFloat>(line, lineLength, onePointData);
               break;
            case ArchapplPB::SCALAR_DOUBLE:
               processValue<ArchapplPB::ScalarDouble>(line, lineLength, onePointData);
               break;
            default:
               printf("Unsupported data format\n");
               return;
            }

            // HOPR and LOPR of a PV are simply added to one or more data point values
            // We set them only once
            //
            if ((displayHigh ==  DBL_MIN || displayLow ==  DBL_MAX) && !onePointData.fieldValues.empty()) {
               std::map<std::string,std::string>::iterator it;
               for (it = onePointData.fieldValues.begin(); it != onePointData.fieldValues.end(); it++) {
                  std::string name = it->first;
                  std::string fieldValue = it->second;
                  if (name == "HOPR") {
                     displayHigh = atof(fieldValue.c_str());
                  } else if (name == "LOPR") {
                     displayLow = atof(fieldValue.c_str());
                  }
               }
            }

            onePointData.year = year;
            pvData.push_back(onePointData);

            headerComming = false;
         }
      }
   }
}

// end
