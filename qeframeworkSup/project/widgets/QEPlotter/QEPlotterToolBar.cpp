/*  QEPlotterToolBar.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2013-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include <QDebug>
#include <QIcon>
#include <QString>
#include <QVariant>

#include <QECommon.h>
#include "QEPlotterToolBar.h"
#include <QEPlotterMenu.h>

// Structure used in buttonSpecs for definining plotter tool bar.
// Note, a similar structure is used in QEStripChart. If they are the same name a
// strange problem occurs when built with GCC 4.5.1 where the QString destructor is
// called inappropriately on exit causing a crash.
//
struct QEPlotterPushButtonSpecifications {
   int gap;
   int width;
   bool isIcon;                         // when false is caption
   const QString captionOrIcon;         // caption text or iocn filename - not full path
   QEPlotterNames::MenuActions action;  // associated action
   const QString toolTip;
};

#define NO_SLOT            0
#define GAP                8          // group gap
#define ICW                26         // icon width

static const struct QEPlotterPushButtonSpecifications buttonSpecs [] = {

   { 0,   ICW, true,  QString ("go_back.png"),       QEPlotterNames::PLOTTER_PREV,               QString ("Previous state")     },
   { 0,   ICW, true,  QString ("go_fwd.png"),        QEPlotterNames::PLOTTER_NEXT,               QString ("Next state")         },

   { GAP, ICW, true,  QString ("normal_video.png"),  QEPlotterNames::PLOTTER_NORMAL_VIDEO,       QString ("White background")   },
   { 0,   ICW, true,  QString ("reverse_video.png"), QEPlotterNames::PLOTTER_REVERSE_VIDEO,      QString ("Black background")   },

   { GAP, ICW, true,  QString ("linear_scale.png"),  QEPlotterNames::PLOTTER_LINEAR_Y_SCALE,     QString ("Linear Y Scale")     },
   { 0,   ICW, true,  QString ("log_scale.png"),     QEPlotterNames::PLOTTER_LOG_Y_SCALE,        QString ("Log Y Scale")        },
   { 0,   ICW, false, QString ("My"),                QEPlotterNames::PLOTTER_MANUAL_Y_RANGE,     QString ("Manual Y Scale")     },
   { 0,   ICW, false, QString ("Ay"),                QEPlotterNames::PLOTTER_CURRENT_Y_RANGE,    QString ("Y Data Range Scale") },
   { 0,   ICW, false, QString ("Dy"),                QEPlotterNames::PLOTTER_DYNAMIC_Y_RANGE,    QString ("Dynamic Y Scale")    },
   { 0,   ICW, false, QString ("N"),                 QEPlotterNames::PLOTTER_NORAMLISED_Y_RANGE, QString ("Noramalised Scale")  },
   { 0,   ICW, false, QString ("F"),                 QEPlotterNames::PLOTTER_FRACTIONAL_Y_RANGE, QString ("Fractional Scale")   },

   { GAP, ICW, true,  QString ("linear_scale.png"),  QEPlotterNames::PLOTTER_LINEAR_X_SCALE,     QString ("Linear X Scale")     },
   { 0,   ICW, true,  QString ("log_scale.png"),     QEPlotterNames::PLOTTER_LOG_X_SCALE,        QString ("Log X Scale")        },
   { 0,   ICW, false, QString ("Mx"),                QEPlotterNames::PLOTTER_MANUAL_X_RANGE,     QString ("Manual X Scale")     },
   { 0,   ICW, false, QString ("Ax"),                QEPlotterNames::PLOTTER_CURRENT_X_RANGE,    QString ("X Data Range Scale") },
   { 0,   ICW, false, QString ("Dx"),                QEPlotterNames::PLOTTER_DYNAMIC_X_RANGE,    QString ("Dynamic X Scale")    },

   { GAP, ICW, false, QString ("M"),                 QEPlotterNames::PLOTTER_MANUAL_XY_RANGE,    QString ("Manual X & Y Scale") },

   { GAP, ICW, true,  QString ("play.png"),          QEPlotterNames::PLOTTER_PLAY,               QString ("Play - Real time")   },
   { 0,   ICW, true,  QString ("pause.png"),         QEPlotterNames::PLOTTER_PAUSE,              QString ("Pause"),             },

   { GAP, ICW, true,  QString ("open_file.png"),     QEPlotterNames::PLOTTER_LOAD_CONFIG,        QString ("Load Configuration...") },
   { 0,   ICW, true,  QString ("save_file.png"),     QEPlotterNames::PLOTTER_SAVE_CONFIG,        QString ("Save Configuration...") }
};


//==============================================================================
//
QEPlotterToolBar::QEPlotterToolBar (QWidget *parent) : QFrame (parent)
{
   int left;
   int j;
   QPushButton *button;
   QString iconPathName;
   int gap;

   this->setFixedHeight (this->designHeight);

   // Clear buttom action map
   //
   this->buttonActionMap.clear ();

   // Create toobar buttons
   // TODO: Try QToolBar - it may auto layout.
   //
   left = 4;
   for (j = 0; j < ARRAY_LENGTH (buttonSpecs); j++) {

      button = new QPushButton (this);

      // Set up icon or caption text.
      //
      if (buttonSpecs[j].isIcon) {
         // NOTE:  This widget used the same icon files as the strip chart.
         iconPathName = ":/qe/stripchart/";
         iconPathName.append (buttonSpecs[j].captionOrIcon);
         button->setIcon (QIcon (iconPathName));
      } else {
         button->setText (buttonSpecs[j].captionOrIcon);
      }

      button->setFocusPolicy (Qt::NoFocus);
      button->setToolTip (QString(" %1 ").arg (buttonSpecs[j].toolTip));
      gap = buttonSpecs[j].gap;
      button->setGeometry (left + gap, 2, buttonSpecs[j].width, 26);
      left += gap + buttonSpecs[j].width + 2;

      QObject::connect (button, SIGNAL (clicked       (bool)),
                        this,   SLOT   (buttonClicked (bool)));

      // save two-way reference
      //
      this->buttonActionMap.insertF (button, buttonSpecs[j].action);
   }
}

//------------------------------------------------------------------------------
//
QEPlotterToolBar::~QEPlotterToolBar ()
{
   // no special action - place holder
}

//------------------------------------------------------------------------------
//
void QEPlotterToolBar::resizeEvent (QResizeEvent *)
{
    // place holder
}

//------------------------------------------------------------------------------
//
void QEPlotterToolBar::setEnabled (QEPlotterNames::MenuActions action, const bool value)
{
   QPushButton* button = this->buttonActionMap.valueI (action, NULL);
   if (button) {
      button->setEnabled (value);
   }
}

//------------------------------------------------------------------------------
//
void QEPlotterToolBar::buttonClicked (bool)
{
   QPushButton* button = dynamic_cast <QPushButton *> (this->sender ());
   if (button) {
      if (this->buttonActionMap.containsF (button)) {
         QEPlotterNames::MenuActions action = this->buttonActionMap.valueF (button);
         emit this->selected (action, NO_SLOT);
      }
   }
}

// end
