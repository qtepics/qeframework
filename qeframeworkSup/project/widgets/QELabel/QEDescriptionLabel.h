/*  QEDescriptionLabel.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2018-2025 Australian Synchrotron
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
 *    andrews@ansto.gov.au
 */

#ifndef QE_DESCRIPTION_LABEL_H
#define QE_DESCRIPTION_LABEL_H

#include <QWidget>
#include <QString>
#include <QEEnums.h>
#include <QELabel.h>
#include <QEFrameworkLibraryGlobal.h>

/// This is just a QELabel, but with some different default property values.
/// These are:
/// a) Clear style sheet and default style.
/// b) The indent set to -1, the QLabel default
/// c) Set display alarm option to DISPLAY_ALARM_STATE_NEVER
/// d) Set Font size to 8.
///
/// Note: there is one additional property: enableDots.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEDescriptionLabel : public QELabel {
   Q_OBJECT

   /// When true, text is right padded with trailing " .  .  .  . " to right-hand
   /// edge of the label.
   /// When false, no additional dot and spaces text is added.
   /// The default enableDots property value is false.
   //
   Q_PROPERTY (bool enableDots READ getEnableDots WRITE setEnableDots)

public:
   /// Create without a variable.
   /// Use setVariableNameProperty() and setSubstitutionsProperty() to define a variable
   /// and, optionally, macro substitutions later.
   ///
   explicit QEDescriptionLabel (QWidget* parent = 0);

   /// Create with a variable.
   /// A connection is automatically established.
   /// If macro substitutions are required, create without a variable and set the
   /// variable and macro substitutions after creation.
   ///
   explicit QEDescriptionLabel (const QString &variableName, QWidget* parent = 0);

   /// Destructor
   ///
   ~QEDescriptionLabel ();

   void setEnableDots (const bool enable);
   bool getEnableDots () const;

private:
   // Common setup.
   //
   void descrptionLabelSetup ();

   bool m_enableDots;

private slots:
   void dbValueChanged (const QString& value);
};

#endif  // QE_DESCRIPTION_LABEL_H
