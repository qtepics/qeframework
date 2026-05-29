/*  QEResizeableFrame.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2013-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEResizeableFrame.h"
#include <QDebug>
#include <QMouseEvent>
#include <QRect>
#include <QString>

#include <QECommon.h>
#include <QEPlatform.h>

#define DEBUG qDebug () << "QEResizeableFrame" << __LINE__ << __FUNCTION__ << " "

static const QString passive ("QWidget { background-color: #a0c0e0; }");  // blue-ish
static const QString hover   ("QWidget { background-color: #ffffff; }");  // white-ish
static const QString active  ("QWidget { background-color: #e0c0a0; }");  // orange-ish

//------------------------------------------------------------------------------
//
QEResizeableFrame::QEResizeableFrame (QWidget *parent) : QEFrame (parent)
{
   this->setup (BottomEdge, 10, 100);
}

//------------------------------------------------------------------------------
//
QEResizeableFrame::QEResizeableFrame (GrabbingEdges grabbingEdge,
                                      int minimumIn,
                                      int maximumIn,
                                      QWidget *parent) : QEFrame (parent)
{
   this->setup (grabbingEdge, minimumIn, maximumIn);
}

//------------------------------------------------------------------------------
//
QEResizeableFrame::~QEResizeableFrame ()
{
   // place holder
}

//------------------------------------------------------------------------------
//
void QEResizeableFrame::setup (GrabbingEdges grabbingEdgeIn, int minimumIn, int maximumIn)
{
   // Set super class pro[ertioes.
   //
   this->setMinimumSize (16, 16);
   this->setFrameShape (QFrame::Box);
   this->setFrameShadow (QFrame::Plain);

   this->userWidget = NULL;
   this->grabber = NULL;
   this->layout = NULL;
   this->defaultWidget = new QWidget (NULL);

   this->grabbingEdge = grabbingEdgeIn;
   this->allowedMin = MAX (minimumIn, 8);
   this->allowedMax = MAX (maximumIn, this->allowedMin);
   this->isActive = false;
   this->noMoreDebug = false;

   this->resetEgde ();
}

//------------------------------------------------------------------------------
//
void QEResizeableFrame::resetEgde ()
{
   if (this->layout) delete this->layout;
   this->layout = NULL;

   if (this->grabber) delete this->grabber;
   this->grabber = NULL;

   this->grabber = new QWidget (this);
   if (this->isVertical()) {
      this->grabber->setCursor (QCursor (Qt::SizeVerCursor));
      this->grabber->setFixedHeight (4);
      this->layout = new QVBoxLayout (this);
   } else {
      this->grabber->setCursor (QCursor (Qt::SizeHorCursor));
      this->grabber->setFixedWidth (4);
      this->layout = new QHBoxLayout (this);
   }

   this->grabber->setStyleSheet (passive);
   this->grabber->setMouseTracking (true);
   this->grabber->installEventFilter (this);   // Use self as the event filter object.
   this->grabber->setToolTip ("");

   this->layout->setContentsMargins (1, 1, 1, 1);
   this->layout->setSpacing (1);

   this->setWidget (this->userWidget);
}

//------------------------------------------------------------------------------
//
void QEResizeableFrame::setGrabbingEdge (const GrabbingEdges edge)
{
   if (this->grabbingEdge != edge) {
      this->grabbingEdge = edge;
      this->resetEgde ();
   }
}

//------------------------------------------------------------------------------
//
QEResizeableFrame::GrabbingEdges QEResizeableFrame::getGrabbingEdge () const
{
   return this->grabbingEdge;
}

//------------------------------------------------------------------------------
//
QWidget *QEResizeableFrame::widget() const
{
   return this->userWidget;
}

//------------------------------------------------------------------------------
//
void QEResizeableFrame::setWidget (QWidget* widgetIn)
{
   QWidget* workingWidget = NULL;

   // Sanity check.
   //
   if (widgetIn) {
      // Cannot set self as widget nor the current widget nor the default widget.
      //
      if ((widgetIn == this) ||
          (widgetIn == this->userWidget) ||
          (widgetIn == this->defaultWidget)) {
         // Here be dragons - cannot set to this etc.
         //
         DEBUG << "unexpected widget parameter";
         return;
      }
   }

   // First clear all widgets (if any) from the layout.
   //
   this->layout->removeWidget (this->grabber);

   this->layout->removeWidget (this->defaultWidget);
   this->defaultWidget->setParent (NULL);

   // If there is user widget then remove and delete it is well.
   //
   if (this->userWidget) {
      this->layout->removeWidget (this->userWidget);
      delete this->userWidget;
      this->userWidget = NULL;
   }

   // Now we can start putting it back together again.
   //
   this->userWidget = widgetIn;

   // Did user specify a widget - if not use default widget.
   //
   if (this->userWidget) {
      workingWidget = this->userWidget;
   } else {
      workingWidget = this->defaultWidget;
   }

   // Reparent userWidget/defaultWidget.
   //
   workingWidget->setParent (this);

   // Ensure user widget resizeable.
   // Actual size is controlled by allowedMin and allowedMax.
   // designer deafults to 16777215
   //
   if (this->isVertical()) {
      workingWidget->setMinimumHeight (0);
      workingWidget->setMaximumHeight (8000);
   } else {
      workingWidget->setMinimumWidth (0);
      workingWidget->setMaximumWidth (8000);
   }

   switch (this->grabbingEdge) {
      case BottomEdge:
      case RightEdge:
         this->layout->addWidget (workingWidget);
         this->layout->addWidget (this->grabber);
         break;

      case TopEdge:
      case LeftEdge:
         this->layout->addWidget (this->grabber);
         this->layout->addWidget (workingWidget);
         break;
   }
}

//------------------------------------------------------------------------------
//
QWidget *QEResizeableFrame::takeWidget ()
{
   QWidget *result = this->userWidget;

   if (result) {
      // It exists. Remove it from the layout and then from this widget as parent
      // and clear the reference. It is now total dis-entangled from this.
      //
      this->layout->removeWidget (result);
      result->setParent (NULL);
      this->userWidget = NULL;

      // Reset layout with default widget
      //
      this->setWidget (NULL);
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEResizeableFrame::setGrabberToolTip (const QString& tip)
{
   this->grabber->setToolTip (tip);
}

//------------------------------------------------------------------------------
//
void QEResizeableFrame::applyLimits ()
{
   int currentSize;  // width/height

   if (this->isVertical()) {
      currentSize = this->geometry().height ();
   } else {
      currentSize = this->geometry().width ();
   }

   int allowedSize = LIMIT (currentSize, this->allowedMin, this->allowedMax);

   if (this->isVertical()) {
      this->setFixedHeight (allowedSize);
   } else {
      this->setFixedWidth (allowedSize);
   }
}

//------------------------------------------------------------------------------
//
void QEResizeableFrame::setAllowedMinimum (const int minimumIn)
{
   this->allowedMin = MAX (minimumIn, 8);
   // Ensure allowed max >= allowed min.
   this->allowedMax = MAX (this->allowedMax, this->allowedMin);
   this->applyLimits ();
}

//------------------------------------------------------------------------------
//
int QEResizeableFrame::getAllowedMinimum () const
{
   return this->allowedMin;
}

//------------------------------------------------------------------------------
//
void QEResizeableFrame::setAllowedMaximum (const int maximumIn)
{
   // Ensure allowed max >= allowed min.
   this->allowedMax = MAX (maximumIn, this->allowedMin);
   this->applyLimits ();
}

//------------------------------------------------------------------------------
//
int QEResizeableFrame::getAllowedMaximum () const
{
   return this->allowedMax;
}

//------------------------------------------------------------------------------
//
bool QEResizeableFrame::isVertical ()
{
   return ((this->grabbingEdge == BottomEdge) || (this->grabbingEdge == TopEdge));
}

//------------------------------------------------------------------------------
//
void QEResizeableFrame::processMouseMove (const int x, const int y)
{
   int delta;
   int current;
   int modified;

   switch (this->grabbingEdge) {

      case TopEdge:
         delta = y - 2;
         current = this->geometry().height ();
         modified = LIMIT (current - delta, this->allowedMin, this->allowedMax);
         this->setFixedHeight (modified);
         break;

      case LeftEdge:
         delta = x - 2;
         current = this->geometry().width ();
         modified = LIMIT (current - delta, this->allowedMin, this->allowedMax);
         this->setFixedWidth (modified);
         break;

      case BottomEdge:
         delta = y - 2;
         current = this->geometry().height ();
         modified = LIMIT (current + delta, this->allowedMin, this->allowedMax);
         this->setFixedHeight (modified);
         break;

      case RightEdge:
         delta = x - 2;
         current = this->geometry().width ();
         modified = LIMIT (current + delta, this->allowedMin, this->allowedMax);
         this->setFixedWidth (modified);
         break;

      default:
         if (!this->noMoreDebug) {
            DEBUG << "Unexpected edge " << this->objectName ();
         }
         this->noMoreDebug = true;
         break;
   }
}

//------------------------------------------------------------------------------
//
bool QEResizeableFrame::eventFilter (QObject* watched, QEvent* event)
{
   QMouseEvent * mouseEvent = NULL;

   // case on type first else we get a seg fault.
   //
   switch (event->type ()) {

      case QEvent::MouseButtonPress:
         if (watched == this->grabber) {
            this->isActive = true;
            this->grabber->setStyleSheet (active);
            return true;
         }
         break;

      case QEvent::MouseButtonRelease:
         if (watched == this->grabber) {
            this->isActive = false;
            this->grabber->setStyleSheet (passive);
            return true;
         }
         break;


      case QEvent::MouseMove:
         mouseEvent = static_cast<QMouseEvent *> (event);
         if (watched == this->grabber) {
            // if Actived then stay Activated otherwise ...
            if (this->isActive) {
               QPoint pos = QEPlatform::positionOf (mouseEvent);
               this->processMouseMove (pos.x(), pos.y());
            } else {
               this->grabber->setStyleSheet (hover);
            }
            return true;
         }
         break;

      case QEvent::Leave:
         if (watched == this->grabber) {
            this->grabber->setStyleSheet (passive);
         }
         break;

      default:
         // Just fall through
         break;
   }

   // standard event processing
   //
   return QObject::eventFilter (watched, event);
}

// end
