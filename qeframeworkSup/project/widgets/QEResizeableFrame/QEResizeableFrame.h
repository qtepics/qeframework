/*  QEResizeableFrame.h
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
 *  Copyright (c) 2013 Australian Synchrotron
 */

#ifndef QE_RESIZEABLE_FRAME_H
#define QE_RESIZEABLE_FRAME_H

#include <QEvent>
#include <QObject>
#include <QWidget>
#include <QVBoxLayout>

#include <QEFrame.h>
#include <QEFrameworkLibraryGlobal.h>

/// The QEResizeableFrame provides a frame capable of holding another widget
/// together with a grabber widget that allows the frame to be re-sized, and
/// hence contained widget to be resized. The class currently only supports
/// vertical or horizontal resizing, but not both.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEResizeableFrame : public QEFrame {
    Q_OBJECT
public:
   enum GrabbingEdges { TopEdge, LeftEdge, BottomEdge, RightEdge };
   Q_ENUMS (GrabbingEdges)

   /// Nominated edge for the grabbing location. Defaults to BottomEdge.
   ///
   Q_PROPERTY (GrabbingEdges grabbingEdge  READ getGrabbingEdge  WRITE setGrabbingEdge)

   /// Set the minimium allowd size (defaults to 10).
   ///
   Q_PROPERTY (int  allowedMinimum  READ getAllowedMinimum  WRITE setAllowedMinimum)

   /// Set the maximium allowd size (defaults to 100).
   ///
   Q_PROPERTY (int  allowedMaximum  READ getAllowedMaximum  WRITE setAllowedMaximum)

public:
   explicit QEResizeableFrame (QWidget *parent = 0);

   /// Construct widget specifying min and max allowed heights.
   explicit QEResizeableFrame (GrabbingEdges grabbingEdge, int minimum, int maximum, QWidget *parent = 0);
   ~QEResizeableFrame ();

   // This modelled on QScrollArea
   //
   /// Returns a ref to the resizeable frame's widget, or 0 if there is none.
   QWidget *widget() const;

   /// Sets the resizeable frame's widget.
   /// The widget becomes a child of the resizeable frame, and will be destroyed when
   /// the resizeable frame is deleted or when a new widget is set.
   /// Any existing widget is deleted - use takeWidget first if needs be.
   void setWidget (QWidget *widget);

   /// Removes the resizeable frame's widget, and passes ownership management of the widget to the caller.
   QWidget *takeWidget();

   /// Set the tool tip for the internal grabber object.
   void setGrabberToolTip (const QString & tip);

   /// (Re)set allowed limits.
   void setAllowedMinimum (const int minimum);
   int getAllowedMinimum () const;

   void setAllowedMaximum (const int maximum);
   int getAllowedMaximum () const;

   void setGrabbingEdge (const GrabbingEdges edge);
   GrabbingEdges getGrabbingEdge () const;

protected:
   bool eventFilter (QObject *obj, QEvent *event);

private:
   void applyLimits ();
   void setup (GrabbingEdges grabbingEdge, int minimum, int maximum);
   void resetEgde ();
   bool isVertical ();
   void processMouseMove (const int x, const int y);
   bool isActive;
   bool noMoreDebug;

   // No slots or signals - this should be safe, as in moc won't get confused.
   //
   QBoxLayout* layout;
   QWidget* userWidget;
   QWidget* grabber;
   QWidget* defaultWidget;

   GrabbingEdges grabbingEdge;

   // We can't use widget's min/maximumHeight values to store these as we call setFixedHeight
   // to the frame height.
   //
   int allowedMin;
   int allowedMax;
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEResizeableFrame::GrabbingEdges)
#endif

#endif  // QE_RESIZEABLE_FRAME_H
