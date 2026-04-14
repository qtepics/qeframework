/*  QEShape.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2009-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

/*
  This class is a CA aware shape widget based on QWidget.
  It is tighly integrated with the base class QEWidget. Refer to QEWidget.cpp for details
 */

#include "QEShape.h"
#include <QPainter>
#include <QPainterPath>
#include <QEScaling.h>
#include <QCaDateTime.h>

/*
    Create without a known variable. Just manage parental hirarchy.
*/
QEShape::QEShape (QWidget *parent):
   QWidget (parent),
   QEWidget (this)
{
   setup ();
}

/*
    Create with a known variable. Subscription occurs immedietly.
*/
QEShape::QEShape (const QString& variableNameIn, QWidget *parent):
   QWidget (parent),
   QEWidget (this)
{
   // Call common setup code.
   setup ();

   // Use the variable name as the first variable
   setVariableName (variableNameIn, 0);
   activate ();
}

/*
    Common setup code.
*/
void QEShape::setup ()
{
   // Set up data
   setMinimumSize (16, 16);

   // Set the number of variables equal to the base VariableNameManager is to manage
   setNumVariables (QESHAPE_NUM_VARIABLES);

   // Set up default properties
   setAllowDrop (false);

   shape = Rect;
   setAutoFillBackground (false);

   antialiased = false;

   setBackgroundRole (QPalette::NoRole);

   originTranslation = QPoint (0, 0);
   scaledOriginTranslation = originTranslation;

   points[0] = QPoint (0, 0);
   points[1] = QPoint (width () - 1, this->height () - 1);
   unsigned int i;
   for (i = 2; i < POINTS_SIZE; i++)
      points[i] = QPoint (0, 0);

   numPoints = 2;

   colors[0] = QColor (255, 0, 0);
   colors[1] = QColor (0, 255, 0);
   colors[2] = QColor (0, 0, 255);
   colors[3] = QColor (255, 255, 255);
   colors[4] = QColor (0, 0, 0);

   for (i = 0; i < QESHAPE_NUM_VARIABLES; i++)
      lastValue[i] = 0.0;

   for (i = 0; i < SCALES_SIZE; i++)
      scales[i] = 1.0;


   for (i = 0; i < OFFSETS_SIZE; i++)
      offsets[i] = 0.0;

   animations[0] = Width;
   animations[1] = Height;
   animations[2] = X;
   animations[3] = Y;
   animations[4] = Transperency;
   animations[5] = ColourHue;

   painterCurrentScaleX = 1.0;
   painterCurrentScaleY = 1.0;
   painterCurrentTranslateX = 0;
   painterCurrentTranslateY = 0;
   rotation = 0.0;
   startAngle = 0.0;

   drawBorder = true;

   lineWidth = 1;
   pen.setWidth (lineWidth);

   fill = true;

   brush.setStyle (Qt::SolidPattern);
   currentColor = 0;
   brush.setColor (colors[currentColor]);

   // Set the initial state
   isConnected = false;
   setSizePolicy (QSizePolicy::Fixed, QSizePolicy::Fixed);

   // Use standard context menu
   setupContextMenu ();

   // Use widget signals
   // !! move this functionality into QEWidget???
   // !! needs one for single variables and one for multiple variables, or just the multiple variable one for all
   // for each variable name property manager, set up an index to identify it when it signals and
   // set up a connection to recieve variable name property changes.
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   for (int i = 0; i < QESHAPE_NUM_VARIABLES; i++) {
      QCaVariableNamePropertyManager* vnpm = &variableNamePropertyManagers[i];
      vnpm->setVariableIndex (i);
      QObject::connect (vnpm, SIGNAL (newPvNameProperties (const QEPvNameProperties&)),
                        this, SLOT   (usePvNameProperties (const QEPvNameProperties&)));
   }
}

/*
    Set up size hint
 */
QSize QEShape::sizeHint () const
{
   return QSize (16, 16);
}

/*
   Scaling drawing parameters.
   Note overall widget size, min size, max size (and font) have already been scaled.
 */
void QEShape::scaleBy (const int m, const int d)
{
   double ratio;
   int i;

   // Sainity check - must avoid zero scaling and divide by zero.
   //
   if ((m < 1) || (d < 1))
      return;

   ratio = (double) m / (double) d;

   for (i = 0; i < SCALES_SIZE; i++)
      scales[i] *= ratio;

   for (i = 0; i < OFFSETS_SIZE; i++)
      offsets[i] *= ratio;

   QEScaling::applyToPoint (scaledOriginTranslation);
   QEScaling::applyToPoint (originTranslation);

   for (i = 0; i < POINTS_SIZE; i++)
      QEScaling::applyToPoint (points[i]);
}

/*
    Implementation of QEWidget's virtual funtion to create the specific type of QEChannel required.
    For a shape a QEChannel that streams integers is required.
*/
QEChannel * QEShape::createQcaItem (unsigned int variableIndex)
{
   // Create the item as a QEInteger.
   //
   const QString pvName = this->getSubstitutedVariableName (variableIndex);
   return new QEInteger (pvName, this, &this->integerFormatting, variableIndex);
}

/*
    Start updating.
    Implementation of VariableNameManager's virtual funtion to establish a connection to a PV as the variable name has changed.
    This function may also be used to initiate updates when loaded as a plugin.
*/
void QEShape::establishConnection (unsigned int variableIndex)
{
   // Create a connection.
   // If successfull, the QEChannel object that will supply data update signals will be returned.
   //
   QEChannel * qca = createConnection (variableIndex);

   // If a QEChannel object is now available to supply data update signals,
   // connect it to the appropriate slots.
   //
   if (qca) {
      // setValue( 0, QCaAlarmInfo(), QCaDateTime(), variableIndex );
      // ??? should this be moved up before the create connection?
      // If create connection fails, then any previous data should be cleared?
      // Do for all types if required.
      //
      QObject::connect (qca,  SIGNAL (valueUpdated (const QEIntegerValueUpdate&)),
                        this, SLOT   (setValue     (const QEIntegerValueUpdate&)));

      QObject::connect (qca,  SIGNAL (connectionUpdated (const QEConnectionUpdate&)),
                        this, SLOT   (connectionUpdated (const QEConnectionUpdate&)));
   }
}

/*
    Act on a connection change.
    Change how the label looks and change the tool tip
    This is the slot used to recieve connection updates from a QEChannel based class.
 */
void QEShape::connectionUpdated (const QEConnectionUpdate& update)
{
   const unsigned int vi = update.variableIndex;

   // Note the connected state.
   //
   this->isConnected = update.connectionInfo.isChannelConnected ();

   // Display the connected state.
   //
   this->updateToolTipConnection (this->isConnected, vi);
   this->processConnectionInfo (this->isConnected, vi);
}

/*
    Use a data update to alter the shape's attributes.
    The name setValue is less appropriate for this QE widgets than for others
    such as QELabel where setValue() sets the value displayed. For this
    widget setting the value means modifying one attribute such as position or
    color.
    This is the slot used to recieve data updates from a QEChannel based class.
*/
void QEShape::setValue (const QEIntegerValueUpdate& update)
{
   const unsigned int vi = update.variableIndex;

   // Signal a database value change to any Link widgets.
   //
   switch (vi) {
      case 0:
         emit dbValueChanged1 (update.value);
         break;
      case 1:
         emit dbValueChanged2 (update.value);
         break;
      case 2:
         emit dbValueChanged3 (update.value);
         break;
      case 3:
         emit dbValueChanged4 (update.value);
         break;
      case 4:
         emit dbValueChanged5 (update.value);
         break;
      case 5:
         emit dbValueChanged6 (update.value);
         break;
      default:
         sendMessage ("Application error: Unexpected variable index",
                      "QEShape.cpp QEShape::setValue()",
                      message_types (MESSAGE_TYPE_ERROR));
   }

   // Save the value (for copy).
   //
   this->lastValue[vi] = update.value;

   // Scale the data.
   // For example, a flow of 0 to 10 l/m may adjust a shape size 0 to 200 pixels.
   // Questions: why don't we use QEFloating?
   //
   const double dval = static_cast<double>(update.value);
   const double scaledValue = dval * this->scales[vi] + this->offsets[vi];

   // Animate the object.
   // Apply the data to the appropriate attribute of the shape.
   //
   switch (this->animations[vi]) {
      case Width:
         {
            painterCurrentScaleX = scaledValue;
            break;
         }
      case Height:
         {
            painterCurrentScaleY = scaledValue;
            break;
         }
      case X:
         {
            painterCurrentTranslateX = scaledValue;
            break;
         }
      case Y:
         {
            painterCurrentTranslateY = scaledValue;
            break;
         }
      case Transperency:
         {
            int newAlpha = 0;
            if (scaledValue >= 0.0) {
               if (scaledValue <= 255) {
                  newAlpha = (unsigned int) scaledValue;
               } else {
                  newAlpha = 255;
               }
            }
            int h, s, v, a;
            QColor color;
            color = brush.color ();
            color.getHsv (&h, &s, &v, &a);
            a = newAlpha;
            color.setHsv (h, s, v, a);
            brush.setColor (color);
            break;
         }
      case Rotation:
         {
            rotation = scaledValue;
            break;
         }
      case ColourHue:
         {
            int newHue = 0;
            if (scaledValue >= 0.0) {
               if (scaledValue <= 255) {
                  newHue = (unsigned int) scaledValue;
               } else {
                  newHue = 255;
               }
            }
            int h, s, v, a;
            QColor color;
            color = brush.color ();
            color.getHsv (&h, &s, &v, &a);
            h = newHue;
            color.setHsv (h, s, v, a);
            brush.setColor (color);
            break;
         }
      case ColourSaturation:
         {
            int newSaturation = 0;
            if (scaledValue >= 0.0) {
               if (scaledValue <= 255) {
                  newSaturation = (unsigned int) scaledValue;
               } else {
                  newSaturation = 255;
               }
            }
            int h, s, v, a;
            QColor color;
            color = brush.color ();
            color.getHsv (&h, &s, &v, &a);
            s = newSaturation;
            color.setHsv (h, s, v, a);
            brush.setColor (color);
            break;
         }
      case ColourValue:
         {
            int newValue = 0;
            if (scaledValue >= 0.0) {
               if (scaledValue <= 255) {
                  newValue = (unsigned int) scaledValue;
               } else {
                  newValue = 255;
               }
            }
            int h, s, v, a;
            QColor color;
            color = brush.color ();
            color.getHsv (&h, &s, &v, &a);
            v = newValue;
            color.setHsv (h, s, v, a);
            brush.setColor (color);
            break;
         }
      case ColourIndex:
         {
            if (scaledValue >= 0.0) {
               if (scaledValue < COLORS_SIZE) {
                  currentColor = (unsigned int) scaledValue;
               } else {
                  currentColor = COLORS_SIZE - 1;
               }
            }
            brush.setColor (colors[currentColor]);
            break;
         }
      case Penwidth:
         {
            pen.setWidth ((int) scaledValue);
            break;
         }
   }

   // Invoke common alarm handling processing.
   //
   this->processAlarmInfo (update.alarmInfo, vi);

   // Force the shape to be redrawn
   // This is why I alwys use this.
   //
   this->update ();
}


/*
    Update date the PV names and subbstitutions.
 */
void QEShape::usePvNameProperties (const QEPvNameProperties& pvNameProperties)
{
   this->setVariableNameAndSubstitutions (pvNameProperties.pvName,
                                          pvNameProperties.substitutions,
                                          pvNameProperties.index);
}

/*
    Draw the shape.
    This is called whenever the applications forces a redraw using update()
    after data modifies some attribute of the shape, or whenever the windowing
    system decides the object requires drawing, such as when the window
    containing the shape widget is scrolled into view.
*/
void QEShape::paintEvent (QPaintEvent * /* event */ )
{
   QPainter painter (this);

   switch (shape) {
      case Line:
      case Points:
         pen.setColor (brush.color ());
         break;

      default:
         pen.setColor (QColor (0, 0, 0));
         break;
   }

   // Set up the pen and brush (color, thickness, etc)
   pen.setWidth (lineWidth);
   painter.setPen (pen);
   painter.setBrush (brush);

   // Draw everything with antialiasing
   painter.setRenderHint (QPainter::Antialiasing, true);

   // Alter the viewport according to the origin translation properties.
   QRect viewportRect = painter.viewport ();
   viewportRect.moveLeft (originTranslation.x ());
   viewportRect.moveTop (originTranslation.y ());
   painter.setWindow (viewportRect);

   // Apply the current translation, scaling and rotation
   painter.translate (painterCurrentTranslateX + 0.5, painterCurrentTranslateY + 0.5);
   painter.scale (painterCurrentScaleX, painterCurrentScaleY);
   painter.rotate (rotation);

   // Draw the shape
   switch (shape) {
      case Line:
         painter.drawLine (points[0], points[1]);
         break;
      case Points:
         painter.drawPoints (points, numPoints);
         break;
      case Polyline:
         if (!drawBorder)
            painter.setPen (Qt::NoPen);
         if (!fill)
            painter.setBrush (Qt::NoBrush);
         painter.drawPolyline (points, numPoints);
         break;
      case Polygon:
         if (!drawBorder)
            painter.setPen (Qt::NoPen);
         if (!fill)
            painter.setBrush (Qt::NoBrush);
         painter.drawPolygon (points, numPoints);
         break;
      case Rect:
         if (!drawBorder)
            painter.setPen (Qt::NoPen);
         if (!fill)
            painter.setBrush (Qt::NoBrush);
         painter.drawRect (points[0].x (), points[0].y (), points[1].x (), points[1].y ());
         break;
      case RoundedRect:
         if (!drawBorder)
            painter.setPen (Qt::NoPen);
         if (!fill)
            painter.setBrush (Qt::NoBrush);
#if QT_VERSION >= 0x040400
         painter.drawRoundedRect (points[0].x (), points[0].y (), points[1].x (),
                                  points[1].y (), 25, 25, Qt::RelativeSize);
#else
         painter.drawRect (points[0].x (), points[0].y (), points[1].x (), points[1].y ());
#endif
         break;
      case Ellipse:
         if (!drawBorder)
            painter.setPen (Qt::NoPen);
         if (!fill)
            painter.setBrush (Qt::NoBrush);
         painter.drawEllipse (points[0].x (), points[0].y (), points[1].x (), points[1].y ());
         break;
      case Arc:
         painter.drawArc (points[0].x (), points[0].y (), points[1].x (), points[1].y (),
                          (int) (startAngle * 16), (int) (arcLength * 16));
         break;
      case Chord:
         if (!drawBorder)
            painter.setPen (Qt::NoPen);
         if (!fill)
            painter.setBrush (Qt::NoBrush);
         painter.drawChord (points[0].x (), points[0].y (), points[1].x (), points[1].y (),
                            (int) (startAngle * 16), (int) (arcLength * 16));
         break;
      case Pie:
         if (!drawBorder)
            painter.setPen (Qt::NoPen);
         if (!fill)
            painter.setBrush (Qt::NoBrush);
         painter.drawPie (points[0].x (), points[0].y (), points[1].x (), points[1].y (),
                          (int) (startAngle * 16), (int) (arcLength * 16));
         break;
      case Path:
         if (!drawBorder)
            painter.setPen (Qt::NoPen);
         if (!fill)
            painter.setBrush (Qt::NoBrush);
         {
            QPainterPath path;
            path.moveTo (points[0]);
            path.cubicTo (points[1], points[2], points[3]);
            painter.drawPath (path);
         }
         break;
   }
}

/*
   Reset the brush color if the color the brush is using is changing
 */
void QEShape::colorChange (unsigned int index)
{
   // Sanity check. Ignore out of range color index
   if (index >= COLORS_SIZE)
      return;

   // Update the brush and redraw the shape if the color being changed is in use
   if (currentColor == index) {
      brush.setColor (colors[currentColor]);
      update ();
   }
}

//==============================================================================
// Drag and Drop
void QEShape::setDrop (QVariant drop)
{
   QStringList PVs = drop.toString ().split (' ');
   for (int i = 0; i < PVs.size () && i < QESHAPE_NUM_VARIABLES; i++) {
      setVariableName (PVs[i], i);
      establishConnection (i);
   }
}

QVariant QEShape::getDrop ()
{
   if (isDraggingVariable ())
      return QVariant (copyVariable ());
   else
      return copyData ();
}

//==============================================================================
// Copy / Paste
QString QEShape::copyVariable ()
{
   QString text;
   for (int i = 0; i < QESHAPE_NUM_VARIABLES; i++) {
      QString pv = getSubstitutedVariableName (i);
      if (!pv.isEmpty ()) {
         if (!text.isEmpty ())
            text.append (" ");
         text.append (pv);
      }
   }

   return text;
   //    return getSubstitutedVariableName(0);
}

QVariant QEShape::copyData ()
{
   QString text;
   for (int i = 0; i < QESHAPE_NUM_VARIABLES; i++) {
      text.append (QString ("%1\t").arg (lastValue[i]));
   }

   return QVariant (text);
}

void QEShape::paste (QVariant v)
{
   if (getAllowDrop ()) {
      setDrop (v);
   }
}

//==============================================================================

void QEShape::refreshData (const int index)
{
   QEChannel * qca = getQcaItem (index);
   if (qca) {
      qca->resendLastData ();
   }
}

//==============================================================================
// Property convenience functions

// variable animations
void QEShape::setAnimation (QEShape::animationOptions animation, const int index)
{
   animations[index] = animation;
   update ();
}

QEShape::animationOptions QEShape::getAnimation (const int index) const
{
   return animations[index];
}

// scales
void QEShape::setScale (const double scale, const int index)
{
   scales[index] = scale;
   refreshData (index);
}

double QEShape::getScale (const int index) const
{
   return scales[index];
}


// offsets
void QEShape::setOffset (const double offset, const int index)
{
   offsets[index] = offset;
   refreshData (index);
}

double QEShape::getOffset (const int index) const
{
   return offsets[index];
}

// shape
void QEShape::setShape (QEShape::shapeOptions shapeIn)
{
   // Refresh the data so the new shape can be redrawn
   shape = shapeIn;
   for (int i = 0; i < QESHAPE_NUM_VARIABLES; i++) {
      refreshData (i);
   }

   // Force an update.
   // Note, this is not needed if new data arrives following the refresh requested above, but is usefull if there is no (valid) data set up yet
   update ();
}

QEShape::shapeOptions QEShape::getShape () const
{
   return shape;
}


// number of points
void QEShape::setNumPoints (unsigned int numPointsIn)
{
   numPoints = (numPointsIn > POINTS_SIZE) ? POINTS_SIZE : numPointsIn;
   update ();
}

unsigned int QEShape::getNumPoints () const
{
   return numPoints;
}

// Origin translation
void QEShape::setOriginTranslation (const QPoint& originTranslationIn)
{
   originTranslation = originTranslationIn;
   update ();
}

QPoint QEShape::getOriginTranslation () const
{
   return originTranslation;
}

// points
void QEShape::setPoint (const QPoint& point, const int index)
{
   points[index] = point;
   update ();
}

QPoint QEShape::getPoint (const int index) const
{
   return points[index];
}

// colors
void QEShape::setColor (const QColor& color, const int index)
{
   colors[index] = color;
   colorChange (index);
}

QColor QEShape::getColor (const int index) const
{
   return colors[index];
}

// draw border
void QEShape::setDrawBorder (bool drawBorderIn)
{
   drawBorder = drawBorderIn;
   update ();
}

bool QEShape::getDrawBorder () const
{
   return drawBorder;
}

// fill
void QEShape::setFill (bool fillIn)
{
   fill = fillIn;
   update ();
}

bool QEShape::getFill () const
{
   return fill;
}

// line width
void QEShape::setLineWidth (unsigned int lineWidthIn)
{
   lineWidth = lineWidthIn;
   update ();
}

unsigned int QEShape::getLineWidth () const
{
   return lineWidth;
}

// start angle
void QEShape::setStartAngle (double startAngleIn)
{
   startAngle = startAngleIn;
   update ();
}

double QEShape::getStartAngle () const
{
   return startAngle;
}

// rotation
void QEShape::setRotation (double rotationIn)
{
   rotation = rotationIn;
   update ();
}

double QEShape::getRotation () const
{
   return rotation;
}

// arc length
void QEShape::setArcLength (double arcLengthIn)
{
   arcLength = arcLengthIn;
   update ();
}

double QEShape::getArcLength () const
{
   return arcLength;
}

// end
