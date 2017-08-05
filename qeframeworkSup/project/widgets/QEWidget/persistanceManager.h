/*  persistanceManager.h
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
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
 * Description:
 *
 * This class provides a mechanism to manage application persistance
 * Any object - typically an application creating QE widgets can use this manager to save and restore
 * a configuration.
 * Refer to persistanceManager.cpp for further details
 */

#ifndef PERSISTANCE_MANAGER_H
#define PERSISTANCE_MANAGER_H

#include <QObject>
#include <QHash>
#include <QStringList>
#include <QXmlStreamWriter>
#include <QEFrameworkLibraryGlobal.h>
#include <QXmlDefaultHandler>
#include <QVariant>
#include <QDomDocument>


// Save / Restore configuration name
#define QE_CONFIG_NAME "QEGuiConfig"

class PersistanceManager;
// Class used to generate signals that a save or restore is require.
// A single instance of this class is shared by all instances of
// the persistanceManager class.
// The persistanceManager class can't generate signals directly as it
// is not based on QObject and can't be as it is a base class for
// Widgets that may also be based on Qwidgets and only one base
// class can be based on a QObject.
class SaveRestoreSignal : public QObject
{
    Q_OBJECT

public:

    // Constructor, destructor
    SaveRestoreSignal(){ owner = NULL; }
    ~SaveRestoreSignal(){}
    void setOwner( PersistanceManager* ownerIn ){ owner = ownerIn; }


    void save();      // Save the current configuration
    void restore();   // Restore a configuration




enum saveRestoreOptions { SAVE, RESTORE_APPLICATION, RESTORE_QEFRAMEWORK };
  signals:
    // Internal use only. Send when a save or restore is needed.
    void saveRestore( SaveRestoreSignal::saveRestoreOptions option );   // Saving or restoring

  private:
    PersistanceManager* owner;
};

// Class to maintain a context while parsing XML save/restore data
class PMContext
{
public:
    PMContext(){ context = 0; }
private:
    int context;
};

class PMElement;

// Class to conceal QDomNodeList from users of the persistance manager (and make it easier to add stuff)
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT PMElementList: private QDomNodeList
{
public:
    PMElementList( PersistanceManager* ownerIn, QDomNodeList elementListIn );
    PMElement getElement( int i );                          // Get an element from the list
    int count(){ return ((QDomNodeList*)this)->count(); }   // Return the size of the list

private:
    PersistanceManager* owner;                              // Persistant manager reference
};

// Class to conceal QDomElement from users of the persistance manager (and make it easier to add stuff)
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT PMElement : private QDomElement
{
public:
    PMElement( PersistanceManager* ownerIn, QDomElement elementIn );
    PMElement addElement( QString name );                   // Add an element

    void addValue( QString name, bool value );              // Add a boolean value
    void addValue( QString name, int value );               // Add an integer value
    void addValue( QString name, double value );            // Add a double value
    void addValue( QString name, QString value );           // Add a string value

    void addAttribute( QString name, bool value );          // Add a boolean attribute
    void addAttribute( QString name, int value );           // Add an integer attribute
    void addAttribute( QString name, double value );        // Add a double attribute
    void addAttribute( QString name, QString value );       // Add a string attribute

    PMElement getElement( QString name );                   // Get a named element
    PMElement getElement( QString name, int i );            // Get one element from a named element list
    PMElement getElement( QString name, QString attrName, QString attrValue ); // Get a named element with a matching attribute
    PMElement getElement( QString name, QString attrName, int     attrValue ); // Get a named element with a matching attribute

    PMElementList getElementList( QString name );           // Get a named element list

    bool getValue( QString name, bool& val );               // Get a boolean value
    bool getValue( QString name, int& val );                // Get an integer value
    bool getValue( QString name, double& val );             // Get a double value
    bool getValue( QString name, QString& val );            // Get a string value

    bool getAttribute( QString name, bool& val );           // Get a named boolean attribute from an element
    bool getAttribute( QString name, int& val );            // Get a named integer attribute from an element
    bool getAttribute( QString name, double& val );         // Get a named double attribute from an element
    bool getAttribute( QString name, QString& val );        // Get a named string attribute from an element

    bool isNull(){ return QDomElement::isNull(); }          // Indicate if an element is empty

private:
    PersistanceManager* owner;                              // Persistance manager that supplied this PMElement instance
};

// Persistance manager
//
class QEWidget;   // differed

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT PersistanceManager : public QXmlDefaultHandler
{
public:

    friend class PMElement;

    PersistanceManager();   // Construction

    // Main kickstarter methods
    QObject* getSaveRestoreObject();                                                                  // Get a reference to the object that will supply save and restore signals
    void     save( const QString fileName, const QString rootName, const QString configName, const bool warnUser );        // Save the current configuration
    void     restore( const QString fileName, const QString rootName, const QString configName );     // Restore a configuration
    void     saveWidget( QEWidget* qewidget, const QString fileName,                                  // Save the current configuration of nominated QEWidget
                         const QString rootName, const QString configName );                          //
    void     restoreWidget( QEWidget* qewidget, const QString fileName,                               // Restore a configuration of nominated QEWidget
                            const QString rootName, const QString configName );                       //
    bool     restoring;                                                                               // True if a restore is in progress
    bool     isRestoring();                                                                           // Returns true if a restore is in progress

    // Helper methods to build configuration data
    PMElement addNamedConfiguration( QString name );               // Add a named configuration. Used during a save signal. The returned element is then loaded with configuration data
    PMElement getNamedConfiguration( QString name );               // Get a named configuration. Used during a restore signal. The returned element contains the configuration data

    // Configuration management
    QStringList getConfigNames( QString fileName, QString rootName );                   // Get a list of the existing configurations
    QStringList getConfigNames( QString fileName, QString rootName, bool& hasDefault ); // Get a list of the existing configurations (and if there is a default configuration)
    void deleteConfigs( const QString fileName, const QString rootName, const QStringList names, const bool warnUser );        // Delete a list of configurations
    bool isConfigurationPresent( const QString fileName, const QString rootName, const QString configName );    // Is a given configuration name present

    static QString defaultName;

private:
    void saveProlog( const QString fileName, const QString rootName, const QString configName, const bool warnUser );   // common save/saveWidget functionality
    void saveEpilog( const QString fileName, const bool warnUser );                                                     // common save/saveWidget functionality

    bool openRead( const QString fileName, const QString rootName, const bool fileExpected, const bool warnUser );// Open and read the configuration file. fileExpected is true if the file must be present

    PMElement addElement( QDomElement parent, QString name );               // Add an element, return the new added element

    void addValue( QDomElement parent, QString name, bool value );          // Add a boolean value to an element
    void addValue( QDomElement parent, QString name, int value );           // Add an integer value to an element
    void addValue( QDomElement parent, QString name, double value );        // Add a double value to an element
    void addValue( QDomElement parent, QString name, QString value );       // Add a string value to an element

    void addAttribute( QDomElement element, QString name, bool value );     // Add a boolean attribute to an element
    void addAttribute( QDomElement element, QString name, int value );      // Add an integer attribute to an element
    void addAttribute( QDomElement element, QString name, double value );   // Add a double attribute to an element
    void addAttribute( QDomElement element, QString name, QString value );  // Add a string attribute to an element

    QDomElement getElement( QDomElement element, QString name );            // Get the (first) named element from within an element
    QDomElement getElement( QDomElement element, QString name, int i );     // Get one of any matching named elements from within an element
    QDomElement getElement( QDomNodeList nodeList, int i );                 // Get an element from within an element
    QDomElement getElement( QDomElement element, QString name, QString attrName, QString attrValue ); // Get a named element with a matching attribute

    QDomNodeList getElementList( QDomElement element, QString name );       // Get a named element list from an element

    bool getElementValue( QDomElement element, QString name, bool& val );        // Get a named boolean value from an element
    bool getElementValue( QDomElement element, QString name, int& val );         // Get a named integer value from an element
    bool getElementValue( QDomElement element, QString name, double& val );      // Get a named double value from an element
    bool getElementValue( QDomElement element, QString name, QString& val );     // Get a named string value from an element

    bool getElementAttribute( QDomElement element, QString name, bool& val );    // Get a named boolean attribute from an element
    bool getElementAttribute( QDomElement element, QString name, int& val );     // Get a named integer attribute from an element
    bool getElementAttribute( QDomElement element, QString name, double& val );  // Get a named double attribute from an element
    bool getElementAttribute( QDomElement element, QString name, QString& val ); // Get a named string attribute from an element


    SaveRestoreSignal signal;           // Save/Restore signal object. One instance to signal all QE Widgets and applications

    QDomDocument doc;                   // Save and restore xml document
    QDomElement config;                 // Current configuration

    QDomElement docElem;                // Configuration document
};

#endif // PERSISTANCE_MANAGER_H
