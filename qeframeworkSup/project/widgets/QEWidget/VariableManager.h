/*  VariableManager.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2025 Australian Synchrotron
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
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_VARIABLE_MANAGER_H
#define QE_VARIABLE_MANAGER_H

#include <QList>
#include <QCaObject.h>
#include <VariableNameManager.h>


/**
  This class manages CA process variables for the QEWidget class.
  The QEWidget class provides a lot of generic support for all QE widgets
  (such as QELabel, QESpinBox, etc), including managing CA data sources. It manages CA
  data sources through this class.

  Refer to the QEwidget class description for an overview of how QE widgets use QEWidget.

  The VariableManager class manages QCaObject classes that stream updates to the
  CA aware widgets. But the VariableManager class, however, doesn't know how to format the data,
  or how the updates will be used.
  To resolve this, the VariableManager class asks a parent class (such as QELabel) to create the
  QCaObject class in what ever flavour it wants, by calling the virtual function createQcaItem.
  A QELabel, for example, wants string updates so it creates a QEString which is based on a
  QCaObject class and formats all updates as strings.

  The CA aware parent class (such as QELabel) defines a variable by calling VariableNameManager::setVariableName().
  The VariableNamePropertyManager class calls the establishConnection function of the CA aware parent class, such as QELabel
  when it has a new variable name.

  After construction, a CA aware widget is activated (starts updating) by VariableManager calling its
  establishConnection() function in one of two ways:

   1) The variable name or variable name substitutions is changed by calling setVariableName
      or setVariableNameSubstitutions respectively. These functions are in the VariableNameManager class.
      The VariableNamePropertyManager calls a virtual function establishConnection() which is implemented by the CA aware widget.
      This is how a CA aware widget is activated in 'designer'. It occurs when 'designer' updates the
      variable name property or variable name substitution property.

   2) When an QEForm widget is created, resulting in a set of CA aware widgets being created by loading a UI file
      contining plugin definitions.
      After loading the plugin widgets, code in the QEForm class calls the activate() function in the VariableManager base of
      this class (QEWiget).
      The activate() function calls  establishConnection() in the CA aware widget for each variable. This simulates
      what the VariableNamePropertyManager does as each variable name is entered (see 1, above, for details)

  No matter which way a CA aware widget is activated, the establishConnection() function in the CA aware widget is called
  for each variable. The establishConnection() function asks this QEWidget base class, by calling the createConnection()
  function, to perform the tasks common to all CA aware widgets for establishing a stream of CA data.

  The createVariable() function calls the CA aware widget back asking it to create an object based on QCaObject.
  This object will supply a stream of CA update signals to the CA aware object in a form that it needs. For example a QELabel
  creates a QEString object. The QEString class is based on the QCaObject class and converts all update data to a strings
  which is required for updating a Qt label widget. This class stores the QCaObject based class.

  After the establishConnection() function in the CA aware widget has called createConnection(), the remaining task of the
  establishConnection() function is to connect the signals of the newly created QCaObject based classes to its own slots
  so that data updates can be used. For example, a QELabel connects the 'stringChanged' signal
  from the QEString object to its setLabelText slot.
 */

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT VariableManager :
   public VariableNameManager {
public:
   /// Constructor
   explicit VariableManager ();

   /// Destructor
   virtual ~VariableManager ();

   /// Initiate updates.
   /// Called after all configuration is complete.
   /// Note: This function invokes the virtual activated() function.
   void activate ();

   /// Terminates updates.
   /// This has been provided for third party (non QEGui) applications using the framework.
   /// Note: This function invokes the virtual deactivated() function.
   ///
   void deactivate ();

   /// Return a reference to one of the qCaObjects used to stream CA updates
   ///
   qcaobject::QCaObject* getQcaItem (unsigned int variableIndex) const;

   /// Perform a single shot read on all variables (Usefull when not subscribing by default)
   ///
   void readNow ();

   /// (Control widgets only - such as QELineEdit)
   /// Write the value now. Used when writeOnChange, writeOnEnter, etc. are all false.
   ///
   virtual void writeNow ();

   /// Return references to the current count of disconnections.
   /// The plugin library (and therefore the static connection and disconnection counts)
   /// can be mapped twice (on Windows at least). So it is no use just referencing these
   /// static variables from an application if the widgets of interest have been created
   /// by the UI Loader. This function can be called on any widget loaded by the UI loader
   /// and the reference returned can be used to get counts for all widgets loaded by the
   /// UI loader.
   ///
   int *getDisconnectedCountRef () const;

   /// Return references to the current count of connections.
   /// The plugin library (and therefore the static connection and disconnection counts)
   /// can be mapped twice (on Windows at least). So it is no use just referencing these
   /// static variables from an application if the widgets of interest have been created
   /// by the UI Loader. This function can be called on any widget loaded by the UI loader
   /// and the reference returned can be used to get counts for all widgets loaded by the
   /// UI loader.
   ///
   int* getConnectedCountRef () const;

protected:
   void setNumVariables (const unsigned int numVariablesIn);                   ///< Set the number of variables that will stream data updates to the widget. Default of 1 if not called.
   unsigned int getNumVariables () const;                                      ///< Get the number of variables streaming data updates to the widget.

   bool subscribe;                                                             ///< Flag if data updates should be requested (default value used by QEWidget)

   qcaobject::QCaObject* createVariable (const unsigned int variableIndex,     ///< Create a CA connection. do_subscribe indicated if updates should be requested.
                                         const bool do_subscribe);             ///< Return a QCaObject if successfull.

   virtual qcaobject::QCaObject* createQcaItem (unsigned int variableIndex);   ///< Function to create a appropriate superclass of QCaObject to stream data updates
   virtual void establishConnection (unsigned int variableIndex);              ///< Create a CA connection and initiates updates if required
   virtual void activated ();                                                  ///< Do any post-all-widgets-constructed, i.e. activated stuff
   virtual void deactivated ();                                                ///< Do any post deactivated stuff

   void deleteQcaItem (const unsigned int variableIndex,                       ///< Delete a stream of CA updates
                       const bool disconnect);

private:
   void clearQcaItems();             // Deallocate and free all QCaObjects.

   typedef QList<qcaobject::QCaObject*> ChannelLists;
   ChannelLists qcaItemList;         // CA/PV access - provides a stream of updates. One for each variable name used by the QE widgets
};

#endif  // QE_VARIABLE_MANAGER_H
