/*  QESubstitutedLabel.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2021 Australian Synchrotron
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
  This class is based on the Qt label widget. It is NOT a CA aware widget.
  It does, however, use the QEWidget and its VariableNameManager base class to manage substituting
  the label's text in the same way most other widgets manage substitutions in variable names.
  It is tighly integrated with the base class QEWidget. Refer to QEWidget.cpp for details, but watch
  out for references to varable names when it is actually just talking about the text that will be
  displayed in the QLabel widget.
  This widget is required to implement some virtual functions required by QEWidget for
  managing data connections. These functions are just stubs.
 */

#ifndef QE_SUBSTITUTED_LABEL_H
#define QE_SUBSTITUTED_LABEL_H

#include <QLabel>
#include <QEWidget.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEFrameworkLibraryGlobal.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QESubstitutedLabel : public QLabel, public QEWidget {
    Q_OBJECT

public:
    QESubstitutedLabel( QWidget *parent = 0 );

    // Property convenience functions

    // label text (prior to substitution)
    void setLabelTextProperty( QString labelTextIn );
    QString getLabelTextProperty();

    // Macro substitions
    void setSubstitutionsProperty( QString macroSubstitutionsIn );
    QString getSubstitutionsProperty();


    // label text (prior to substitution) with line feeds encoded in C style to allow entry in designer
    QString getLabelTextPropertyFormat();
    void setLabelTextPropertyFormat( QString labelTextIn );


protected:
    QString labelText;                                                 // Fixed text to which substitutions will be applied


private:
    void setup();
    qcaobject::QCaObject* createQcaItem( unsigned int ){ return NULL; } // Not used as this widget does not connect to any data source

public:
    //=================================================================================
    // NOTE, this plugin uses the variable-name-and-substitutions mechanism normally used to manage variable names
    //       to manage text displayed in the label.
    //       If you are reading this because you searched for SINGLEVARIABLEPROPERTIES, this section is not the standard set normally
    //       used for all widgets with a single variable, but some changes may be needed here as well

    // Make the text proerty non-designable. This both hides the text property within
    // designer and stops the text value from being written to the .ui file.
    // And stop users accidentally setting wrong propery.
    //
    Q_PROPERTY(QString text READ text WRITE setText DESIGNABLE false)

    /// Label text to be substituted.
    /// This text will be copied to the label text after applying any macro substitutions from the textSubstitutions property
    Q_PROPERTY(QString labelText READ getLabelTextPropertyFormat WRITE setLabelTextPropertyFormat)

    /// Text substitutions.
    /// These substitutions are applied to the 'labelText' property prior to copying it to the label text.
    Q_PROPERTY(QString textSubstitutions READ getSubstitutionsProperty WRITE setSubstitutionsProperty)

private:
    // No standard variaable name and substitutions management required
    //=================================================================================
};

#endif // QE_SUBSTITUTED_LABEL_H
