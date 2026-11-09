/*  QEPvaClient_STUB.cpp
 */

#include "QEPvaClient_STUB.h"

// QE_INCLUDE_PV_ACCESS not defined - just provide stub functions.

QEPvaClient::QEPvaClient (const QString& pvName,
                          QObject* parent) :
   QEBaseClient (QEBaseClient::PVAType, pvName, parent) { }
QEPvaClient::~QEPvaClient () { }
bool QEPvaClient::openChannel (const ChannelModesFlags) { return false; }
void QEPvaClient::closeChannel () { }
bool QEPvaClient::getIsConnected () const { return false; }
bool QEPvaClient::dataIsAvailable () const { return false; }
QString QEPvaClient::getId () const { return ""; }
QString QEPvaClient::getRemoteAddress() const { return ""; }
QString QEPvaClient::getEgu() const { return ""; }
int QEPvaClient::getPrecision() const { return 0; }
unsigned int QEPvaClient::hostElementCount () const { return 0; }
unsigned int QEPvaClient::dataElementCount () const { return 0; }
double QEPvaClient::getDisplayLimitHigh() const { return 0.0; }
double QEPvaClient::getDisplayLimitLow() const { return 0.0; }
double QEPvaClient::getHighAlarmLimit() const { return 0.0; }
double QEPvaClient::getLowAlarmLimit() const { return 0.0; }
double QEPvaClient::getHighWarningLimit() const { return 0.0; }
double QEPvaClient::getLowWarningLimit() const { return 0.0; }
double QEPvaClient::getControlLimitHigh() const { return 0.0; }
double QEPvaClient::getControlLimitLow() const { return 0.0; }
double QEPvaClient::getMinStep() const { return 0.0; }
QVariant QEPvaClient::getPvData () const { QVariant d; return d; }
bool QEPvaClient::putPvData (const QVariant&) { return false; }
QStringList QEPvaClient::getEnumerations () const { QStringList d; return d; }
QCaAlarmInfo QEPvaClient::getAlarmInfo () const { QCaAlarmInfo d; return d; }
QCaDateTime  QEPvaClient::getTimeStamp () const { QCaDateTime d; return d; }
QString QEPvaClient::getDescription () const { return ""; }
bool QEPvaClient::getReadAccess() const { return false; }
bool QEPvaClient::getWriteAccess() const { return false; }

// end
