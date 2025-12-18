/*  QEDescriptionLabel.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2018-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
