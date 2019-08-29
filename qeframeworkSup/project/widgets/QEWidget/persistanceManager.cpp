/*  persistanceManager.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2019 Australian Synchrotron
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
 * Description:
 *
 * This class provides a mechanism to manage application persistance.
 * Any object - typically an application creating QE widgets can use this manager to save and restore
 * a configuration.
 * QE widgets also use this mechanism to maintain their own persistant data.
 *
 * - QE widgets that have persistant data and the application creating them such as QEGui
 *   can connect to the persistance manager and be notified by a signal that they should save
 *   or recover data.
 *
 * - The application (typically) can ask the persistance manager to save the current state or
 *   restore a previous named state.
 *
 * - When notified to save or restore, a signal is sent to all interested objects to save or restore data.
 *   The persistance manager provides methods to help build and read XML containing the data.
 *   the QEWidget class (a base class for all QEWidgets catched the save and restore signals and calls
 *   saveConfiguration() and restoreConfiguration() in the QE widget if implemented.
 *
 * - Each object dealing with the persistance manager must have a unique persistant name to identify itself
 *   when saving and restoring persistant data.
 *
 *   An application may use any rules it likes to determine persistant names for any objects it creates that
 *   will use the persistant manager.
 *   For example, QEGui's application object is named QEGui. QEGui's main windows are named
 *   QEGuiMainWindow_0, QEGuiMainWindow_1, etc.
 *
 *   If the application creates QEForm widgets, it should give these widgets a unique persistant name prefix,
 *   by calling QEForm::setUniqueIdentifier(), the QEForm widget and all QE framework widgets within it will use this
 *   prefix to build its own unique persistant name.
 *   For example, QEGui provides a unique name to each of the QEForm widgets it creates for presenting GUIs
 *   to the user. The unique names are in the form QEGui_window_<n>_form_<n>.
 *
 *   The QE framework generates unique persistant names for all QE widgets. This name is constructed from the
 *   unique persistant name given to any top level QEForm by the application, followed by the QE widget class name,
 *   followed by a numeric representation of its position in the widget hierarchy under the top level QEForm.
 *   For example a QEPvProperties widget deep within a GUI that QEGui has loaded into the fourth form in its third
 *   main window may have the name QEGui_window_2_form_3_QEPvProperties_2_3_1_3_1_3.
 *   If a QE widget is not within a widget hierarcy containing a QEForm with a unique persistant names, the QE widgets
 *   unique persistant name is simply the QE widget class name followed by a numeric representation of its position
 *   in the widget hierarchy.
 *
 * The PMElement class is used by users of the PersistanceManager class to return XML components from the XML data.
 * The PMElement class simplifies and hides QDom functionality.
 *
 * QE widgets can use the persistant manager through the QEWidget base class. They simply need to
 * overload QEWidget::saveConfiguration() and QEWidget::restoreConfiguration().
 *
 *
 * Typical scenarios:
 *
 *  Initial startup or restoring:
 *  =============================
 *
 *   - QEGui starts, connects to the persistance manager's save and restore signals.
 *
 *   - If specified in the startup parameters, or if the user requests a restore,
 *     QEGui asks the persistance manager to restore.
 *     (In the case of a user request, QEGui closes existing windows returning to a
 *     state very similar to initial startup)
 *
 *   - The restore signal is emitted. Only the QEGui application object is present to
 *     receive the signal.
 *
 *   - QEGui receives the signal to restore. (Yes, it know it's restoring - it just asked!)
 *     No QE widgets get this signal, they haven't been created yet.
 *
 *   - QEGui asks the persistance manager for its data.
 *
 *   - QGui uses the persistance manager to help parse the XML restore data it gets.
 *     QEGui then and acts on the recovered data. Typically the data will contain Main Window
 *     and GUI information such as positions, sizes, filenames.
 *
 *   - QEGui creates main windows and GUIs as its part of restoring a configuration.
 *
 *   - A second restore signal is emited.
 *   - The QEGui application object, and the QEGui main window objects ignore this signal.
 *     Any QE widgets that use persistance data will receive this signal (via the QEWidget
 *     base class), collect their data and apply it. for example a QEForm sub form may set
 *     its scroll bars.
 *
 *  Saving: (instigated by the user or optionaly when QEGui exits)
 *  =======
 *
 *   - QEGui asks the persistance manager to save, providing a name to tag the
 *     saved data with.
 *
 *   - The persistance manager sends a 'save' signal.
 *
 *   - All objects connected to the signal (QEGui itself and any interested QE
 *     widgets) use the persistance manager services to build XML data and
 *     then give that data to the persistance manager to save.
 *
 *   - The persistance manager saves all data presented to it.
 */

#include <persistanceManager.h>
#include <QDebug>
#include <QFile>
#include <QByteArray>
#include <QBuffer>
#include <QMessageBox>
#include <QEWidget.h>

//------------------------------------------------------------------------------
// QLockFile introduced in Qt 5.1 - fake it for earlier versions.
//
#if QT_VERSION >= 0x050100

#include <QLockFile>

// Prevent multiple processes from accessing concurrently the same resource.
//
class PersistanceManager::ResourceLocker : public QLockFile {
public:
   explicit ResourceLocker (const QString &filename ) :  QLockFile( filename ) {}
};

#else

class PersistanceManager::ResourceLocker {
public:
   explicit ResourceLocker( const QString & ) {}
   ~ResourceLocker() {}
   bool tryLock(int = 0) { return true; }
};

#endif

#define DEBUG qDebug () << "persistanceManager" << __LINE__ << __FUNCTION__ << "  "

#define CONFIG_COMPONENT_KEY "Component"

QString PersistanceManager::defaultName("Default");


//==============================================================================
// PersistanceManager class methods
//==============================================================================
// Construction
PersistanceManager::PersistanceManager()
{
   // Initialise
   restoring = false;
   doc = QDomDocument( "QEConfig" );
}

//------------------------------------------------------------------------------
// Destruction - place holder
PersistanceManager::~PersistanceManager() { }

//------------------------------------------------------------------------------
// Save the current configuration - set up
//
void PersistanceManager::saveProlog( const QString fileName,
                                     const QString rootName,
                                     const QString configName,
                                     const bool warnUser )
{
   // Try to read the configuration file we are saving to
   // If OK, remove the configuration we are overwriting if present.
   if( openRead( fileName, rootName, false, warnUser ) )
   {
      // Get the 'Config' elements
      QDomNodeList configNodes = docElem.elementsByTagName( "Config" );
      QDomElement oldConfig;

      // Look for the configuration with a name matching what we want to save
      int i;
      for( i = 0; i < configNodes.count(); i++ )
      {
         oldConfig = configNodes.at(i).toElement();
         if( oldConfig.attribute( "Name" ) == configName )
         {
            // A matching config name has been found
            break;
         }
      }

      // If a matching name was found...
      if( i < configNodes.count() )
      {
         // If interacting with the user, check it is OK to overwrite
         if( warnUser )
         {
            // Saving will overwrite previous configuration, check with the user this is OK
            QMessageBox msgBox;
            msgBox.setText( "A previous configuration will be overwritten. Do you want to continue?" );
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            switch ( msgBox.exec() )
            {
               case QMessageBox::Yes:
                  // Yes, continue
                  break;

               case QMessageBox::No:
               case QMessageBox::Cancel:
               default:
                  // No, do nothing
                  return;
            }
         }

         // Remove the old configuration
         docElem.removeChild( oldConfig );
      }
   }

   // Couldn't read the configuration file, create a new document
   else
   {
      doc.clear();
      docElem = doc.createElement( rootName );
   }

   // Add the root to the document
   doc.appendChild( docElem );

   // Add the configuration name
   config = doc.createElement( "Config" );
   config.setAttribute( "Name", configName );
   docElem.appendChild( config );
}

//------------------------------------------------------------------------------
// Save the current configuration
void PersistanceManager::save( const QString fileName,
                               const QString rootName,
                               const QString configName,
                               const bool warnUser )
{
   const QString lockFileName = QString ("%1.lck").arg (fileName);
   PersistanceManager::ResourceLocker lockFile (lockFileName);

   // Attempt to get the lock file - allow a 50 mSec grace.
   //
   if( lockFile.tryLock( 50 ) ) {
      // Initialise saving
      saveProlog( fileName, rootName, configName, warnUser );

      // Notify any interested objects to contribute their persistant data
      signal.save();

      // Finalise saving
      saveEpilog( fileName, warnUser );

   } else {
      DEBUG << "failed to lock file" << lockFileName;
   }
}

//------------------------------------------------------------------------------
// Save the current configuration of a single widget
void PersistanceManager::saveWidget( QEWidget* qewidget, const QString fileName,
                                     const QString rootName, const QString configName )
{
   if( !qewidget ) return;    // sanity check

   const QString lockFileName = QString ("%1.lck").arg (fileName);
   PersistanceManager::ResourceLocker lockFile (lockFileName);

   // Attempt to get the lock file - allow a 50 mSec grace.
   //
   if( lockFile.tryLock( 50 ) ) {
      // Initialise saving
      saveProlog( fileName, rootName, configName, true );

      // Request object save its persistant data
      qewidget->saveConfiguration( this );

      // Finalise saving
      saveEpilog( fileName, true );
   } else {
      DEBUG << "failed to lock file" << lockFileName;
   }
}

//------------------------------------------------------------------------------
// Save the current configuration - tidy up
void PersistanceManager::saveEpilog( const QString fileName, const bool warnUser )
{
   QFile file( fileName );

   // Write the configuration file
   if ( file.open( QIODevice::WriteOnly ) )
   {
      QTextStream ts( &file );
      ts << doc.toString() ;
      file.close();
   }

   // Handle not writing configuration file
   else
   {
      QString message = QString( "Could not save configuration. Could not open configuration file ").append( fileName );
      if( warnUser )
      {
         QMessageBox::warning( 0, "Configuration management", message );
      }
      else
      {
         qDebug() << "Configuration management: " << message;
      }
   }
}

//------------------------------------------------------------------------------
// Restore a configuration
void PersistanceManager::restore( const QString fileName,
                                  const QString rootName,
                                  const QString configName )
{
   const QString lockFileName = QString ("%1.lck").arg (fileName);
   PersistanceManager::ResourceLocker lockFile (lockFileName);

   // Attempt to get the lock file - allow a 50 mSec grace.
   //
   if( lockFile.tryLock( 50 ) ) {

      if( !openRead( fileName, rootName, true, true ) )
      {
         return;
      }

      config = getElement( docElem, "Config", "Name", configName );

      // Notify any interested objects to collect their persistant data
      restoring = true;
      signal.restore();
      restoring = false;
   } else {
      DEBUG << "failed to lock file" << lockFileName;
   }
}

//------------------------------------------------------------------------------
// Restore a configuration of a single widget
// Unlike saving, there is no prolog/epilog
void PersistanceManager::restoreWidget( QEWidget* qewidget, const QString fileName,
                                        const QString rootName, const QString configName )
{
   if( !qewidget ) return;

   const QString lockFileName = QString ("%1.lck").arg (fileName);
   PersistanceManager::ResourceLocker lockFile (lockFileName);

   // Attempt to get the lock file - allow a 50 mSec grace.
   //
   if( lockFile.tryLock( 50 ) ) {

      if( !openRead( fileName, rootName, true, true ) )
      {
         return;
      }

      config = getElement( docElem, "Config", "Name", configName );

      // Request object restore its persistant data
      restoring = true;
      qewidget->restoreConfiguration( this, QEWidget::FRAMEWORK );
      restoring = false;

   } else {
      DEBUG << "failed to lock file" << lockFileName;
   }
}

//------------------------------------------------------------------------------
// Returns true if a restore is in progress.
// Used when QE widgets are being created to determine if they are being restored
bool PersistanceManager::isRestoring() const
{
   return restoring;
}

//------------------------------------------------------------------------------
// Open and read the configuration file
bool PersistanceManager::openRead( const QString fileName, const QString rootName,
                                   const bool fileExpected, const bool warnUser )
{
   QFile file( fileName );
   if (!file.open(QIODevice::ReadOnly))
   {
      if( fileExpected )
      {
         QString message = QString( "Could not open configuration file for reading: ").append( fileName );
         if( warnUser )
         {
            QMessageBox::warning( 0, "Configuration management", message );
         }
         else
         {
            qDebug() << "Configuration management: " << message;
         }
      }
      return false;
   }

   if ( !doc.setContent( &file ) )
   {
      QString message = QString( "Could not parse the XML in the config file: ").append( fileName );
      if( warnUser )
      {
         QMessageBox::warning( 0, "Configuration management", message );
      }
      else
      {
         qDebug() << "Configuration management: " << message ;
      }
      file.close();
      return false;
   }
   file.close();

   docElem = doc.documentElement();

   if( docElem.nodeName().compare( rootName ) )
   {
      QString message = QString( "XML did not contain the expected root element " ).append( rootName ).append( " in the config file: ").append( fileName );
      if( warnUser )
      {
         QMessageBox::warning( 0, "Configuration management", message );
      }
      else
      {
         qDebug() << "Configuration management: " << message;
      }
      return false;
   }
   return true;
}

//------------------------------------------------------------------------------
// Add a named configuration. Used during a save signal. The returned element is then loaded with configuration data
PMElement PersistanceManager::addNamedConfiguration( QString name )
{
   QDomElement element = doc.createElement( CONFIG_COMPONENT_KEY );
   element.setAttribute( "Name", name );
   config.appendChild( element );
   return PMElement( this, element );
}

//------------------------------------------------------------------------------
// Get a named configuration. Used during a restore signal.
// The returned element contains the configuration data
PMElement PersistanceManager::getNamedConfiguration( QString name )
{
   QDomElement element = getElement( config, CONFIG_COMPONENT_KEY, "Name", name );
   return PMElement( this, element );
}

//------------------------------------------------------------------------------
// Determine if a configuration is present
bool PersistanceManager::isConfigurationPresent( const QString fileName,
                                                 const QString rootName,
                                                 const QString configName )
{
   QStringList nameList = getConfigNames( fileName, rootName );
   return nameList.contains( configName );
}

//------------------------------------------------------------------------------
// Get a list of the existing configurations
QStringList PersistanceManager::getConfigNames( QString fileName, QString rootName )
{
   bool hasDefault;
   return getConfigNames( fileName, rootName, hasDefault );
}

//------------------------------------------------------------------------------
// Get a list of the existing configurations (and if there is a default configuration)
QStringList PersistanceManager::getConfigNames( QString fileName, QString rootName,
                                                bool& hasDefault )
{
   hasDefault = false;
   QStringList nameList;

   // Return the empty list if can't read file
   if( !openRead( fileName, rootName, false, true ) )
   {
      return nameList;
   }

   QDomNodeList nodeList = docElem.elementsByTagName( "Config" );
   for( int i = 0; i < nodeList.count(); i++ )
   {
      QString configName = nodeList.at(i).toElement().attribute( "Name" );
      if( !configName.isEmpty() )
      {
         if(  configName == PersistanceManager::defaultName )
         {
            hasDefault = true;
         }
         else
         {
            nameList.append( configName );
         }
      }
   }
   return nameList;
}

//------------------------------------------------------------------------------
// Delete a list of configurations, confirming deletions with the user if appropriate
void PersistanceManager::deleteConfigs( const QString fileName, const QString rootName,
                                        const QStringList names, const bool warnUser )
{
   // Deleting configurations. If appropriate check with the user this is OK
   if( warnUser )
   {
      QMessageBox msgBox;
      QString message;
      if( names.count()==1 && names.at(0) == PersistanceManager::defaultName )
      {
         message = QString( "The default configuration used at startup will be deleted. Do you want to continue?" );
      }
      else
      {
         message = QString( "%1 configuration%2 will be deleted. Do you want to continue?" ).arg( names.count() ).arg( names.count()>1?QString("s"):QString("") );
      }

      msgBox.setText( message );
      msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Cancel);
      switch ( msgBox.exec() )
      {
         case QMessageBox::Yes:
            // Yes, continue
            break;

         case QMessageBox::No:
         case QMessageBox::Cancel:
         default:
            // No, do nothing
            return;
      }
   }

   // Try to read the configuration file we are saving to
   // If OK, remove the configuration we are overwriting if present.
   if( openRead( fileName, rootName, true, warnUser ) )
   {
      QDomNodeList nodeList = docElem.elementsByTagName( "Config" );
      for( int i = 0; i < names.count(); i++ )
      {
         for( int j = 0; j < nodeList.count(); j++ )
         {
            if( nodeList.at(j).toElement().attribute( "Name" ) == names[i] )
            {
               docElem.removeChild( nodeList.at( j ));
               break;
            }
         }
      }
   }

   // Recreate the file
   QFile file( fileName );
   if ( file.open( QIODevice::WriteOnly ) )
   {
      QTextStream ts( &file );
      ts << doc.toString() ;
      file.close();
   }
   else
   {
      QString message = QString( "Could not save remaining configurations to configuration file ").append( fileName );
      if( warnUser )
      {
         QMessageBox::warning( 0, "Configuration management", message );
      }
      else
      {
         qDebug() << "Configuration management: " << message;
      }
   }
}

//------------------------------------------------------------------------------
// Get a node list by name
QDomNodeList PersistanceManager::getElementList( QDomElement element, QString name ) const
{
   if( element.isNull() || !element.isElement() )
      return QDomNodeList();

   return element.elementsByTagName( name );
}

//------------------------------------------------------------------------------
// Get the (first) named element from within an element
QDomElement PersistanceManager::getElement( QDomElement element, QString name ) const
{
   if( element.isNull() || !element.isElement() )
      return QDomElement();

   return element.elementsByTagName( name ).at(0).toElement();
}

//------------------------------------------------------------------------------
// Get one of any matching named elements from within an element
QDomElement PersistanceManager::getElement( QDomElement element, QString name, int i ) const
{
   if( element.isNull() || !element.isElement() )
      return QDomElement();

   return element.elementsByTagName( name ).at(i).toElement();
}

//------------------------------------------------------------------------------
// Get an element from within an element
QDomElement PersistanceManager::getElement( QDomNodeList nodeList, int i ) const
{
   if( nodeList.isEmpty() || nodeList.count() <= i )
      return QDomElement();

   return nodeList.at( i ).toElement();
}

//------------------------------------------------------------------------------
// Get a named element with a matching attribute
// This is used when there is a group of same-named elements differentiated by an attribute value.
QDomElement PersistanceManager::getElement( QDomElement element,
                                            QString elementName,
                                            QString attrName,
                                            QString attrValue ) const
{
   // If element is not null and is an element
   if( !element.isNull() && element.isElement() )
   {
      // Look for correct attribute in each matching element
      QDomNodeList list = element.elementsByTagName( elementName );
      for( int i = 0; i < list.count(); i++ )
      {
         QDomNode listNode = list.at( i );
         if( listNode.isElement() )
         {
            QDomElement listElement = listNode.toElement();
            QDomNode attrNode = listElement.attributes().namedItem( attrName );
            if( !attrNode.isNull() )
            {
               if( attrNode.nodeValue() == attrValue )
               {
                  return listElement;
               }
            }
         }
      }
   }

   // Not found
   return QDomElement();
}

//------------------------------------------------------------------------------
// Get a named boolean value from an element
bool PersistanceManager::getElementValue( QDomElement element,
                                          QString name, bool& val ) const
{
   QString strVal;
   bool okay;
   int b;

   if( !getElementValue( element, name, strVal ) )
   {
      return false;
   }

   b = strVal.toInt( &okay );
   if (okay && ((b == 0) || (b == 1))) {
      val = (b == 1);
   } else {
      okay = false;
   }
   return okay;
}

//------------------------------------------------------------------------------
// Get a named integer value from an element
bool PersistanceManager::getElementValue( QDomElement element,
                                          QString name, int& val ) const
{
   QString strVal;
   bool okay;

   if( !getElementValue( element, name, strVal ) )
   {
      return false;
   }

   val = strVal.toInt( &okay );
   return okay;
}

//------------------------------------------------------------------------------
// Get a named double value from an element
bool PersistanceManager::getElementValue( QDomElement element,
                                          QString name, double& val ) const
{
   QString strVal;
   bool okay;

   if( !getElementValue( element, name, strVal ) )
   {
      return false;
   }

   val = strVal.toDouble( &okay );
   return okay;
}

//------------------------------------------------------------------------------
// Get a named string value from an element
bool PersistanceManager::getElementValue( QDomElement element,
                                          QString name, QString& val ) const
{
   if( element.isNull() || !element.isElement() )
   {
      val = QString();
      return false;
   }

   QDomNode textElement = element.namedItem( name );
   QDomNode node = textElement.firstChild().toText();
   QDomText text = node.toText();
   if( text.isNull() )
   {
      val = QString();
      return false;
   }

   val = text.nodeValue();
   return true;
}

//------------------------------------------------------------------------------
// Get a named colour value from an element
bool PersistanceManager::getElementValue( QDomElement element,
                                          QString name, QColor& val ) const
{
   QDomElement colourElement = getElement( element, name );
   if( colourElement.isNull() || !colourElement.isElement() )
   {
      return false;
   }

   int r = 0, g = 0, b = 0, a = 0;
   if (getElementAttribute( colourElement, "red",    r ) &&
       getElementAttribute( colourElement, "green",  g ) &&
       getElementAttribute( colourElement, "blue",   b ) &&
       getElementAttribute( colourElement, "alpha",  a ))
   {
      val.setRgb( r, g, b, a );
      return true;
   }
   return false;
}

//------------------------------------------------------------------------------
// Get a named boolean attribute from an element
bool PersistanceManager::getElementAttribute( QDomElement element,
                                              QString name, bool& val ) const
{
   // If not null and is an element...
   if( !element.isNull() && element.isElement() )
   {
      // If attribute is present...
      QDomNode node = element.attributes().namedItem( name );
      if( !node.isNull() )
      {
         // If integer attribute is present, return it as a bool
         bool okay;
         int b = node.nodeValue().toInt( &okay );
         if( okay && ((b == 0) || (b == 1)) )
         {
            val = (b == 1);
            return true;
         }
      }
   }

   // Something was wrong, default to zero
   val = 0;
   return false;
}

//------------------------------------------------------------------------------
// Get a named integer attribute from an element
bool PersistanceManager::getElementAttribute( QDomElement element,
                                              QString name, int& val ) const
{
   // If not null and is an element...
   if( !element.isNull() && element.isElement() )
   {
      // If attribute is present...
      QDomNode node = element.attributes().namedItem( name );
      if( !node.isNull() )
      {
         // If integer attribute is present, return it
         bool okay;
         val = node.nodeValue().toInt( &okay );
         if( okay )
         {
            return true;
         }
      }
   }

   // Something was wrong, default to zero
   val = 0;
   return false;
}

//------------------------------------------------------------------------------
// Get a named double attribute from an element
bool PersistanceManager::getElementAttribute( QDomElement element,
                                              QString name, double& val ) const
{
   // If not null and is an element...
   if( !element.isNull() && element.isElement() )
   {
      // If attribute is present...
      QDomNode node = element.attributes().namedItem( name );
      if( !node.isNull() )
      {
         // If double attribute is present, return it
         bool okay;
         val = node.nodeValue().toDouble( &okay );
         if( okay )
         {
            return true;
         }
      }
   }

   // Something was wrong, default to zero
   val = 0;
   return false;
}

//------------------------------------------------------------------------------
// Get a named string attribute from an element
bool PersistanceManager::getElementAttribute( QDomElement element,
                                              QString name, QString& val ) const
{
   // If not null and is an element...
   if( !element.isNull() && element.isElement() )
   {
      // If attribute is present...
      QDomNode node = element.attributes().namedItem( name );
      if( !node.isNull() )
      {
         val = node.nodeValue();
         return true;
      }
   }

   // Something was wrong, default to empty string
   val = QString();
   return false;
}

//------------------------------------------------------------------------------
// Add an element, return the new added element
PMElement PersistanceManager::addElement( QDomElement parent, QString name )
{
   QDomElement element = doc.createElement( name );
   parent.appendChild( element );
   return PMElement( this, element );
}

//------------------------------------------------------------------------------
// Add a boolean value to an element
void PersistanceManager::addValue( QDomElement parent, QString name, bool value )
{
   // Enocode boolean as integer
   addValue( parent, name, ( value ? 1 : 0 ) );
}

//------------------------------------------------------------------------------
// Add an integer value to an element
void PersistanceManager::addValue( QDomElement parent, QString name, int value )
{
   addValue( parent, name, QString( "%1" ).arg ( value ) );
}

//------------------------------------------------------------------------------
// Add a double value to an element
void PersistanceManager::addValue( QDomElement parent, QString name, double value )
{
   // Default precision is designed for school-boy sums, not professional
   // engineering, hence need full double precision required.
   addValue( parent, name, QString( "%1" ).arg( value, 0, 'g', 16 ) );
}

//------------------------------------------------------------------------------
// Add a string value to an element
void PersistanceManager::addValue( QDomElement parent, QString name, QString value )
{
   QDomElement element = doc.createElement( name );
   parent.appendChild( element );
   QDomText text = doc.createTextNode( value );
   element.appendChild( text );
}

//------------------------------------------------------------------------------
// Add a colour value to an element
// No value per se - just attributes.
void PersistanceManager::addValue( QDomElement parent, QString name, const QColor& value )
{
   QDomElement element = doc.createElement( name );
   parent.appendChild( element );

   int r, g, b, a;
   value.getRgb (&r, &g, &b, &a);
   element.setAttribute( "red",    r );
   element.setAttribute( "green",  g );
   element.setAttribute( "blue",   b );
   element.setAttribute( "alpha",  a );
}

//------------------------------------------------------------------------------
// Add a boolean attribute to an element
void PersistanceManager::addAttribute( QDomElement element, QString name, bool value )
{
   element.setAttribute( name, value );
}

//------------------------------------------------------------------------------
// Add an integer attribute to an element
void PersistanceManager::addAttribute( QDomElement element, QString name, int value )
{
   element.setAttribute( name, value );
}

//------------------------------------------------------------------------------
// Add a double attribute to an element
void PersistanceManager::addAttribute( QDomElement element, QString name, double value )
{
   element.setAttribute( name, value );
}

//------------------------------------------------------------------------------
// Add a string attribute to an element
void PersistanceManager::addAttribute( QDomElement element, QString name, QString value )
{
   element.setAttribute( name, value );
}

//------------------------------------------------------------------------------
// Get a reference to the object that will supply save and restore signals
QObject* PersistanceManager::getSaveRestoreObject()
{
   return &signal;
}


//==============================================================================
// PMElement class methods
//==============================================================================
// Construction
PMElement::PMElement( PersistanceManager* ownerIn, QDomElement elementIn ) : QDomElement( elementIn )
{
   owner = ownerIn;
}

//------------------------------------------------------------------------------
// Add an element
PMElement PMElement::addElement( QString name )
{
   return owner->addElement( *this, name );
}

//------------------------------------------------------------------------------
// Add a boolean value
void PMElement::addValue( QString name, bool value )
{
   // Enocode boolean as integer
   return owner->addValue( *this, name, value ? 1 : 0 );
}

//------------------------------------------------------------------------------
// Add an integer value
void PMElement::addValue( QString name, int value )
{
   return owner->addValue( *this, name, QString( "%1" ).arg( value ) );
}

//------------------------------------------------------------------------------
// Add a double value
void PMElement::addValue( QString name, double value )
{
   // Don't be clever - just go for full resolution.
   return owner->addValue( *this, name, QString( "%1" ).arg( value, 0, 'g', 16 ) );
}

//------------------------------------------------------------------------------
// Add a string value
void PMElement::addValue( QString name, QString value )
{
   return owner->addValue( *this, name, value );
}

//------------------------------------------------------------------------------
// Add a colour value
void PMElement::addValue( QString name, const QColor& value )
{
   return owner->addValue( *this, name, value );
}

//------------------------------------------------------------------------------
// Add a boolean attribute
void PMElement::addAttribute( QString name, bool value )
{
   owner->addAttribute( *this, name, value );
}

//------------------------------------------------------------------------------
// Add an integer attribute
void PMElement::addAttribute( QString name, int value )
{
   owner->addAttribute( *this, name, value );
}

//------------------------------------------------------------------------------
// Add a double attribute
void PMElement::addAttribute( QString name, double value )
{
   owner->addAttribute( *this, name, value );
}

//------------------------------------------------------------------------------
// Add a string attribute
void PMElement::addAttribute( QString name, QString value )
{
   owner->addAttribute( *this, name, value );
}

//------------------------------------------------------------------------------
// Get a named element
PMElement PMElement::getElement( QString name ) const
{
   return PMElement( owner, owner->getElement( *this, name ) );
}

//------------------------------------------------------------------------------
// Get one element from a named element list
PMElement PMElement::getElement( QString name, int i ) const
{
   return PMElement( owner, owner->getElement( *this, name, i ) );
}

//------------------------------------------------------------------------------
// Get a named element list
PMElementList PMElement::getElementList( QString name ) const
{
   return PMElementList( owner, owner->getElementList( *this, name ) );
}

//------------------------------------------------------------------------------
// Get a named element with a matching attribute
// This is used when there is a group of same-named elements differentiated by an attribute value.
PMElement PMElement::getElement( QString elementName, QString attrName, QString attrValue ) const
{
   return PMElement( owner, owner->getElement( *this, elementName, attrName, attrValue ) );
}

//------------------------------------------------------------------------------
PMElement PMElement::getElement( QString name, QString attrName, int attrValue ) const
{
   return getElement( name, attrName, QString( "%1" ).arg( attrValue ) );
}

//------------------------------------------------------------------------------
// Get a boolean value
bool PMElement::getValue( QString name, bool& val ) const
{
   return owner->getElementValue( *this, name, val );
}

//------------------------------------------------------------------------------
// Get an integer value
bool PMElement::getValue( QString name, int& val ) const
{
   return owner->getElementValue( *this, name, val );
}

//------------------------------------------------------------------------------
// Get a double value
bool PMElement::getValue( QString name, double& val ) const
{
   return owner->getElementValue( *this, name, val );
}

//------------------------------------------------------------------------------
// Get a string value
bool PMElement::getValue( QString name, QString& val ) const
{
   return owner->getElementValue( *this, name, val );
}

//------------------------------------------------------------------------------
// Get a colour value
bool PMElement::getValue( QString name, QColor& val ) const
{
   return owner->getElementValue( *this, name, val );
}

//------------------------------------------------------------------------------
// Get a named boolean attribute from an element
bool PMElement::getAttribute( QString name, bool& val ) const
{
   return owner->getElementAttribute( *this, name, val );
}

//------------------------------------------------------------------------------
// Get a named integer attribute from an element
bool PMElement::getAttribute( QString name, int& val ) const
{
   return owner->getElementAttribute( *this, name, val );
}

//------------------------------------------------------------------------------
// Get a named double attribute from an element
bool PMElement::getAttribute( QString name, double& val ) const
{
   return owner->getElementAttribute( *this, name, val );
}

//------------------------------------------------------------------------------
// Get a named string attribute from an element
bool PMElement::getAttribute( QString name, QString& val ) const
{
   return owner->getElementAttribute( *this, name, val );
}


//==============================================================================
// PMElementList class methods
//==============================================================================
// Construction
PMElementList::PMElementList( PersistanceManager* ownerIn,
                              QDomNodeList elementListIn ) : QDomNodeList( elementListIn )
{
   owner = ownerIn;
}

//------------------------------------------------------------------------------
// Get an element from the list
PMElement PMElementList::getElement( int i ) const
{
   // check range of i
   if( (i >= 0) && (i < count()) ) {
      return PMElement( owner, this->at( i ).toElement() );
   } else {
      DEBUG << "element" << i << "out of range";
      QDomElement nullDomElemnt;
      PMElement nullPMElement( owner, nullDomElemnt );
      return nullPMElement;
   }
}

//------------------------------------------------------------------------------
// Return the size of the list
int PMElementList::count() const
{
   return ( (QDomNodeList*)this )->count();
}


//==============================================================================
// SaveRestoreSignal class mothods
//==============================================================================
//
void SaveRestoreSignal::save()
{
   // Ask all interested components to add their persistant data
   //!!! signal must be blocking
   emit saveRestore( SAVE );
}

//------------------------------------------------------------------------------
void SaveRestoreSignal::restore()
{
   // Ask  all interested components to collect their persistant data
   //!!! signal must be blocking
   emit saveRestore( RESTORE_APPLICATION );
   emit saveRestore( RESTORE_QEFRAMEWORK );
}

// end
