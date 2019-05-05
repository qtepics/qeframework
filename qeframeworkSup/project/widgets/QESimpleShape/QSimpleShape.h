/*  QSimpleShape.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2014-2019 Australian Synchrotron.
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
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef Q_SIMPLE_SHAPE_H
#define Q_SIMPLE_SHAPE_H

#include <QString>
#include <QVector>
#include <QWidget>

#include <QEFrameworkLibraryGlobal.h>
#include <QEScanTimers.h>

/*!
  This class is an basic simple shape widget.
  It has been refactored from QESimpleShape and will become the bases
  for QESimpleShape.
*/
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QSimpleShape : public QWidget  {
   Q_OBJECT
public:
   // QSimpleShape specific properties ==============================================
   //
   enum Shapes { circle, ellipse, rectangle, roundRectangle, roundSquare, square,
                 triangleUp, triangleDown, triangleLeft, triangleRight,
                 triangleTopRight, triangleBottomRight, triangleBottomLeft, triangleTopLeft,
                 diamond, equalDiamond, arrowUp, arrowDown, arrowLeft, arrowRight,
                 crossHorizontal, crossVertical, hexagon, octogon,
                 snakeHorizontal, snakeVertical, pentagon, star, plus, cross,
                 roundpie, pie };
   Q_ENUMS (Shapes)

   /// Nominated shape
   ///
   Q_PROPERTY (Shapes shape       READ getShape       WRITE setShape)

   /// Edge width - range 0 to 20 - default is 1.
   /// If edge width set to 0, then shape colour used for edge/boarder colour.
   Q_PROPERTY (int    edgeWidth   READ getEdgeWidth   WRITE setEdgeWidth)

   /// Edge style - default is solid line
   Q_PROPERTY (Qt::PenStyle edgeStyle  READ getEdgeStyle     WRITE setEdgeStyle)

   /// Number of semi-cycles - range 1 to 30 - default is 8.
   /// Only applies to serpentine items.
   Q_PROPERTY (int    semiCycles  READ getSemiCycles  WRITE setSemiCycles)

   /// Breadth of serpentine line as a percentage of widget height or width
   /// depending on wheather horizontal or vertical orientatiion.
   /// Range 1 to 50, default 10%
   /// Only applies to serpentine items.
   Q_PROPERTY (int    percentSize READ getPercentSize WRITE setPercentSize)

   /// The pie centre angle in degrees.
   /// Zero represents a vertical centerline and angles increment clockwise.
   Q_PROPERTY (int    centreAngle READ getCentreAngle WRITE setCentreAngle)

   /// The span of the pie in degrees
   Q_PROPERTY (int    spanAngle   READ getSpanAngle   WRITE setSpanAngle)

   /// Shape value - range 0 to 15 - default is zero.
   Q_PROPERTY (int    value       READ getValue       WRITE setValue)

   /// Shape value modulus - range 2 to 16 - default is 16.
   Q_PROPERTY (int    modulus     READ getModulus     WRITE setModulus)

   //----------------------------------------------------------------------------------
   // Note: PvText and LocalEnumeration are a nod to QESimpleShape. LocalEnumeration has
   // been kept to help maintain compatibility. For PvText/LocalEnumeration formats,
   // the virtual getItemText/getItemColour functions are used to determine.
   //
   enum TextFormats { FixedText,           ///< Use user specified fixed text (default)
                      StateSet,            ///< Use one of the stae Set values
                      PvText,              ///< Use EPICS value agumented with units if selected.
                      LocalEnumeration };  ///< Use specied enumeration values, PV value used to select item
   Q_ENUMS (TextFormats)

   /// Nominated text format
   ///
   Q_PROPERTY (TextFormats format    READ getTextFormat WRITE setTextFormat)

   /// User specified text - defaults to null string.
   Q_PROPERTY (QString     fixedText READ getFixedText  WRITE setFixedText)

   /// Text values used when format is LocalEnumeration.
   Q_PROPERTY (QStringList stateSet  READ getStateSet    WRITE setStateSet)

   /// Flash rate.
   /// The default value is Medium, i.e. 1Hz flash rate.
   Q_PROPERTY (QEScanTimers::ScanRates flashRate READ getFlashRate WRITE setFlashRate)

   /// When inActive, the widgers is grayed-out, as if disabled. The default is true.
   Q_PROPERTY (bool    isActive         READ getIsActive         WRITE setIsActive)

   // Group various colours together.
   // There order here is the order shown in designer.
   //
   /// Edge colour - default is black
   Q_PROPERTY (QColor edgeColour     READ getEdgeColour      WRITE setEdgeColour)

   /// Flash offColour - default is clear.
   Q_PROPERTY (QColor flashOffColour READ getFlashOffColour  WRITE setFlashOffColour)

   /// Value (modulo modulus) used to select colour.
   Q_PROPERTY (QColor  colour0    READ getColour0Property        WRITE setColour0Property)
   Q_PROPERTY (QColor  colour1    READ getColour1Property        WRITE setColour1Property)
   Q_PROPERTY (QColor  colour2    READ getColour2Property        WRITE setColour2Property)
   Q_PROPERTY (QColor  colour3    READ getColour3Property        WRITE setColour3Property)
   Q_PROPERTY (QColor  colour4    READ getColour4Property        WRITE setColour4Property)
   Q_PROPERTY (QColor  colour5    READ getColour5Property        WRITE setColour5Property)
   Q_PROPERTY (QColor  colour6    READ getColour6Property        WRITE setColour6Property)
   Q_PROPERTY (QColor  colour7    READ getColour7Property        WRITE setColour7Property)
   Q_PROPERTY (QColor  colour8    READ getColour8Property        WRITE setColour8Property)
   Q_PROPERTY (QColor  colour9    READ getColour9Property        WRITE setColour9Property)
   Q_PROPERTY (QColor  colour10   READ getColour10Property       WRITE setColour10Property)
   Q_PROPERTY (QColor  colour11   READ getColour11Property       WRITE setColour11Property)
   Q_PROPERTY (QColor  colour12   READ getColour12Property       WRITE setColour12Property)
   Q_PROPERTY (QColor  colour13   READ getColour13Property       WRITE setColour13Property)
   Q_PROPERTY (QColor  colour14   READ getColour14Property       WRITE setColour14Property)
   Q_PROPERTY (QColor  colour15   READ getColour15Property       WRITE setColour15Property)

   //----------------------------------------------------------------------------------
   /// When the widget's state coresponds to N and flashN is set true, the widget will
   /// alternate its normal colour with the specified flashOffColour.
   ///
   Q_PROPERTY (bool    flash0     READ getFlash0Property         WRITE setFlash0Property)
   Q_PROPERTY (bool    flash1     READ getFlash1Property         WRITE setFlash1Property)
   Q_PROPERTY (bool    flash2     READ getFlash2Property         WRITE setFlash2Property)
   Q_PROPERTY (bool    flash3     READ getFlash3Property         WRITE setFlash3Property)
   Q_PROPERTY (bool    flash4     READ getFlash4Property         WRITE setFlash4Property)
   Q_PROPERTY (bool    flash5     READ getFlash5Property         WRITE setFlash5Property)
   Q_PROPERTY (bool    flash6     READ getFlash6Property         WRITE setFlash6Property)
   Q_PROPERTY (bool    flash7     READ getFlash7Property         WRITE setFlash7Property)
   Q_PROPERTY (bool    flash8     READ getFlash8Property         WRITE setFlash8Property)
   Q_PROPERTY (bool    flash9     READ getFlash9Property         WRITE setFlash9Property)
   Q_PROPERTY (bool    flash10    READ getFlash10Property        WRITE setFlash10Property)
   Q_PROPERTY (bool    flash11    READ getFlash11Property        WRITE setFlash11Property)
   Q_PROPERTY (bool    flash12    READ getFlash12Property        WRITE setFlash12Property)
   Q_PROPERTY (bool    flash13    READ getFlash13Property        WRITE setFlash13Property)
   Q_PROPERTY (bool    flash14    READ getFlash14Property        WRITE setFlash14Property)
   Q_PROPERTY (bool    flash15    READ getFlash15Property        WRITE setFlash15Property)

   //
   // End of QSimpleShape specific properties =====================================

public:
   /// Construction
   ///
   QSimpleShape (QWidget* parent = 0);

   /// Destruction
   virtual ~QSimpleShape () {}

   // Property functions.
   //
public slots:
   void setValue (const bool value);
   void setValue (const int value);

public:
   int getValue () const;

   void setModulus (const int value);
   int getModulus () const;

   void setStateSet (const QStringList& stateSet);
   QStringList getStateSet () const;

public slots:
   // Allows shape to be selected by a signal from another widget. 
   void setShape (const int value);
public:
   void setShape (const Shapes value);
   Shapes getShape () const;

   void setEdgeWidth (const int value);
   int getEdgeWidth () const;

   void setSemiCycles (const int value);
   int getSemiCycles () const;

   void setPercentSize (const int value);
   int getPercentSize () const;

   void setCentreAngle (const int angle);
   int getCentreAngle () const;

   void setSpanAngle (const int span);
   int getSpanAngle () const;

   void setEdgeStyle (const Qt::PenStyle penStyle);
   Qt::PenStyle getEdgeStyle () const;

   void setEdgeColour (const QColor value);
   QColor getEdgeColour () const;

   void setTextFormat (const TextFormats value);
   TextFormats getTextFormat () const;

   void setFixedText (const QString& value);
   QString getFixedText () const;

   void setFlashRate (const QEScanTimers::ScanRates);
   QEScanTimers::ScanRates getFlashRate () const;

   void setFlashOffColour (const QColor colour);
   QColor getFlashOffColour () const;

   void setIsActive (const bool value);
   bool getIsActive () const;

   void   setColourProperty (const int slot, const QColor color);
   QColor getColourProperty (const int slot) const;

   void  setFlashProperty (int slot, const bool isFlashing);
   bool  getFlashProperty (int slot) const;

   // Property access READ and WRITE functions.
   // We can define the access functions using a macro.
   // Alas, due to SDK limitation, we cannot embedded the property definition itself in a macro.
   //
#define PROPERTY_ACCESS(slot)                                                                          \
   void   setColour##slot##Property (QColor colour) { this->setColourProperty (slot, colour); }        \
   QColor getColour##slot##Property () const { return this->getColourProperty (slot); }                \
   \
   void  setFlash##slot##Property (bool flash)    { this->setFlashProperty (slot, flash); }            \
   bool  getFlash##slot##Property () const { return this->getFlashProperty (slot); }

   PROPERTY_ACCESS  (0)
   PROPERTY_ACCESS  (1)
   PROPERTY_ACCESS  (2)
   PROPERTY_ACCESS  (3)
   PROPERTY_ACCESS  (4)
   PROPERTY_ACCESS  (5)
   PROPERTY_ACCESS  (6)
   PROPERTY_ACCESS  (7)
   PROPERTY_ACCESS  (8)
   PROPERTY_ACCESS  (9)
   PROPERTY_ACCESS  (10)
   PROPERTY_ACCESS  (11)
   PROPERTY_ACCESS  (12)
   PROPERTY_ACCESS  (13)
   PROPERTY_ACCESS  (14)
   PROPERTY_ACCESS  (15)

#undef PROPERTY_ACCESS

protected:
   void paintEvent (QPaintEvent* event);

   // Allows drived class to do specials as required, e.g. PV text or
   // the alarm severity colour.
   //
   virtual QString getItemText ();    /// returns "" unless overriden.
   virtual QColor getItemColour ();

private:
   void equaliseRect (QRect& rect);
   QString calcTextImage ();    // text image to be used.

   // Like painter.drawText, but centred on textCentre.
   // (painter.drawText aligns bottom left corner on given point).
   //
   void drawText (QPainter& painter, const QPoint& textCentre, const QString& text);

   Shapes shape;
   QStringList stateSet;
   TextFormats textFormat;
   QString fixedText;
   QEScanTimers::ScanRates flashRate;
   bool flashStateIsOn;
   int edgeWidth;
   int semiCycles;
   int percentSize;
   int centreAngle;
   int spanAngle;
   Qt::PenStyle edgeStyle;
   QColor flashOffColour;
   QColor edgeColour;
   QColor colourList [16];
   bool   flashList [16];
   bool isActive;
   int value;
   int modulus;

private slots:
   void flashTimeout (const bool);
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QSimpleShape::Shapes)
Q_DECLARE_METATYPE (QSimpleShape::TextFormats)
#endif

#endif // Q_SIMPLE_SHAPE_H
