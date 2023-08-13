/*  QEStripChartToolBar.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2023 Australian Synchrotron
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The EPICS QT Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY { } without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with the EPICS QT Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *
 */

#include "QEStripChartToolBar.h"
#include "QEStripChartUtilities.h"

#include <QDebug>
#include <QComboBox>
#include <QFile>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QString>
#include <QTextStream>
#include <QVariant>

#include <QECommon.h>
#include <QCaDateTime.h>
#include <QEScaling.h>
#include <QEAdaptationParameters.h>

#define DEBUG qDebug () << "QEStripChartToolBar:" << __LINE__ << __FUNCTION__ << " "

#define NUMBER_OF_BUTTONS  34
#define ICW                26         // standard icon width
#define ICH                26         // standard icon height
#define DBW                28         // duration button width
#define VALUE_PROPERTY     "QE_STRIPCHART_BUTTON_VALUE"

// Special slots NUMBERS  - must be consistent with the buttonSpecs array declared below
//
#define PREV_SLOT          0
#define NEXT_SLOT          1
#define YSCALE_SLOT        6
#define TSCALE_SLOT        12
#define ARCHIVE_SLOT       28
#define PLAY_SLOT          30

#define STANDARD_TAB       0
#define CONFIGURATION_TAB  1
#define MARKERS_TAB        2
#define NUMBER_OF_TABS     3


#define SELECT_FILE        "--- select configuration file ---"

// Structure used in buttonSpecs for definining strip chart tool bar.
// Note, a similar structure is used in QEPlotter. If they are the same name a
// strange problem occurs when built with GCC 4.5.1 where the QString destructor is
// called inappropriately on exit causing a crash.
//
struct QEStripChartPushButtonSpecifications {
   int gap;
   int width;
   int value;
   bool isIcon;  // when false is caption
   const QString captionOrIcon;
   const QString toolTip;
   const char* member;
};

static const QString localZone = QEUtilities::getTimeZoneTLA (Qt::LocalTime, QDateTime::currentDateTime ());

static const char* hoprLopr = "Notional Operational Range\n(encompass all LOPR/HOPR values)";

static const struct QEStripChartPushButtonSpecifications buttonSpecs [NUMBER_OF_BUTTONS] = {
   { 0,   ICW, 0, true,  QString ("go_back.png"),           QString ("Previous state"),               SLOT (prevStateClicked (bool))        },
   { 0,   ICW, 0, true,  QString ("go_fwd.png"),            QString ("Next state"),                   SLOT (nextStateClicked (bool))        },

   { 8,   ICW, 0, true,  QString ("normal_video.png"),      QString ("White background"),             SLOT (normalVideoClicked (bool))      },
   { 0,   ICW, 0, true,  QString ("reverse_video.png"),     QString ("Black background"),             SLOT (reverseVideoClicked (bool))     },

   { 8,   ICW, 0, true,  QString ("linear_scale.png"),      QString ("Linear scale"),                 SLOT (linearScaleClicked (bool))      },
   { 0,   ICW, 0, true,  QString ("log_scale.png"),         QString ("Log Scale"),                    SLOT (logScaleClicked (bool))         },

   { 8,   ICW, 0, false, QString ("M"),                     QString ("Manual Scale"),                 SLOT (manualYScaleClicked (bool))     },
   { 0,   ICW, 0, false, QString ("A"),                     QString (hoprLopr),                       SLOT (automaticYScaleClicked (bool))  },
   { 0,   ICW, 0, false, QString ("P"),                     QString ("Plotted Data Scale"),           SLOT (plottedYScaleClicked (bool))    },
   { 0,   ICW, 0, false, QString ("B"),                     QString ("Buffer Data Scale"),            SLOT (bufferedYScaleClicked (bool))   },
   { 0,   ICW, 0, false, QString ("D"),                     QString ("Dynamic Scale"),                SLOT (dynamicYScaleClicked (bool))    },
   { 0,   ICW, 0, false, QString ("N"),                     QString ("Normalised Scale"),             SLOT (normalisedYScaleClicked (bool)) },

   { 8,   DBW, 60,     false, QString ("1m"),               QString ("Select chart duration"),        SLOT (duration2Clicked (bool))        },
   { 0,   DBW, 120,    false, QString ("2m"),               QString ("Select chart duration"),        SLOT (duration2Clicked (bool))        },
   { 0,   DBW, 300,    false, QString ("5m"),               QString ("Select chart duration"),        SLOT (duration2Clicked (bool))        },
   { 0,   DBW, 600,    false, QString ("10m"),              QString ("Select chart duration"),        SLOT (duration2Clicked (bool))        },
   { 0,   DBW, 1200,   false, QString ("20m"),              QString ("Select chart duration"),        SLOT (duration2Clicked (bool))        },
   { 0,   DBW, 1800,   false, QString ("30m"),              QString ("Select chart duration"),        SLOT (duration2Clicked (bool))        },
   { 0,   DBW, 3600,   false, QString ("1h"),               QString ("Select chart duration"),        SLOT (duration2Clicked (bool))        },
   { 0,   DBW, 7200,   false, QString ("2h"),               QString ("Select chart duration"),        SLOT (duration2Clicked (bool))        },
   { 0,   DBW, 14400,  false, QString ("4h"),               QString ("Select chart duration"),        SLOT (duration2Clicked (bool))        },
   { 0,   DBW, 21600,  false, QString ("6h"),               QString ("Select chart duration"),        SLOT (duration2Clicked (bool))        },
   { 0,   DBW, 43200,  false, QString ("12h"),              QString ("Select chart duration"),        SLOT (duration2Clicked (bool))        },
   { 0,   DBW, 86400,  false, QString ("1d"),               QString ("Select chart duration"),        SLOT (duration2Clicked (bool))        },
   { 0,   DBW, 172800, false, QString ("2d"),               QString ("Select chart duration"),        SLOT (duration2Clicked (bool))        },
   { 0,   ICW, 0, true, QString ("select_time.png"),        QString ("Select chart duration"),        SLOT (selectDurationClicked (bool))   },

   { 8,   40,  0, false, localZone,                         QString ("Use local time"),               SLOT (localTimeClicked (bool))        },
   { 0,   40,  0, false, QString ("UTC"),                   QString ("Use UTC (GMT) time"),           SLOT (utcTimeClicked (bool))          },

   { 8,   ICW, 0, true,  QString ("archive.png"),           QString ("Extract data from archive(s)"), SLOT (readArchiveClicked (bool))      },

   { 8,   ICW, 0, true,  QString ("select_date_times.png"), QString ("Set chart start/end time"),     SLOT (selectTimeClicked (bool))       },
   { 0,   ICW, 0, true,  QString ("play.png"),              QString ("Play - Real time"),             SLOT (playClicked (bool))             },
   { 0,   ICW, 0, true,  QString ("pause.png"),             QString ("Pause"),                        SLOT (pauseClicked (bool))            },
   { 0,   ICW, 0, true,  QString ("page_backward.png"),     QString ("Back one page"),                SLOT (backwardClicked (bool))         },
   { 0,   ICW, 0, true,  QString ("page_forward.png"),      QString ("Forward one page"),             SLOT (forwardClicked (bool))          },
};


//==============================================================================
// QEStripChartToolBar::OwnTabWidget
//==============================================================================
//
class QEStripChartToolBar::OwnTabWidget : public QTabWidget {
public:
   explicit OwnTabWidget (QEStripChartToolBar* parent);
   ~OwnTabWidget ();

   // Standard tab
   //
   QPushButton* pushButtons [NUMBER_OF_BUTTONS];
   QLabel* yScaleStatus;
   QLabel* timeStatus;
   QLabel* durationStatus;
   QLabel* numberOfOutandingRequests;
   QLabel* timeModeStatus;

   // Configuration tab
   //
   QComboBox* predefinedComboBox;
   QPushButton* loadButton;
   QPushButton* saveAsButton;

   // Markers tab
   //
   QLabel* timeRefLabel;
   QLabel* time1;
   QLabel* time2;
   QLabel* timeDeltaLabel;
   QLabel* timeDelta;

   QLabel* valueRefLabel;
   QLabel* value1;
   QLabel* value2;
   QLabel* valueDelta1;
   QLabel* value3;
   QLabel* value4;
   QLabel* valueDelta2;

private:
   QEStripChartToolBar* owner;
   int originalPointSize;
   int reducedPointSize;

   // Conveniance QLabel contructor.
   // Need text, parent, geometry and optional enhancement.
   // Note: ax is modified, and becomes  ax + aw + 4
   //
   QLabel* createLabel (const QString& text, QWidget* parent,
                        int& ax, const int ay, const int aw, const int ah,
                        const bool enhance = false);
};

//------------------------------------------------------------------------------
//
QEStripChartToolBar::OwnTabWidget::OwnTabWidget (QEStripChartToolBar* parent) : QTabWidget (parent)
{
   const int buttonTop = 4;
   const int labelTop = buttonTop + 26;
   const int labelHeight = 16;

   QWidget* tabParent = NULL;
   int left;
   int top;
   QPushButton* button;
   QString iconPathName;
   int gap;

   this->owner = parent;  // save parent reference as QEStripChartToolBar

   // Do smaller (80%) standard font, enhanced is original size.
   //
   QFont font = this->font ();
   this->originalPointSize = font.pointSize ();
   this->reducedPointSize = (this->originalPointSize * 4) / 5;
   font.setPointSize (this->reducedPointSize);
   this->setFont (font);

   // The default tab height is dervied from font size, but is too small. The setFixedHeight
   // function does not work as expected when when appled to the tabBar, but setting the
   // style does do the job.
   //
   const int tabHeight = QEScaling::scale (20);
   const QString tabBarStyle = QString ("QTabBar::tab { height: %1px }").arg (tabHeight);
   this->tabBar()->setStyleSheet (tabBarStyle);

   this->setDocumentMode (true);  // remove un-needed boarders.
   this->setTabPosition (QTabWidget::South);
   this->setTabShape (QTabWidget::Triangular);

   // Create required tabs.
   //
   this->addTab (new QWidget (this), QObject::tr(" Standard "));
   this->addTab (new QWidget (this), QObject::tr(" Configuration "));
   this->addTab (new QWidget (this), QObject::tr(" Markers "));

   // Create toolbar widgets
   //
   tabParent = this->widget (STANDARD_TAB);
   left = 4;
   for (int j = 0 ; j < NUMBER_OF_BUTTONS; j++) {
      button = new QPushButton (tabParent);
      button->setFont (font);    // set reduced fontsize.

      // Set up icon or caption text.
      //
      if (buttonSpecs[j].isIcon) {
         iconPathName = ":/qe/stripchart/";
         iconPathName.append (buttonSpecs[j].captionOrIcon);
         button->setIcon (QIcon (iconPathName));
      } else {
         button->setText (QObject::tr(buttonSpecs[j].captionOrIcon.toStdString().c_str()));
      }

      button->setFocusPolicy (Qt::NoFocus);
      button->setProperty (VALUE_PROPERTY, buttonSpecs[j].value);
      button->setToolTip (QObject::tr(buttonSpecs[j].toolTip.toStdString().c_str()));
      gap = buttonSpecs[j].gap;
      button->setGeometry (left + gap, buttonTop, buttonSpecs[j].width, ICH);  // left top width height
      left += gap + buttonSpecs[j].width;
      if (buttonSpecs[j].member != NULL) {
         QObject::connect (button, SIGNAL (clicked (bool)),
                           this->owner, buttonSpecs[j].member);
      }

      this->pushButtons [j] = button;
   }

   // Set up status labels.
   //
   left = this->pushButtons [YSCALE_SLOT]->geometry().x ();
   this->yScaleStatus = createLabel ("", tabParent, left, labelTop, 6*ICW, labelHeight);
   this->yScaleStatus->setAlignment (Qt::AlignHCenter);

   left = this->pushButtons [TSCALE_SLOT]->geometry().x ();
   this->timeStatus = createLabel ("", tabParent, left, labelTop, 328, labelHeight);
   this->durationStatus = createLabel ("", tabParent, left, labelTop, 84, labelHeight);
   this->durationStatus->setAlignment (Qt::AlignRight);

   left = this->pushButtons [ARCHIVE_SLOT]->geometry().x ();
   this->numberOfOutandingRequests = createLabel("0", tabParent, left, labelTop, ICW, labelHeight);
   this->numberOfOutandingRequests->setAlignment (Qt::AlignHCenter);
   this->numberOfOutandingRequests->setToolTip (QObject::tr(" Number of outstanding archive requests "));

   left = this->pushButtons [PLAY_SLOT]->geometry().x ();
   this->timeModeStatus = createLabel ("", tabParent, left, labelTop, 3*ICW, labelHeight);
   this->timeModeStatus->setAlignment (Qt::AlignHCenter);


   // Set up config tab place holder
   //
   tabParent = this->widget (CONFIGURATION_TAB);

   left = 4;
   this->predefinedComboBox = new QComboBox (tabParent);
   this->predefinedComboBox->setGeometry (left, 5, 600, 23);  // left top width height
   font = this->predefinedComboBox->font ();
   font.setPointSize (this->originalPointSize);
   this->predefinedComboBox->setFont (font);
   this->predefinedComboBox->setToolTip (QObject::tr(" Select and load predefined chart configuration "));
   QObject::connect (this->predefinedComboBox, SIGNAL (activated (QString)),
                     this->owner,       SLOT (predefinedSelected (QString)));

   QEAdaptationParameters ap ("QE_");
   QString predefinedFilename = ap.getString ("stripchart_configurations", "");
   if (!predefinedFilename.isEmpty()) {
      QFile predefinedFile (predefinedFilename);
      if (predefinedFile.open (QIODevice::ReadOnly | QIODevice::Text)) {
         this->predefinedComboBox->addItem (SELECT_FILE);
         QTextStream source (&predefinedFile);
         while (!source.atEnd()) {
            QString line = source.readLine ().trimmed ();

            // Skip empty line and comment lines.
            //
            if (line.length () == 0) continue;
            if (line.left (1) == "#") continue;

            this->predefinedComboBox->addItem (line);
         }
      }
   }

   left += (600 + 4);
   this->loadButton = new QPushButton (tabParent);
   this->loadButton->setIcon (QIcon (":/qe/stripchart/open_file.png"));
   this->loadButton->setFocusPolicy (Qt::NoFocus);
   this->loadButton->setToolTip (QObject::tr(" Load chart configuration "));
   this->loadButton->setGeometry (left, buttonTop, ICW, ICH);  // left top width height
   QObject::connect (this->loadButton, SIGNAL (clicked (bool)),
                     this->owner, SLOT (loadClicked (bool)));

   left += (ICW + 4);
   this->saveAsButton = new QPushButton (tabParent);
   this->saveAsButton->setIcon (QIcon (":/qe/stripchart/save_file.png"));
   this->saveAsButton->setFocusPolicy (Qt::NoFocus);
   this->saveAsButton->setToolTip (QObject::tr(" Save chart configuration "));
   this->saveAsButton->setGeometry (left, buttonTop, ICW, ICH);  // left top width height
   QObject::connect (this->saveAsButton, SIGNAL (clicked (bool)),
                     this->owner, SLOT (saveAsClicked (bool)));


   // Set up markers tab.
   //
   tabParent = this->widget (MARKERS_TAB);

   top = 4;   // first row
   left = 4;
   this->timeRefLabel =   createLabel (QObject::tr("Time References"), tabParent, left, top, 108, labelHeight);   // left top width height
   this->time1 =          createLabel ("", tabParent, left, top, 224, labelHeight, true);
   this->time2 =          createLabel ("", tabParent, left, top, 224, labelHeight, true);
   left += 12;
   this->timeDeltaLabel = createLabel (QObject::tr("Delta Time"), tabParent, left, top, 72, labelHeight);
   this->timeDelta =      createLabel ("", tabParent, left, top, 160, labelHeight, true);

   top = 24;   // second row
   left = 4;
   this->valueRefLabel =  createLabel (QObject::tr("Value References"), tabParent, left, top, 108, labelHeight);
   this->value1 =         createLabel ("", tabParent, left,  top, 140, labelHeight, true);
   this->value2 =         createLabel ("", tabParent, left,  top, 140, labelHeight, true);
   this->valueDelta1 =    createLabel ("", tabParent, left,  top, 140, labelHeight, true);
   left += 12;
   this->value3 =         createLabel ("", tabParent, left,  top, 140, labelHeight, true);
   this->value4 =         createLabel ("", tabParent, left,  top, 140, labelHeight, true);
   this->valueDelta2 =    createLabel ("", tabParent, left,  top, 140, labelHeight, true);
}

//------------------------------------------------------------------------------
//
QEStripChartToolBar::OwnTabWidget::~OwnTabWidget ()
{
  // no special action - place holder
}

//------------------------------------------------------------------------------
//
QLabel* QEStripChartToolBar::OwnTabWidget::createLabel (const QString& text, QWidget* parent,
                                                        int& ax, const int ay, const int aw, const int ah,
                                                        const bool enhance)
{
   QLabel* result = new QLabel (text, parent);
   result->setGeometry (ax, ay, aw, ah);    // left top width height
   ax += (aw + 4);  // move along

   // Has enhancement been specified ?
   //
   QFont font = result->font ();
   if (enhance) {
      // Yes - use larger font size and set background.
      font.setPointSize (this->originalPointSize);
      result->setStyleSheet ("QLabel { background-color: #e8e8e8; }");
   } else {
      // No - use smaller font size.
      font.setPointSize (this->reducedPointSize);
   }
   result->setFont (font);
   result->setIndent (6);

   return result;
}


//==============================================================================
// Local functions
//==============================================================================
// Consider moveing to QEUtilities in QECommon
//
static QString floatToString (const double value, const int precision)
{
   const double av = ABS (value);
   const int magnitude = int (LOG10 (av) + 1.0);
   const double low_fixed_limit = EXP10 (2 - precision);
   const double high_fixed_limit = EXP10 (precision - 2);

   QString result;

   // The troub. with 'g' mode is that it does not honour precision.
   //
   if (av == 0.0) {
      result = QString ("%1").arg (value, 0, 'f', precision - 1);
   } else if ((av >= low_fixed_limit) && (av <= high_fixed_limit)) {
      result = QString ("%1").arg (value, 0, 'f', precision - magnitude);
   } else {
      result = QString ("%1").arg (value, 0, 'e', precision);
   }

   return result;
}


//==============================================================================
// QEStripChartToolBar functions
//==============================================================================
// static
int QEStripChartToolBar::designHeight ()
{
   // Required height found emperically.
   //
   return 66;
}

//------------------------------------------------------------------------------
//
QEStripChartToolBar::QEStripChartToolBar (QWidget* parent) : QWidget (parent)
{
   this->ownTabWidget = new OwnTabWidget (this);

   QRect tabGeo (0, 0, this->geometry().width(), QEStripChartToolBar::designHeight () - 1);
   this->ownTabWidget->setGeometry (tabGeo);
}

//------------------------------------------------------------------------------
//
QEStripChartToolBar::~QEStripChartToolBar ()
{
   // no special action - place holder
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::resizeEvent (QResizeEvent *)
{
   // Resize inner QTabWidget - we don't use a layout - these don't do what we want.
   // Note: original ownWidgets height unchanged, and we do not use designHeight as we
   // did during contruction as the widget may have since been scaled.
   //
   QRect tabGeo (0, 0, this->geometry().width(), this->ownTabWidget->geometry().height());
   this->ownTabWidget->setGeometry (tabGeo);
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::setYRangeStatus (const QEStripChartNames::ChartYRanges yRange)
{
   this->ownTabWidget->yScaleStatus->setText (QEStripChartNames::chartYRangeStatus (yRange));
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::setTimeStatus (const QString& timeStatusIn)
{
   this->ownTabWidget->timeStatus->setText (timeStatusIn);
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::setDurationStatus (const QString& durationStatusIn)
{
   this->ownTabWidget->durationStatus->setText (durationStatusIn);
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::setNOARStatus (const int noarIn)
{
   QString image = QString::number (noarIn);
   this->ownTabWidget->numberOfOutandingRequests->setText (image);
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::setTimeModeStatus (const QEStripChartNames::ChartTimeModes timeMode)
{
   this->ownTabWidget->timeModeStatus->setText (QEStripChartNames::chartTimeModeStatus (timeMode));
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::setStateSelectionEnabled (const QEStripChartNames::StateModes mode, const bool enabled)
{
   switch (mode) {
      case QEStripChartNames::previous:
         this->ownTabWidget->pushButtons [PREV_SLOT]->setEnabled (enabled);
         break;

      case QEStripChartNames::next:
         this->ownTabWidget->pushButtons [NEXT_SLOT]->setEnabled (enabled);
         break;
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::setTimeRefs (const QDateTime& t1, const QDateTime& t2)
{
   QString zone = QEUtilities::getTimeZoneTLA (t2);
   QString format = "yyyy-MM-dd hh:mm:ss.zzz";   // include milli seconds.

   double delta = QCaDateTime (t1).secondsTo (t2);
   this->ownTabWidget->time1->setText (QString ("%1 %2").arg (t1.toString (format)).arg (zone));
   this->ownTabWidget->time2->setText (QString ("%1 %2").arg (t2.toString (format)).arg (zone));
   this->ownTabWidget->timeDelta->setText (QEUtilities::intervalToString (delta, 3, true));
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::setValue1Refs (const double v1, const double v2)
{
   const double dv = v2 - v1;
   this->ownTabWidget->value1->setText (floatToString (v1, 8));
   this->ownTabWidget->value2->setText (floatToString (v2, 8));
   this->ownTabWidget->valueDelta1->setText (floatToString (dv, 8));
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::setValue2Refs (const double v1, const double v2)
{
   const double dv = v2 - v1;
   this->ownTabWidget->value3->setText (floatToString (v1, 8));
   this->ownTabWidget->value4->setText (floatToString (v2, 8));
   this->ownTabWidget->valueDelta2->setText (floatToString (dv, 8));
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::duration2Clicked (bool)
{
   QPushButton* button = dynamic_cast <QPushButton*> (this->sender ());
   if (button) {
      int d;
      bool okay;
      d = button->property(VALUE_PROPERTY).toInt(&okay);
      if (okay) {
         emit this->durationSelected (d);
      }
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::selectDurationClicked (bool)
{
   emit this->selectDuration ();
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::prevStateClicked (bool)
{
   emit this->stateSelected (QEStripChartNames::previous);
}

void QEStripChartToolBar::nextStateClicked (bool)
{
   emit this->stateSelected (QEStripChartNames::next);
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::normalVideoClicked (bool)
{
   emit this->videoModeSelected (QEStripChartNames::normal);
}

void QEStripChartToolBar::reverseVideoClicked (bool)
{
   emit this->videoModeSelected (QEStripChartNames::reverse);
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::linearScaleClicked (bool)
{
   this->yScaleModeSelected (QEStripChartNames::linear);
}

void QEStripChartToolBar::logScaleClicked (bool)
{
   this->yScaleModeSelected (QEStripChartNames::log);
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::manualYScaleClicked (bool)
{
   emit this->yRangeSelected (QEStripChartNames::manual);
}

void QEStripChartToolBar::automaticYScaleClicked (bool)
{
   emit this->yRangeSelected (QEStripChartNames::operatingRange);
}

void QEStripChartToolBar::plottedYScaleClicked (bool)
{
   emit this->yRangeSelected (QEStripChartNames::plotted);
}

void QEStripChartToolBar::bufferedYScaleClicked (bool)
{
   emit this->yRangeSelected (QEStripChartNames::buffered);
}

void QEStripChartToolBar::dynamicYScaleClicked (bool)
{
   emit this->yRangeSelected (QEStripChartNames::dynamic);
}

void QEStripChartToolBar::normalisedYScaleClicked (bool)
{
   emit this->yRangeSelected (QEStripChartNames::normalised);
}


//------------------------------------------------------------------------------
//
void QEStripChartToolBar::playClicked (bool)
{
   emit this->playModeSelected (QEStripChartNames::play);
}

void QEStripChartToolBar::pauseClicked (bool)
{
   emit this->playModeSelected (QEStripChartNames::pause);
}

void QEStripChartToolBar::forwardClicked (bool)
{
   emit this->playModeSelected (QEStripChartNames::forward);
}

void QEStripChartToolBar::backwardClicked (bool)
{
   emit this->playModeSelected (QEStripChartNames::backward);
}

void QEStripChartToolBar::selectTimeClicked (bool)
{
   emit this->playModeSelected (QEStripChartNames::selectTimes);
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::readArchiveClicked (bool)
{
    emit this->readArchiveSelected ();
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::localTimeClicked (bool)
{
   emit this->timeZoneSelected (Qt::LocalTime);
}

void  QEStripChartToolBar::utcTimeClicked (bool)
{
   emit this->timeZoneSelected (Qt::UTC);
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::predefinedSelected (QString filename)
{
   if (filename != SELECT_FILE) {
      emit this->loadSelectedFile (filename);
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartToolBar::loadClicked (bool)
{
   emit this->loadSelected ();
}

void QEStripChartToolBar::saveAsClicked (bool)
{
   emit this->saveAsSelected ();
}

// end
