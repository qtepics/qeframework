/*  archapplData.h
 *
 *  A library that takes Google Protocol Buffers data for one PV
 *  recieved from Archiver Appliance, parses it and returns a vector
 *  of object cointaining said data where each object represents one data point.
 *
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

#ifndef ARCHAPPLDATA_H
#define ARCHAPPLDATA_H

#include <vector>
#include <map>
#include <string>

#include <shareLib.h>

namespace ArchapplData {

   // Data structure holding all available information of one data point
   //
   struct PBData {
      double value;
      int year;
      int seconds;
      int nanos;
      std::map<std::string,std::string> fieldValues;
      int severity;
      int status;
   };


   /**
    * Google Protcol Buffers processing
    * 
    * params:
    *  - std::vector<char> *pbData     [in]  pointer to char vector recieved from AA containint PB data for one PV
    *  - int &precision                [out]
    *  - std::string &pvName           [out]
    *  - std::string &units            [out]
    *  - double &displayHigh           [out]
    *  - double &displayLow            [out]
    *  - std::vector<PBData> &pvData   [out] Vector of data points
    */
   epicsShareFunc void processProtoBuffers(std::vector<char> *pbData,
                            int &precision,
                            std::string &pvName,
                            std::string &units,
                            double &displayHigh,
                            double &displayLow,
                            std::vector<PBData> &pvData);

}

#endif // ARCHAPPLDATA_H
