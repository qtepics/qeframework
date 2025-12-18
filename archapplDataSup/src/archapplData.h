/*  archapplData.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2018-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andraz Pozar
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
