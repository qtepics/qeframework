/*  QEFormMapper.cpp
 *
 */

#include <QDebug>
#include <QECommon.h>

#include "QEFormMapper.h"

#define DEBUG qDebug () << "QEFormMapper" << __FUNCTION__ << __LINE__ << "==>"

QEFormMapper* mapper = NULL;

//-----------------------------------------------------------------------------
//
bool QEFormMapper::initialise ()
{
   if (!mapper) mapper = new QEFormMapper ();  // create singleton instance.
   return true;
}

bool QEFormMapper::initialised = QEFormMapper::initialise ();

//-----------------------------------------------------------------------------
//
QEFormMapper::QEFormMapper ()
{
   this->nextId = 1;
}

//-----------------------------------------------------------------------------
//
QEFormMapper::~QEFormMapper()
{
   // place holder
}

//-----------------------------------------------------------------------------
// static
//
QEFormMapper::FormHandles QEFormMapper::nullHandle ()
{
   return 0;
}

//-----------------------------------------------------------------------------
// static
//
QEFormMapper::FormHandles QEFormMapper::allocateHandle ()
{
   mapper->nextId++;

   // Ensure not null (unlikely to wrap-around given the size of the handle space)
   // and also ensure unique (as in not currenlty in use).
   //
   // Note: although the map will not contain the allocated handle until
   // QEMappable::setFormIdentifier () is called with the allocated id, it will
   // take approx 18,000,000,000,000,000,000 calls before this handle is
   // potentially re-allocated (so we should be okay ;-)
   //
   while ( (mapper->nextId == mapper->nullHandle ()) ||
           (mapper->map.contains (mapper->nextId)) ) {
      mapper->nextId++;
   }
   return mapper->nextId;
}

//-----------------------------------------------------------------------------
// static
QWidget* QEFormMapper::findWidget (const FormHandles& handle)
{
   return mapper->map.value (handle, NULL);
}

//-----------------------------------------------------------------------------
// static
//
QWidget* QEFormMapper::findWidget (const FormHandles& handle, const QString& className)
{
   QWidget* form = QEFormMapper::findWidget (handle);
   QWidget* widget = NULL;

   if (form) {
      widget = QEUtilities::findWidget (form, className);
   }
   return widget;
}


//-----------------------------------------------------------------------------
//
bool QEFormMapper::associateHandleWithWidget (const QEFormMapper::FormHandles& handle, QWidget* widget)
{
   bool okay = false;

   if (widget && (handle != QEFormMapper::nullHandle ())) {
      if (!mapper->map.contains (handle)) {
         this->map.insert (handle, widget);
         okay = true;
      } else {
         DEBUG << "map already contains handle" << handle;
      }
   } else {
      DEBUG << "null widget and/or handle" << widget << handle;
   }
   return okay;
}

//-----------------------------------------------------------------------------
//
void QEFormMapper::disasociatedHandleAndWidget (const  QEFormMapper::FormHandles& handle)
{
   if (this->map.contains (handle)) {
      this->map.remove (handle);
   }
}

//==============================================================================
// QEMapable
//==============================================================================
//
QEMapable::QEMapable (QWidget* ownerIn)
{
   this->owner = ownerIn;
   this->formHandle = QEFormMapper::nullHandle ();
}

//-----------------------------------------------------------------------------
//
QEMapable::~QEMapable ()
{
   if (this->formHandle != QEFormMapper::nullHandle ()) {
      mapper->disasociatedHandleAndWidget (this->formHandle);
   }
}

//-----------------------------------------------------------------------------
//
bool QEMapable::setFormHandle (const QEFormMapper::FormHandles& handleIn)
{
   bool result = true;

   // Dis-associate existing handle if any
   //
   if (this->formHandle != QEFormMapper::nullHandle ()) {
      mapper->disasociatedHandleAndWidget (this->formHandle);
      this->formHandle = QEFormMapper::nullHandle ();
   }

   // Associate new handle if specified.
   //
   if (handleIn != QEFormMapper::nullHandle ()) {
      result = mapper->associateHandleWithWidget (handleIn, this->owner);
      if (result) {
         this->formHandle = handleIn;
      }
   }

   return result;
}

//-----------------------------------------------------------------------------
//
QEFormMapper::FormHandles QEMapable::getFormHandle () const
{
   return this->formHandle;
}

// end
