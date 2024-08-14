/*  QERecipe.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2024 Australian Synchrotron
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
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
 *    Ricardo Fernandes
 *  Contact details:
 *    ricardo.fernandes@synchrotron.org.au
 */

#ifndef QE_RECIPE_H
#define QE_RECIPE_H

#include <QEEnums.h>
#include <QEConfiguredLayout.h>
#include <QDomDocument>


// ============================================================
//  QERECIPE METHODS
// ============================================================
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QERecipe:
      public QWidget, public QEWidget
{

   Q_OBJECT

private:

protected:
   QLabel *qLabelRecipeDescription;
   QComboBox *qComboBoxRecipeList;
   QPushButton *qPushButtonNew;
   QPushButton *qPushButtonSave;
   QPushButton *qPushButtonDelete;
   QPushButton *qPushButtonApply;
   QPushButton *qPushButtonRead;
   QEConfiguredLayout *qEConfiguredLayoutRecipeFields;
   QDomDocument document;
   QString recipeFile;
   QString filename;
   QE::LayoutOptions optionsLayout;
   QE::UserLevels currentUserType;

public:

   QERecipe(QWidget *pParent = 0);
   virtual ~QERecipe(){}

   void setRecipeDescription(QString pValue);
   QString getRecipeDescription();

   void setShowRecipeList(bool pValue);
   bool getShowRecipeList();

   void setShowNew(bool pValue);
   bool getShowNew();

   void setShowSave(bool pValue);
   bool getShowSave();

   void setShowDelete(bool pValue);
   bool getShowDelete();

   void setShowApply(bool pValue);
   bool getShowApply();

   void setShowRead(bool pValue);
   bool getShowRead();

   void setShowFields(bool pValue);
   bool getShowFields();

   void setConfigurationType(QE::SourceOptions pValue);
   QE::SourceOptions getConfigurationType();

   void setConfigurationFile(QString pValue);
   QString getConfigurationFile();

   void setRecipeFile(QString pValue);
   QString getRecipeFile();

   void setConfigurationText(QString pValue);
   QString getConfigurationText();

   void setOptionsLayout(QE::LayoutOptions pValue);
   QE::LayoutOptions getOptionsLayout();

   void setCurrentUserType(QE::UserLevels pValue);
   QE::UserLevels getCurrentUserType();

   bool saveRecipeList();

   void refreshRecipeList();

   void refreshButton();

   void userLevelChanged(QE::UserLevels pValue);


   Q_PROPERTY(QString recipeDescription READ getRecipeDescription WRITE setRecipeDescription)

   Q_PROPERTY(bool showRecipeList READ getShowRecipeList WRITE setShowRecipeList)

   Q_PROPERTY(bool showNew READ getShowNew WRITE setShowNew)

   Q_PROPERTY(bool showSave READ getShowSave WRITE setShowSave)

   Q_PROPERTY(bool showDelete READ getShowDelete WRITE setShowDelete)

   Q_PROPERTY(bool showApply READ getShowApply WRITE setShowApply)

   Q_PROPERTY(bool showRead READ getShowRead WRITE setShowRead)

   Q_PROPERTY(bool showFields READ getShowFields WRITE setShowFields)

   Q_PROPERTY(QString recipeFile READ getRecipeFile WRITE setRecipeFile)


   Q_PROPERTY(QE::SourceOptions configurationType READ getConfigurationType WRITE setConfigurationType)

   Q_PROPERTY(QString configurationFile READ getConfigurationFile WRITE setConfigurationFile)

   Q_PROPERTY(QString configurationText READ getConfigurationText WRITE setConfigurationText)

   Q_PROPERTY(QE::LayoutOptions optionsLayout READ getOptionsLayout WRITE setOptionsLayout)

   Q_PROPERTY(QE::UserLevels currentUserType READ getCurrentUserTypeProperty WRITE setCurrentUserTypeProperty)

   void setCurrentUserTypeProperty(QE::UserLevels pUserType)
   {
      this->setCurrentUserType(pUserType);
   }

   QE::UserLevels getCurrentUserTypeProperty()
   {
      return this->getCurrentUserType();
   }


private slots:
   void comboBoxRecipeSelected(int);

   void buttonNewClicked();

   void buttonSaveClicked();

   void buttonDeleteClicked();

   void buttonApplyClicked();

   void buttonReadClicked();
};

#endif // QE_RECIPE_H
