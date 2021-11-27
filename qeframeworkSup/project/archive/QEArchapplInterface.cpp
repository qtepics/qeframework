/*  QEArchapplInterface.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2021 Australian Synchrotron
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
 *  Author:
 *    Andraz Pozar
 *  Contact details:
 *    andrazp@ansto.gov.au
 */

#include "QEArchapplInterface.h"
#include <QDebug>
#include <QtCore>
#include <QtXml>
#include <QVariantList>
#include <alarm.h>
#include <iostream>
#include <cfloat>
#include <cstring>
#include <string>
#include <vector>
#include <map>

// Enable Archiver Appliance support
//
#ifdef QE_ARCHAPPL_SUPPORT
   #include <QJsonDocument>
   #include <QJsonObject>
   #include <QUrlQuery>
   #include "../../../include/archapplData.h"
#endif

#define DEBUG qDebug () << "QEArchiveInterfaceAA" << __LINE__ << __FUNCTION__  << "  "


//------------------------------------------------------------------------------
// Enable Archiver Appliance support
//
#ifdef QE_ARCHAPPL_SUPPORT
typedef QMap<QEArchiveInterface::Methods, QString> URLMap;
static URLMap requestMethodToURL;

typedef QMap<QEArchiveInterface::How, QString> PostProcessingMap;
static PostProcessingMap howToPostPP;

static bool setupMaps () {
   requestMethodToURL.insert (QEArchiveInterface::Information, QString("getApplianceInfo"));
   requestMethodToURL.insert (QEArchiveInterface::Names,       QString("getTimeSpanReport"));
   requestMethodToURL.insert (QEArchiveInterface::Values,      QString("data/getData.raw"));

   howToPostPP.insert (QEArchiveInterface::Raw,         QString(""));
   howToPostPP.insert (QEArchiveInterface::SpreadSheet, QString(""));
   howToPostPP.insert (QEArchiveInterface::Averaged,    QString("mean_"));
   howToPostPP.insert (QEArchiveInterface::PlotBinning, QString("caplotbinning_"));
   howToPostPP.insert (QEArchiveInterface::Linear,      QString("linear_"));

   return true;
}

static const bool elaborateMaps = setupMaps ();


//==============================================================================
// QEArchapplNetworkManager
//==============================================================================
//
QEArchapplNetworkManager::QEArchapplNetworkManager(const QUrl& bplURL, QObject* parent) :
   QObject (parent)
{
   this->bplURL = bplURL;
   networkManager = new QNetworkAccessManager(this);
}

QEArchapplNetworkManager::~QEArchapplNetworkManager()
{
   delete networkManager;
}

void QEArchapplNetworkManager::getApplianceInfo(const QEArchiveInterface::Context& context)
{
   URLMap::const_iterator it = requestMethodToURL.find(context.method);
   if (it != requestMethodToURL.end()) {
      QString query = it.value();
      QUrl url = this->bplURL.resolved(query);
      executeRequest(url, context);
   }
}

void QEArchapplNetworkManager::getPVs(const QEArchiveInterface::Context& context, const QString& pattern)
{
   URLMap::const_iterator it = requestMethodToURL.find(context.method);
   if (it != requestMethodToURL.end()) {
      QString endPoint = it.value();
      QUrl url = this->bplURL.resolved(endPoint);

      // Set regular expression to be used when retrieving PVs list
      //
      if (!pattern.isEmpty()) {
         QUrlQuery query;
         query.addQueryItem("regex", pattern);
         url.setQuery(query);
      }

      executeRequest(url, context);
   }
}

void QEArchapplNetworkManager::getValues(const QEArchiveInterface::Context& context,
                                         const ValuesRequest& request,
                                         const unsigned int binSize = 0)
{
   URLMap::const_iterator it = requestMethodToURL.find(context.method);
   if (it != requestMethodToURL.end()) {
      QString endPoint = it.value();

      // Check if dataURL is known. This is needed because dataURL is retrieved
      // from the Archiver Appliance upon the first connection.
      // This is mostly a sainity check.
      //
      if (this->dataURL.isEmpty()) {
         return;
      }
      QUrl url = this->dataURL.resolved(endPoint);

      for (int i = 0; i < request.names.count(); i++) {
         const QString pvName = request.names.at(i);

         // Set a post processing option based on QEArchiveInterface::How
         //
         QString postProcessingString;
         PostProcessingMap::const_iterator it = howToPostPP.find(request.how);
         if (it != howToPostPP.end()) {
            postProcessingString = it.value();
         }

         // Set bin size
         //
         QString pvNameWithPP;
         if (!postProcessingString.isEmpty() && binSize > 0) {
            pvNameWithPP = postProcessingString + QString::number(binSize) + "(" + pvName + ")";
         } else {
            pvNameWithPP = pvName;
         }

         QUrlQuery query;

         query.addQueryItem("pv", pvNameWithPP);

         // If the old data will be requested from CA Archiver, set the number
         // of data points
         //
         query.addQueryItem("ca_count", QString("5000"));

         query.addQueryItem("from", request.startTime);
         query.addQueryItem("to", request.endTime);
         QUrl pvUrl(url);
         pvUrl.setQuery(query);
         executeRequest(pvUrl, context);
      }
   }
}

void QEArchapplNetworkManager::executeRequest(const QUrl url,
                                              const QEArchiveInterface::Context& context)
{
   // Set URL of the request and request the data
   //
   QNetworkRequest request;
   request.setUrl(url);
   QNetworkReply* reply = this->networkManager->get(request);

   // Set the context as a part of reply so that the slot catching finished() signal
   // knows how to handle the response
   //
   QVariant variant;
   variant.setValue(context);
   reply->setProperty("context", variant);

   // Do the plumbing
   //
   QObject::connect (reply, SIGNAL(finished()), this, SLOT(replyFinished()));
}

void QEArchapplNetworkManager::replyFinished()
{
   QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

   if (reply) {
      QVariant property = reply->property("context");
      QEArchiveInterface::Context context = qvariant_cast<QEArchiveInterface::Context>(property);
      if (reply->error() == QNetworkReply::NoError) {
         emit this->networkManagerResponse(context, reply);
      } else {
         emit this->networkManagerFault(context, reply->error());
      }
   }

   // We don't delete the reply straight away but we give it time to be read
   // by the slot processing the data and schedule it for deletion
   //
   reply->deleteLater();
}


//==============================================================================
// QEArchapplInterface
//==============================================================================
//
QEArchapplInterface::QEArchapplInterface (QUrl url, QObject *parent) : QEArchiveInterface(parent)
{
   // Append a trailing "/" to the url if need be.
   //
   if (!url.path().endsWith ("/")) {
      url.setPath(url.path() + "/");
   }

   this->mUrl = url;
   this->networkManager = new QEArchapplNetworkManager (this->mUrl, this);

   QObject::connect(this->networkManager, SIGNAL (networkManagerResponse(const QEArchiveInterface::Context&, QNetworkReply*)),
                    this,                 SLOT   (networkManagerResponse(const QEArchiveInterface::Context&, QNetworkReply*)));

   QObject::connect(this->networkManager, SIGNAL (networkManagerFault(const QEArchiveInterface::Context&, const QNetworkReply::NetworkError)),
                    this,                 SLOT   (networkManagerFault(const QEArchiveInterface::Context&, const QNetworkReply::NetworkError)));

   // Request info upon creation so that we can get the URL which is used to retrieve data
   //
   QObject userData;
   this->infoRequest(&userData);
}

//------------------------------------------------------------------------------
//
QEArchapplInterface::~QEArchapplInterface () { }

//------------------------------------------------------------------------------
//
void QEArchapplInterface::namesRequest (QObject* userData, const int /* key */, QString pattern)
{
   Context context;

   // Set up context
   //
   context.method = Names;
   context.userData = userData;
   context.requested_element = 0;

   if (this->networkManager != 0)
   {
      this->networkManager->getPVs(context, pattern);
   }
}

//------------------------------------------------------------------------------
//
void QEArchapplInterface::valuesRequest (QObject* userData,
                                         const QCaDateTime startTime,
                                         const QCaDateTime endTime,
                                         const int count,
                                         const How how,
                                         const QStringList pvNames,
                                         const int /* key */,
                                         const unsigned int requested_element)
{
   Context context;
   QStringList list;
   int j;

   QEArchapplNetworkManager::ValuesRequest request;
   // Set up context
   //
   context.method = Values;
   context.userData = userData;
   context.requested_element = requested_element;

   // Convert list of QStrings to a list of QVariants that hold QString values.
   //
   for (j = 0; j < pvNames.count (); j++) {
      list.append (pvNames.value (j));
   }
   request.names = list;
   request.how = how;
   request.startTime = startTime.ISOText();
   request.endTime = endTime.ISOText();
   request.count = count;

   // Number of points to be used to calculate whichver posprocessing method is being used
   //
   unsigned int binSize = 0;
   double secondsDiff = startTime.secondsTo(endTime);
   if (secondsDiff > count)
   {
      binSize = (unsigned int) secondsDiff/count;
   }

   if (this->networkManager != 0)
   {
      this->networkManager->getValues(context, request, binSize);
   }
}

//------------------------------------------------------------------------------
//
void QEArchapplInterface::infoRequest (QObject* userData)
{
   Context context;

   // Set up context
   //
   context.method = Information;
   context.userData = userData;
   context.requested_element = 0;

   if (this->networkManager != 0)
   {
      this->networkManager->getApplianceInfo(context);
   }
}

//------------------------------------------------------------------------------
//
void QEArchapplInterface::archivesRequest (QObject* userData)
{
   // Archiver appliance doesn't provide information equal to that of archiver.archives
   // by EPICS Channel Archiver. That's why we don't even try to get anything from it
   // but call the process function directly
   //
   this->processArchives(userData);
}

//------------------------------------------------------------------------------
//
void QEArchapplInterface::networkManagerResponse(const QEArchiveInterface::Context & context,
                                                 QNetworkReply* reply)
{
   switch (context.method) {

   case Information:
      this->processInfo (context.userData, reply);
      break;

   case Names:
      this->processPvNames (context.userData, reply);
      break;

   case Values:
      this->processValues (context.userData, reply, context.requested_element);
      break;

   default:
      DEBUG << "unexpected method: " << context.method;
      break;
   };
}

//------------------------------------------------------------------------------
//
void QEArchapplInterface::networkManagerFault(const QEArchiveInterface::Context & context,
                                              const QNetworkReply::NetworkError error)
{
   PVNameList nullPvNames;
   ResponseValueList nullPvValues;

   switch (context.method) {

   case Information:
      emit this->infoResponse (context.userData, false, 0, "");
      break;

   case Names:
      emit this->pvNamesResponse (context.userData, false, nullPvNames);
      break;

   case Values:
      emit this->valuesResponse (context.userData, false, nullPvValues);
      break;

   default:
      DEBUG << "unexpected method: " << context.method << error;
      break;
   };
}

//------------------------------------------------------------------------------
//
void QEArchapplInterface::processInfo(const QObject* userData, QNetworkReply* reply)
{
   // We get a JSON encoded data
   //
   QByteArray arrayData = reply->readAll();
   QJsonDocument jsonDocument = QJsonDocument::fromJson(arrayData);
   if(jsonDocument.isNull()) {
      DEBUG << "data received is not JSON encoded";
      return;
   }

   const QJsonObject jsonObject = jsonDocument.object();

   // If this is the first time this function has been called, data retrieval URL
   // is not yet set, so it gets set here
   //
   if (this->networkManager->dataURL.isEmpty()) {
      QJsonValue retrievalURL = jsonObject.value("dataRetrievalURL");
      if (retrievalURL == QJsonValue::Undefined) {
         DEBUG << "could not resolve data retrieval URL";
         return;
      }
      this->networkManager->dataURL = QUrl(retrievalURL.toString().append("/"));
   }  

   // Description and version are embodied in the description since the Atchiver Appliance
   // doesn't provide and integer version number but rather a snapshot version with a date.
   //
   const int version = 0;
   QString description;
   QJsonValue descriptionObject = jsonObject.value("version");

   if (descriptionObject == QJsonValue::Undefined) {
      DEBUG << "no description found, not critical";
   } else {
      description = descriptionObject.toString();
   }

   emit this->infoResponse (userData, true, version, description);
}

//------------------------------------------------------------------------------
//
void QEArchapplInterface::processPvNames  (const QObject* userData, QNetworkReply* reply)
{
   PVNameList PvNames;
   struct PVName item;

   QByteArray arrayData = reply->readAll();

   if (arrayData.isNull()) {
      DEBUG << "response empty";
      return;
   }

   QJsonDocument jsonDocument = QJsonDocument::fromJson(arrayData);
   if(jsonDocument.isNull()) {
      DEBUG << "data received is not JSON encoded";
      return;
   }

   const QJsonObject jsonObject = jsonDocument.object();
   QJsonObject::const_iterator dataIterator = jsonObject.begin();

   while (dataIterator != jsonObject.end()) {
      const QJsonObject  onePVData = (*(dataIterator++)).toObject();
      QString name = onePVData.value("pvName").toString();
      int startTime = onePVData.value("creationTS").toString().toInt();

      item.pvName = name;
      item.startTime = this->convertArchiveToEpics (startTime, 0);

      QString paused = onePVData.value("paused").toString();
      if (paused.compare(QString("false")) == 0) {
         int endTime = onePVData.value("lastEvent").toString().toInt();
         item.endTime = this->convertArchiveToEpics (endTime, 0);
      }

      PvNames.append (item);
   }

   emit this->pvNamesResponse (userData, true, PvNames);
}

//------------------------------------------------------------------------------
// A dummy function just to fill in the expectations of QEArchiveAccess
//
void QEArchapplInterface::processArchives (const QObject* userData)
{
   ArchiveList PvArchives;
   struct Archive item;

   item.key  = 0;
   item.name = QString("Archiver Appliance");
   item.path = QString("");

   PvArchives.append (item);

   emit this->archivesResponse (userData, true, PvArchives);
}

//------------------------------------------------------------------------------
//
void QEArchapplInterface::processValues(const QObject* userData, QNetworkReply* reply,
                                        const unsigned int /* requested_element */)
{
   QByteArray arrayData = reply->readAll();

   if (arrayData.isNull()) {
      DEBUG << "response empty";
      return;
   }

   std::vector<char> charData;
   charData.assign(arrayData.data(), arrayData.data() + arrayData.size());

   std::vector<ArchapplData::PBData> pvData;
   int precision;
   std::string pvName;
   std::string units;
   double displayHigh;
   double displayLow;
   ArchapplData::processProtoBuffers(&charData, precision, pvName, units, displayHigh, displayLow, pvData);

   ResponseValueList PvValues;
   QCaDataPointList dataPointList;

   std::vector<ArchapplData::PBData>::iterator it = pvData.begin();
   while (it != pvData.end()) {
      ArchapplData::PBData onePointData = *it;

      // To save space, the record processing timestamps in the samples are split into three parts
      // 1. year - This is stored once in the PB file in the header.
      // 2. secondsintoyear - This is stored with each sample.
      // 3. nano - This is stored with each sample.
      //
      // Here we combine all three into one timestamp and covert to local time
      //
      QDateTime pointDateTime(QDate(onePointData.year, 1, 1));
      pointDateTime.setTimeSpec(Qt::UTC);
      pointDateTime = pointDateTime.addSecs(onePointData.seconds);
      pointDateTime = pointDateTime.addMSecs((int)(onePointData.nanos/1000000));
      pointDateTime = pointDateTime.toLocalTime();
      QCaDateTime caDateTime(pointDateTime);

      // Create a data point structure used by other clients
      //
      QCaDataPoint dataPoint;
      dataPoint.value = onePointData.value;
      dataPoint.alarm = QCaAlarmInfo(onePointData.status, onePointData.severity);
      dataPoint.datetime = caDateTime;
      dataPointList.append(dataPoint);

      it++;
   }

   ResponseValues responseValues;
   responseValues.dataPoints = dataPointList;
   responseValues.precision = precision;
   responseValues.pvName = QString::fromStdString(pvName);
   responseValues.units = QString::fromStdString(units);
   responseValues.displayHigh = displayHigh;
   responseValues.displayLow = displayLow;
   responseValues.elementCount = dataPointList.count();

   PvValues.push_back(responseValues);   

   emit this->valuesResponse (userData, true, PvValues);
}

#else

//------------------------------------------------------------------------------
// QE framework is being only built for CA Archiver only.
//
QEArchapplNetworkManager::QEArchapplNetworkManager(const QUrl&, QObject* parent) : QObject (parent) {}

QEArchapplNetworkManager::~QEArchapplNetworkManager() {}

void QEArchapplNetworkManager::getPVs(const QEArchiveInterface::Context&, const QString&) {}

void QEArchapplNetworkManager::getApplianceInfo(const QEArchiveInterface::Context&) {}

void QEArchapplNetworkManager::executeRequest(const QUrl, const QEArchiveInterface::Context&) {}

void QEArchapplNetworkManager::getValues(const QEArchiveInterface::Context&, const ValuesRequest&, const unsigned int) {}

void QEArchapplNetworkManager::replyFinished() {}

QEArchapplInterface::QEArchapplInterface (QUrl, QObject*) {}

QEArchapplInterface::~QEArchapplInterface () {}

void QEArchapplInterface::namesRequest (QObject*, const int, QString) {}

void QEArchapplInterface::valuesRequest (QObject*,
                    const QCaDateTime,
                    const QCaDateTime,
                    const int,
                    const How,
                    const QStringList,
                    const int,
                    const unsigned int) {}

void QEArchapplInterface::infoRequest (QObject*) {}

void QEArchapplInterface::archivesRequest (QObject*) {}

void QEArchapplInterface::networkManagerResponse (const QEArchiveInterface::Context &,
                                                  QNetworkReply*) {}

void QEArchapplInterface::networkManagerFault (const QEArchiveInterface::Context&,
                                               const QNetworkReply::NetworkError) {}

#endif

// end
