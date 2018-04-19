/*  QEArchiveStatus.cpp
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
 *  Copyright (c) 2013,2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QDebug>
#include <QLabel>
#include <QFrame>

#include <QECommon.h>
#include "QEArchiveStatus.h"

#define DEBUG  qDebug () << "QEArchiveStatus" << __LINE__ << __FUNCTION__ << "  "


//==============================================================================
//
void QEArchiveStatus::createInternalWidgets ()
{

#define CREATE_LABEL(member, width, align, text)  {              \
   this->rowList [j].member = new QLabel (text, frame);          \
   this->rowList [j].member->setIndent (6);                      \
   this->rowList [j].member->setMinimumWidth (width);            \
   this->rowList [j].member->setAlignment (align);               \
   this->rowList [j].member->setStyleSheet (sheet);              \
   hLayout->addWidget (this->rowList [j].member);                \
}

   const int frameHeight = 19;
   const int horMargin = 2;    // 19 - 2 - 2 => widget height is 15
   const int horSpacing = 4;

   int j;
   QColor background;
   QString sheet;
   QFrame* frame;
   QHBoxLayout *hLayout;

   this->archiveAccess = new QEArchiveAccess (this);
   this->archiveAccess->setMessageSourceId(9001);

   const QEArchiveAccess::ArchiverTypes archType = QEArchiveAccess::getArchiverType();

   this->vLayout = new QVBoxLayout (this);
   this->vLayout->setContentsMargins (2, 6, 2, 2);  // left, top, right, bottom
   this->vLayout->setSpacing (1);

   // Use use the last row as a header row.
   //
   j = NumberRows;
   sheet = "";
   this->rowList [j].frame = frame = new QFrame (this);
   frame->setFixedHeight (frameHeight);
   this->rowList [j].hLayout = hLayout = new QHBoxLayout (frame);
   hLayout->setMargin (horMargin);
   hLayout->setSpacing (horSpacing);

   CREATE_LABEL (hostNamePort, 160, Qt::AlignLeft,    "Host:Port");
   CREATE_LABEL (endPoint,     220, Qt::AlignLeft,    "End Point");
   CREATE_LABEL (state,         88, Qt::AlignHCenter, "Status");
   if (archType == QEArchiveAccess::CA) {
      CREATE_LABEL (available,     68, Qt::AlignRight,   "Available");
      CREATE_LABEL (read,          68, Qt::AlignRight,   "Read");
   }
   CREATE_LABEL (numberPVs,     68, Qt::AlignRight,   "Num PVs");
   if (archType == QEArchiveAccess::CA) {
      CREATE_LABEL (pending,       68, Qt::AlignRight,   "Pending");
   }
   this->vLayout->addWidget (frame);


   background = QColor (240, 240, 240, 255);
   sheet = QEUtilities::colourToStyle (background);

   for (j = 0; j < NumberRows; j++ ) {
      QEArchiveStatus::Rows* row = &this->rowList [j];

      row->frame = frame = new QFrame (this);
      frame->setFixedHeight (frameHeight);

      this->rowList [j].hLayout = hLayout = new QHBoxLayout (row->frame);
      hLayout->setMargin (horMargin);
      hLayout->setSpacing (horSpacing);

      CREATE_LABEL (hostNamePort, 160, Qt::AlignLeft,     " - ");
      CREATE_LABEL (endPoint,     220, Qt::AlignLeft,     " - ");
      CREATE_LABEL (state,         88, Qt::AlignHCenter,  " - ");
      if (archType == QEArchiveAccess::CA) {
         CREATE_LABEL (available,     68, Qt::AlignRight,    " - ");
         CREATE_LABEL (read,          68, Qt::AlignRight,    " - ");
      }
      CREATE_LABEL (numberPVs,     68, Qt::AlignRight,    " - ");
      if (archType == QEArchiveAccess::CA) {
         CREATE_LABEL (pending,       68, Qt::AlignRight,    " - ");
      }

      this->vLayout->addWidget (row->frame);

      row->frame->setVisible (false);
   }

   this->vLayout->addStretch ();

#undef CREATE_LABEL

}

//---------------------------------------------------------------------------------
//
void QEArchiveStatus::calcMinimumHeight ()
{
   int count;
   int delta_top;

   // Allow +1 for titles.
   //
   count = this->inUseCount + 1;

   delta_top = 20;
   this->setMinimumHeight ((delta_top * count) + 24);
}

//------------------------------------------------------------------------------
//
QEArchiveStatus::QEArchiveStatus (QWidget* parent) : QEGroupBox (parent)
{
   this->createInternalWidgets();

   const QEArchiveAccess::ArchiverTypes archType = QEArchiveAccess::getArchiverType();
   switch (archType) {
      case QEArchiveAccess::CA:
         this->setTitle (" Channel Archiver Host Status ");
         break;
      case QEArchiveAccess::ARCHAPPL:
         this->setTitle (" Archive Appliance Host Status ");
         break;
      case QEArchiveAccess::Error:
         this->setTitle (" Archive Status Summary ");
         break;
   }

   this->inUseCount = 0;

   QObject::connect (this->archiveAccess,
                     SIGNAL     (archiveStatus (const QEArchiveAccess::StatusList&)),
                     this, SLOT (archiveStatus (const QEArchiveAccess::StatusList&)));

   this->calcMinimumHeight ();
   this->setMinimumWidth (776);

   // This info re-emitted on change, but we need to stimulate an initial update.
   //
   this->archiveAccess->resendStatus ();
}

//------------------------------------------------------------------------------
//
QEArchiveStatus::~QEArchiveStatus ()
{
}

//------------------------------------------------------------------------------
//
QSize QEArchiveStatus::sizeHint () const
{
   return QSize (776, 64);   // two rows
}

//------------------------------------------------------------------------------
//
void QEArchiveStatus::reReadAvailablePVs ()
{
   if (this->archiveAccess) this->archiveAccess->reReadAvailablePVs ();
}

//------------------------------------------------------------------------------
//
void QEArchiveStatus::archiveStatus (const QEArchiveAccess::StatusList& statusList)
{
   this->inUseCount = statusList.count ();
   this->calcMinimumHeight ();

   for (int j = 0; j < QEArchiveStatus::NumberRows; j++ ) {
      QEArchiveStatus::Rows* row = &this->rowList [j];

      if (j <  statusList.count ()) {
         QEArchiveAccess::Status state = statusList.value (j);

         row->hostNamePort->setText (QString ("%1:%2").arg (state.hostName).arg (state.portNumber));
         row->endPoint->setText (state.endPoint);
         row->state->setText (QEUtilities::enumToString(QEArchapplInterface::staticMetaObject, QString("States"), state.state));
         row->numberPVs->setText (QString ("%1").arg (state.numberPVs));

         if (archiveAccess->getArchiverType() == QEArchiveAccess::CA) {
            row->available->setText (QString ("%1").arg (state.available));
            row->read->setText (QString ("%1").arg (state.read));
            row->pending->setText (QString ("%1").arg (state.pending));
         }



         row->frame->setVisible (true);
      } else {
         row->frame->setVisible (false);
      }
   }
}

// end
