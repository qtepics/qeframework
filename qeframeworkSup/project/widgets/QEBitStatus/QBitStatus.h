/*  QBitStatus.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2011-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef Q_BIT_STATUS_H
#define Q_BIT_STATUS_H

#include <QString>
#include <QRect>
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QWidget>
#include <QEEnums.h>
#include <QEFrameworkLibraryGlobal.h>

/// This class is a BitStatus widget based on directly QWidget.
/// It provides a non-EPICS aware bit status widget.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QBitStatus : public QWidget
{
   Q_OBJECT

public:
   /// \enum    Shapes
   /// Specified the displayed bit shape
   enum Shapes {
      Rectangle,
      Circle
   };
   Q_ENUM (Shapes)

   Q_PROPERTY (int     value               READ getValue                WRITE setValue)
   Q_PROPERTY (int     numberOfBits        READ getNumberOfBits         WRITE setNumberOfBits)
   Q_PROPERTY (int     shift               READ getShift                WRITE setShift)

   /// For orientations/invertedAppearance
   /// Default: Horizontal
   /// Default: false, i.e. LSB on right if Horizontal, LSB in top if Vertical.
   ///
   Q_PROPERTY (Qt::Orientation orientation READ getOrientation          WRITE setOrientation)
   Q_PROPERTY (bool invertedAppearance     READ getInvertedAppearance   WRITE setInvertedAppearance)

   Q_PROPERTY (Shapes  shape               READ getShape                WRITE setShape)

   // If draw borders are off, a gap of zero means right and left pixel positions of
   // consecutive items are adjactent pixels. If draw borders are on, a gap of zero means
   // the borders of consecutive items use the same pixel position.
   //
   Q_PROPERTY (int     gap                 READ getGap                  WRITE setGap)

   /* The revserve polarity mask applies to value AFTER the shift.
    */
   Q_PROPERTY (QString reversePolarityMask READ getReversePolarityMask  WRITE setReversePolarityMask)

   /* The clear (aka inhibit aka not applicable) masks apply to value AFTER reversePolarityMask.
    */
   Q_PROPERTY (QString onClearMask         READ getOnClearMask          WRITE setOnClearMask)
   Q_PROPERTY (QString offClearMask        READ getOffClearMask         WRITE setOffClearMask)

   Q_PROPERTY (QColor  boarderColour       READ getBorderColour         WRITE setBorderColour)
   Q_PROPERTY (QColor  invalidColour       READ getInvalidColour        WRITE setInvalidColour)
   Q_PROPERTY (QColor  onColour            READ getOnColour             WRITE setOnColour)
   Q_PROPERTY (QColor  offColour           READ getOffColour            WRITE setOffColour)
   Q_PROPERTY (QColor  clearColour         READ getClearColour          WRITE setClearColour)

   Q_PROPERTY (bool    drawBorder          READ getDrawBorder           WRITE setDrawBorder)
   Q_PROPERTY (bool    isValid             READ getIsValid              WRITE setIsValid)
   Q_PROPERTY (bool    isActive            READ getIsActive             WRITE setIsActive)

public:
   // Constructor/destructor
   //
   explicit QBitStatus (QWidget *parent = 0);
   virtual ~QBitStatus ();

   virtual QSize sizeHint () const;

   // Property functions
   //
   void setBorderColour (const QColor value);
   QColor getBorderColour () const;

   void setOnColour (const QColor value);
   QColor getOnColour () const;

   void setOffColour (const QColor value);
   QColor getOffColour () const;

   void setInvalidColour (const QColor value);
   QColor getInvalidColour () const;

   void setClearColour (const QColor value);
   QColor getClearColour () const;

   void setDrawBorder (const bool value);
   bool getDrawBorder () const;

   void setNumberOfBits (const int value);
   int getNumberOfBits () const;

   void setGap (const int value);
   int getGap () const;

   void setShift (const int value);
   int getShift () const;

   void setOnClearMask (const QString value);
   QString getOnClearMask () const;

   void setOffClearMask (const QString value);
   QString getOffClearMask () const;

   void setReversePolarityMask (const QString value);
   QString getReversePolarityMask () const;

   void setIsValid (const bool value);
   bool getIsValid () const;

   void setOrientation (const Qt::Orientation value);
   Qt::Orientation getOrientation () const;

   void setInvertedAppearance (const bool invertedAppearance);
   bool getInvertedAppearance () const;

   void setShape (const Shapes value);
   Shapes getShape () const;

public slots:
   void setValue (const int value);

public:
   int getValue () const;

protected:
   void paintEvent (QPaintEvent *event);
   void setIsActive (const bool value);
   bool getIsActive () const;

private:
   // Note: the getXxxxColour functions (line 105-ish) gets the Xxxx property colour.
   // The getXxxxPaintColour functions return actual colour to for drawing the widget.
   //
   QColor getBorderPaintColour () const;
   QColor getOffPaintColour () const;
   QColor getOnPaintColour () const;
   QColor getInvalidPaintColour () const;

   // Like painter drawRect or drawEllipse, but bounded by rect, i.e.sensible.
   //
   void drawItem  (QPainter & painter, const QRect& rect);

   static QString intToMask (const int n);
   static int maskToInt (const QString mask);

   // Class member variable names start with m so as not to clash with
   // the property names - thats a qtcreator thing - not a c++ thing.
   // NOTE: Where possible I spell colour properly.
   //
   QColor mBorderColour;
   QColor mOnColour;
   QColor mOffColour;
   QColor mInvalidColour;
   QColor mClearColour;

   bool mDrawBorder;
   int  mGap;
   int  mNumberOfBits;      // 1 .. 32
   int  mShift;             // 0 .. 31
   int  mReversePolarityMask;
   int  mOnClearMask;
   int  mOffClearMask;
   bool mIsActive;          // i.e. is connected in CA/PVA speak
   bool mIsValid;
   int  mValue;
   Qt::Orientation mOrientation;
   bool mInvertedAppearance;
   Shapes mShape;
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QBitStatus::Shapes)
#endif

#endif // Q_BIT_STATUS_H
