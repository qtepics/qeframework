/*  QEFileImage.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2022 Australian Synchrotron
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

/*
  This class is a CA aware label widget based on the Qt label widget.
  It is tighly integrated with the base class QEWidget. Refer to QEWidget.cpp for details
 */
#include "QEFileImage.h"
#include <QDebug>
#include <QFile>
#include <QImage>
#include <QECommon.h>

#define DEBUG qDebug() << "QEFileImage" << __LINE__ << __FUNCTION__ << "  "

enum Constants {
   PV_VARIABLE_INDEX = 0,
   NULL_THRESHOLD = 256
};

//------------------------------------------------------------------------------
// Constructor with no initialisation
//
QEFileImage::QEFileImage(QWidget *parent) :
   QLabel(parent),
   QEWidget(this),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX)
{
   this->setup();
}

//------------------------------------------------------------------------------
// Constructor with known variable
//
QEFileImage::QEFileImage(const QString &variableNameIn, QWidget *parent) :
   QLabel(parent),
   QEWidget(this),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX)
{
   this->setup();
   this->setVariableName(variableNameIn, 0);
   this->activate();
}

//------------------------------------------------------------------------------
// Place holder destructor
//
QEFileImage::~QEFileImage() {}

//------------------------------------------------------------------------------
// Setup common to all constructors
//
void QEFileImage::setup()
{
   this->threshold = NULL_THRESHOLD;
   this->thresholdColor= QColor("#ffffff");
   this->scaledContents = false;

   // Set up data
   // This control used a single data source
   this->setNumVariables(1);

   // Set minimum size
   this->setMinimumSize(100, 100);

   // Set up default properties
   this->setAllowDrop(false);

   // Set the initial state
   this->isConnected = false;

   // Use standard context menu
   this->setupContextMenu();

   //    defaultStyleSheet = styleSheet();
   // Use label signals
   // --Currently none--

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   this->connectNewVariableNameProperty
         (SLOT (useNewVariableNameProperty(QString, QString, unsigned int)));

   // Prepare to recieve notification of changes from the file monitor.
   //
   QObject::connect (&this->fileMon, SIGNAL(fileChanged(const QString &)),
                     this, SLOT(setImageFileName(const QString &)));
}


//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of QCaObject required.
// For a QEFileImage a QCaObject that streams strings is required.
//
qcaobject::QCaObject* QEFileImage::createQcaItem(unsigned int variableIndex)
{
   qcaobject::QCaObject* result = NULL;

   // Create the item as a QEString
   QString pvName = this->getSubstitutedVariableName(variableIndex);
   result = new QEString(pvName, this, &stringFormatting, variableIndex);

   // Apply currently defined array index/elements request values.
   this->setSingleVariableQCaProperties(result);

   return result;
}

//------------------------------------------------------------------------------
// Start updating. Implementation of VariableNameManager's virtual funtion to
// establish a connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QEFileImage::establishConnection(unsigned int variableIndex) {

   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals
   // will be returned.
   //
   qcaobject::QCaObject* qca = this->createConnection(variableIndex);

   // If a QCaObject object is now available to supply data update signals,
   // connect it to the appropriate slots.
   //
   if( qca) {
      QObject::connect(qca,  SIGNAL(stringChanged(const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)),
                       this, SLOT(  setLabelImage(const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)));
      QObject::connect(qca,  SIGNAL(connectionChanged(QCaConnectionInfo&, const unsigned int& )),
                       this, SLOT(  connectionChanged(QCaConnectionInfo&, const unsigned int& )));
      QObject::connect(this, SIGNAL(requestResend()),
                       qca,  SLOT(  resendLastData()));
   }
}


//------------------------------------------------------------------------------
//
void QEFileImage::useNewVariableNameProperty(QString variableNameIn,
                                              QString substitutionsIn,
                                              unsigned int variableIndex)
{
   this->setVariableNameAndSubstitutions(variableNameIn, substitutionsIn, variableIndex);
}

//------------------------------------------------------------------------------
// Act on a connection change.
// Change how the label looks and change the tool tip
// This is the slot used to recieve connection updates from a QCaObject based class.
//
void QEFileImage::connectionChanged(QCaConnectionInfo& connectionInfo,
                                    const unsigned int& variableIndex)
{
   // Note the connected state
   this->isConnected = connectionInfo.isChannelConnected();

   // Display the connected state
   this->updateToolTipConnection(isConnected, variableIndex);
   this->processConnectionInfo(isConnected, variableIndex);

   // Signal channel connection change to any (Link) widgets.
   // using signal dbConnectionChanged.
   //
   this->emitDbConnectionChanged(variableIndex);
}

//------------------------------------------------------------------------------
// Update the label pixmap from variable data.
// This is the slot used to recieve data updates from a QCaObject based class.
//
void QEFileImage::setLabelImage(const QString& textIn, QCaAlarmInfo& alarmInfo,
                                QCaDateTime&, const unsigned int& variableIndex)
{
   // Update the image
   this->setImageFileName(textIn);

   // Invoke common alarm handling processing.
   this->processAlarmInfo(alarmInfo, variableIndex);

   // Signal a database value change to any Link (or other) widgets using one
   // of the dbValueChanged signals declared in header file.
   this->emitDbValueChanged(variableIndex);
}

//------------------------------------------------------------------------------
// Slot to update the label pixmap from any source
//
void QEFileImage::setImageFileName(const QString& text)
{
   // Find the file
   QFile* imageFile =  QEWidget::findQEFile(text);

   // If filename not found
   if(!imageFile) {
      QPixmap emptyPixmap;
      this->setPixmap(emptyPixmap);
      this->fileName = text;
      this->setText(QString("File not found: ").append(text));
      return;
   }

   // Get the filename and discard the image file object.
   //
   this->fileName = imageFile->fileName();
   delete imageFile;

   // Clear any text
   this->clear();

   // Form pixmap object from the file
   QPixmap pixmap(this->fileName);

   // Do we need to apply a threshold?
   //
   if (this->threshold != NULL_THRESHOLD) {
      // Yes - convert to image.
      //
      QImage image = pixmap.toImage();
      const int nr = image.height();
      const int nc = image.width();

      // Is this an RGB32 image?
      //
      if (image.format() == QImage::Format_RGB32) {
         // Yes - do thresholding the fast way.
         //
         const QRgb tRgb = this->thresholdColor.rgb();

         for (int row = 0; row < nr; row++) {
            QRgb* rowData = (QRgb*) image.scanLine(row);

            for (int col = 0; col < nc; col++) {
               QColor pc = QColor (rowData[col]);

               // Decompise colour to find its lightness.
               //
               int h, s, l, a;
               pc.getHsl (&h, &s, &l, &a);

               if (l >= this->threshold) {
                  rowData[col] = tRgb;
               }
            }
         }
      } else {
         // Else do thresholding the slow way using more expensive pixelColor
         // and setPixelColor functions.
         //
         for (int row = 0; row < nr; row++) {
            for (int col = 0; col < nc; col++) {
               QColor pc = image.pixelColor(col, row);

               // Decompise colour to find its lightness.
               //
               int h, s, l, a;
               pc.getHsl (&h, &s, &l, &a);

               if (l >= this->threshold) {
                  image.setPixelColor(col, row, this->thresholdColor);
               }
            }
         }
      }

      // Lastly update the pixmap.
      //
      pixmap.convertFromImage(image);
   }

   // Update the label pixmap
   Qt::AspectRatioMode aspectMode;
   aspectMode = this->scaledContents ?  Qt::IgnoreAspectRatio : Qt::KeepAspectRatio;
   this->setPixmap (pixmap.scaled(this->size(), aspectMode));

   // Ensure no other files are being monitored.
   // We could be smarter here if same file name.
   //
   QStringList monitoredPaths = this->fileMon.files();
   if(monitoredPaths.count()) {
      this->fileMon.removePaths(monitoredPaths);
   }

   // Monitor this file
   this->fileMon.addPath (this->fileName);
}

//------------------------------------------------------------------------------
//
QString QEFileImage::getImageFileName () const
{
   return this->fileName;
}

//------------------------------------------------------------------------------
// slot function
void QEFileImage::setThresholdColor (const QColor thresholdColorIn)
{
   this->thresholdColor = thresholdColorIn;
   this->setImageFileName(this->fileName);   // trigger reprocessing of the same file
}

//------------------------------------------------------------------------------
//
QColor QEFileImage::getThresholdColor () const
{
   return this->thresholdColor;
}

//------------------------------------------------------------------------------
// slot function
void QEFileImage::setThreshold(const int thresholdIn)
{
   this->threshold = LIMIT(thresholdIn, 0, NULL_THRESHOLD);
   this->setImageFileName(this->fileName);   // trigger reprocessing of the same file
}

//------------------------------------------------------------------------------
//
int QEFileImage::getThreshold() const
{
   return this->threshold;
}

//------------------------------------------------------------------------------
// slot function
void QEFileImage::setScaledContents (bool scaledContentsIn)
{
   this->scaledContents = scaledContentsIn;
   this->setImageFileName(this->fileName);   // trigger reprocessing of the same file
}

//------------------------------------------------------------------------------
//
bool QEFileImage::getScaledContents () const
{
   return this->scaledContents;
}

//==============================================================================
// Copy / Paste
QString QEFileImage::copyVariable()
{
   return this->getSubstitutedVariableName(PV_VARIABLE_INDEX);
}

//------------------------------------------------------------------------------
//
QVariant QEFileImage::copyData()
{
   return QVariant(this->fileName);
}

//------------------------------------------------------------------------------
//
void QEFileImage::paste(QVariant v)
{
   this->setVariableName(v.toString(), PV_VARIABLE_INDEX);
   this->establishConnection(PV_VARIABLE_INDEX);
}

// end
