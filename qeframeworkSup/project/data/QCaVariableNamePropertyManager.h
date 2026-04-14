/*  QCaVariableNamePropertyManager.h
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

// Handles variable name sets.

#ifndef QE_VARIABLE_NAME_PROPERTY_MANAGER_H
#define QE_VARIABLE_NAME_PROPERTY_MANAGER_H

#include <QTimer>
#include <QString>
#include <QEFrameworkLibraryGlobal.h>

// Structure used in signal to indicate new PV name properties.
//
struct QEPvNameProperties {
   QString pvName;
   QString substitutions;
   unsigned int index;
};

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QCaVariableNamePropertyManager : public QTimer {
   Q_OBJECT

public:
   QCaVariableNamePropertyManager();
   ~QCaVariableNamePropertyManager();

   QString getVariableNameProperty() const;
   void setVariableNameProperty( QString variableNamePropertyIn );

   QString getSubstitutionsProperty() const;
   void setSubstitutionsProperty( QString substitutionsPropertyIn );

   void setVariableIndex( unsigned int variableIndexIn );
   unsigned int getVariableIndex() const;

signals:
   // new style
   void newPvNameProperties (const QEPvNameProperties& pvNameProperties);

   // old style - consider deprecated.
   void newVariableNameProperty( QString variable, QString substitutions, unsigned int variableIndex );

private slots:
   void subscribeDelayExpired();       // Called a short time after a user stops typing in 'designer'

private:
   QString variableNameProperty;
   QString substitutionsProperty;

   unsigned int variableIndex;     // Index into the list of variable names maintained by a QE widgets
   bool interactive;   // If set, there is a user typing new variable names and macro substitutions. Use timers to wait for typing to finish.
};

#endif // QE_VARIABLE_NAME_PROPERTY_MANAGER_H
