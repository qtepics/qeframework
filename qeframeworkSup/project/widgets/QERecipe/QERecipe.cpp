/*  QERecipe.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at
 *  the Australian Synchrotron.
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

#include "QERecipe.h"
#include <QDebug>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>

#define DEBUG qDebug () << "QERecipe" << __LINE__ << __FUNCTION__ << "  "

// =============================================================================
//  QERECIPE METHODS
// =============================================================================
//
QERecipe::QERecipe(QWidget *pParent):QWidget(pParent), QEWidget(this)
{
   qLabelRecipeDescription = new QLabel(this);
   qComboBoxRecipeList = new QComboBox(this);
   qPushButtonNew = new QPushButton(this);
   qPushButtonSave = new QPushButton(this);
   qPushButtonDelete = new QPushButton(this);
   qPushButtonApply = new QPushButton(this);
   qPushButtonRead = new QPushButton(this);
   qEConfiguredLayoutRecipeFields = new QEConfiguredLayout(this, false);


   qComboBoxRecipeList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
   qComboBoxRecipeList->setToolTip("Select recipe");
   QObject::connect(qComboBoxRecipeList, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxRecipeSelected(int)));

   qPushButtonNew->setText("New");
   qPushButtonNew->setToolTip("Create new recipe");
   QObject::connect(qPushButtonNew, SIGNAL(clicked()), this, SLOT(buttonNewClicked()));

   qPushButtonSave->setText("Save");
   qPushButtonSave->setToolTip("Save values in the selected recipe");
   qPushButtonSave->setEnabled(false);
   QObject::connect(qPushButtonSave, SIGNAL(clicked()), this, SLOT(buttonSaveClicked()));

   qPushButtonDelete->setText("Delete");
   qPushButtonDelete->setToolTip("Delete selected recipe");
   qPushButtonDelete->setEnabled(false);
   QObject::connect(qPushButtonDelete, SIGNAL(clicked()), this, SLOT(buttonDeleteClicked()));

   qPushButtonApply->setText("Apply");
   qPushButtonApply->setToolTip("Apply values to process variables");
   qPushButtonApply->setEnabled(false);
   QObject::connect(qPushButtonApply, SIGNAL(clicked()), this, SLOT(buttonApplyClicked()));

   qPushButtonRead->setText("Read");
   qPushButtonRead->setToolTip("Read values from process variables");
   QObject::connect(qPushButtonRead, SIGNAL(clicked()), this, SLOT(buttonReadClicked()));

   qEConfiguredLayoutRecipeFields->setShowItemList(false);

   setRecipeFile("");
   setConfigurationFile("");
   setConfigurationText("");
   setConfigurationType(QE::SourceFile);
   setShowRecipeList(true);
   setOptionsLayout(QE::Top);
   setCurrentUserType(getUserLevel());
}

//---------------------------------------------------------------------------------
//
void QERecipe::setRecipeDescription(QString pValue)
{
   qLabelRecipeDescription->setText(pValue);
   qLabelRecipeDescription->setVisible(qLabelRecipeDescription->text().isEmpty() == false);
}

//---------------------------------------------------------------------------------
//
QString QERecipe::getRecipeDescription()
{
   return qLabelRecipeDescription->text();
}

//---------------------------------------------------------------------------------
//
void QERecipe::setShowRecipeList(bool pValue)
{
   qComboBoxRecipeList->setVisible(pValue);
}

//---------------------------------------------------------------------------------
//
bool QERecipe::getShowRecipeList()
{
   return qComboBoxRecipeList->isVisible();
}

//---------------------------------------------------------------------------------
//
void QERecipe::setShowNew(bool pValue)
{
   qPushButtonNew->setVisible(pValue);
}

//---------------------------------------------------------------------------------
//
bool QERecipe::getShowNew()
{
   return qPushButtonNew->isVisible();
}

//---------------------------------------------------------------------------------
//
void QERecipe::setShowSave(bool pValue)
{
   qPushButtonSave->setVisible(pValue);
}

//---------------------------------------------------------------------------------
//
bool QERecipe::getShowSave()
{
   return qPushButtonSave->isVisible();
}

//---------------------------------------------------------------------------------
//
void QERecipe::setShowDelete(bool pValue)
{
   qPushButtonDelete->setVisible(pValue);
}

//---------------------------------------------------------------------------------
//
bool QERecipe::getShowDelete()
{
   return qPushButtonDelete->isVisible();
}

//---------------------------------------------------------------------------------
//
void QERecipe::setShowApply(bool pValue)
{
   qPushButtonApply->setVisible(pValue);
}

//---------------------------------------------------------------------------------
//
bool QERecipe::getShowApply()
{
   return qPushButtonApply->isVisible();
}

//---------------------------------------------------------------------------------
//
void QERecipe::setShowRead(bool pValue)
{
   qPushButtonRead->setVisible(pValue);
}

//---------------------------------------------------------------------------------
//
bool QERecipe::getShowRead()
{
   return qPushButtonRead->isVisible();
}

//---------------------------------------------------------------------------------
//
void QERecipe::setShowFields(bool pValue)
{
   qEConfiguredLayoutRecipeFields->setVisible(pValue);
}

//---------------------------------------------------------------------------------
//
bool QERecipe::getShowFields()
{
   return qEConfiguredLayoutRecipeFields->isVisible();
}

//---------------------------------------------------------------------------------
//
void QERecipe::setConfigurationType(QE::SourceOptions pValue)
{
   qEConfiguredLayoutRecipeFields->setConfigurationType(pValue);
}

//---------------------------------------------------------------------------------
//
QE::SourceOptions QERecipe::getConfigurationType()
{
   return qEConfiguredLayoutRecipeFields->getConfigurationType();
}

//---------------------------------------------------------------------------------
//
void QERecipe::setConfigurationFile(QString pValue)
{
   qEConfiguredLayoutRecipeFields->setConfigurationFile(pValue);
}

//---------------------------------------------------------------------------------
//
QString QERecipe::getConfigurationFile()
{
   return qEConfiguredLayoutRecipeFields->getConfigurationFile();
}

//---------------------------------------------------------------------------------
//
void QERecipe::setConfigurationText(QString pValue)
{
   qEConfiguredLayoutRecipeFields->setConfigurationText(pValue);
}

//---------------------------------------------------------------------------------
//
QString QERecipe::getConfigurationText()
{
   return qEConfiguredLayoutRecipeFields->getConfigurationText();
}

//---------------------------------------------------------------------------------
//
void QERecipe::setRecipeFile(QString pValue)
{
   QDomElement rootElement;
   QFile *file;
   QString data;
   bool flag;

   recipeFile = pValue;
   document.clear();

   if (recipeFile.isEmpty())
   {
      QFileInfo fileInfo;
      fileInfo.setFile( defaultFileLocation(), QString( "QERecipe.xml" ) );
      filename = fileInfo.filePath();
   }
   else
   {
      filename = recipeFile;
   }

   file = openQEFile( filename, (QIODevice::OpenModeFlag)((int)(QFile::ReadOnly | QFile::Text)) );
   if (file)
   {
      data = file->readAll();
      file->close();
      flag = static_cast<bool>(document.setContent(data));
   }
   else
   {
      flag = false;
   }

   if (flag)
   {
      refreshRecipeList();
   }
   else
   {
      rootElement = document.createElement("epicsqt");
      document.appendChild(rootElement);
   }
}

//---------------------------------------------------------------------------------
//
QString QERecipe::getRecipeFile()
{
   return recipeFile;
}

//---------------------------------------------------------------------------------
//
void QERecipe::setOptionsLayout(QE::LayoutOptions pValue)
{
   QLayout *qLayoutMain;
   QLayout *qLayoutChild;

   delete layout();

   //TODO: fix issue of buttons not being centered when using LEFT and RIGHT layout

   switch(pValue)
   {
      case QE::Top:
         optionsLayout = QE::Top;
         qLayoutMain = new QVBoxLayout(this);
         qLayoutChild = new QHBoxLayout();
         qLayoutChild->addWidget(qLabelRecipeDescription);
         qLayoutChild->addWidget(qComboBoxRecipeList);
         qLayoutChild->addWidget(qPushButtonNew);
         qLayoutChild->addWidget(qPushButtonSave);
         qLayoutChild->addWidget(qPushButtonDelete);
         qLayoutChild->addWidget(qPushButtonApply);
         qLayoutChild->addWidget(qPushButtonRead);
         qLayoutMain->addItem(qLayoutChild);
         qLayoutMain->addWidget(qEConfiguredLayoutRecipeFields);
         break;

      case QE::Bottom:
         optionsLayout = QE::Bottom;
         qLayoutMain = new QVBoxLayout(this);
         qLayoutMain->addWidget(qEConfiguredLayoutRecipeFields);
         qLayoutChild = new QHBoxLayout();
         qLayoutChild->addWidget(qLabelRecipeDescription);
         qLayoutChild->addWidget(qComboBoxRecipeList);
         qLayoutChild->addWidget(qPushButtonNew);
         qLayoutChild->addWidget(qPushButtonSave);
         qLayoutChild->addWidget(qPushButtonDelete);
         qLayoutChild->addWidget(qPushButtonApply);
         qLayoutChild->addWidget(qPushButtonRead);
         qLayoutMain->addItem(qLayoutChild);
         break;

      case QE::Left:
         optionsLayout = QE::Left;
         qLayoutMain = new QHBoxLayout(this);
         qLayoutChild = new QVBoxLayout();
         qLayoutChild->addWidget(qLabelRecipeDescription);
         qLayoutChild->addWidget(qComboBoxRecipeList);
         qLayoutChild->addWidget(qPushButtonNew);
         qLayoutChild->addWidget(qPushButtonSave);
         qLayoutChild->addWidget(qPushButtonDelete);
         qLayoutChild->addWidget(qPushButtonApply);
         qLayoutChild->addWidget(qPushButtonRead);
         qLayoutMain->addItem(qLayoutChild);
         qLayoutMain->addWidget(qEConfiguredLayoutRecipeFields);
         break;

      case QE::Right:
         optionsLayout = QE::Right;
         qLayoutMain = new QHBoxLayout(this);
         qLayoutChild = new QVBoxLayout();
         qLayoutChild->addWidget(qLabelRecipeDescription);
         qLayoutChild->addWidget(qComboBoxRecipeList);
         qLayoutChild->addWidget(qPushButtonNew);
         qLayoutChild->addWidget(qPushButtonSave);
         qLayoutChild->addWidget(qPushButtonDelete);
         qLayoutChild->addWidget(qPushButtonApply);
         qLayoutChild->addWidget(qPushButtonRead);
         qLayoutMain->addWidget(qEConfiguredLayoutRecipeFields);
         qLayoutMain->addItem(qLayoutChild);
   }
}

//---------------------------------------------------------------------------------
//
QE::LayoutOptions QERecipe::getOptionsLayout()
{
   return optionsLayout;
}

//---------------------------------------------------------------------------------
//
void QERecipe::setCurrentUserType(QE::UserLevels pValue)
{
   if (pValue == QE::User || pValue == QE::Scientist || pValue == QE::Engineer)
   {
      this->currentUserType = pValue;
      refreshRecipeList();
      qEConfiguredLayoutRecipeFields->setCurrentUserType(currentUserType);
   }
}

//---------------------------------------------------------------------------------
//
QE::UserLevels QERecipe::getCurrentUserType()
{
   return this->currentUserType;
}

//---------------------------------------------------------------------------------
//
void QERecipe::comboBoxRecipeSelected(int)
{
   refreshButton();
}

//---------------------------------------------------------------------------------
//
void QERecipe::buttonNewClicked()
{
   QDomElement rootElement;
   QDomElement recipeElement;
   QDomElement processVariableElement;
   QDomNode rootNode;
   _Field *fieldInfo;
   QString currentName;
   QString name;
   QString visible;
   bool flag;
   int count;
   int i;

   do
   {
      name = QInputDialog::getText(this, "New Recipe", "Name:", QLineEdit::Normal , "", &flag);
   }
   while(flag && name.isEmpty());

   if (name.isEmpty() == false)
   {
      flag = true;
      count = 0;
      rootElement = document.documentElement();
      if (rootElement.tagName() == "epicsqt")
      {
         rootNode = rootElement.firstChild();
         while (rootNode.isNull() == false)
         {
            recipeElement = rootNode.toElement();
            if (recipeElement.tagName() == "recipe")
            {
               if (recipeElement.attribute("name").isEmpty())
               {
                  currentName = "Recipe #" + QString::number(count);
                  count++;
               }
               else
               {
                  currentName = recipeElement.attribute("name");
               }
               if (currentName.compare(name) == 0)
               {
                  flag = false;
                  break;
               }
            }
            rootNode = rootNode.nextSibling();
         }
      }
      if (flag == false)
      {
         visible = recipeElement.attribute("visible").toUpper();
         if (visible.isEmpty())
         {
            flag = true;
         }
         else if (visible == "USER")
         {
            flag = true;
         }
         else if (visible == "SCIENTIST")
         {
            flag = (this->currentUserType > 0);
         }
         else if (visible == "ENGINEER")
         {
            flag = (this->currentUserType > 1);
         }
         else
         {
            flag = false;
         }
         if (flag)
         {
            flag = (QMessageBox::question(this, "Info", "Do you want to replace existing recipe '" + name + "'?",
                                          QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes);
            if (flag)
            {
               rootElement.removeChild(rootNode);
            }
         }
         else
         {
            QMessageBox::warning(this, "Warning", "Unable to create recipe '" + name +
                                 "' since it already exists and belongs to another user type with more priviledges!");
         }
      }
      else
      {
         switch (currentUserType)
         {
            case QE::User:
               visible = "USER";
               break;
            case QE::Scientist:
               visible = "SCIENTIST";
               break;
            default:
               visible = "ENGINEER";
         }
      }
      if (flag)
      {
         recipeElement = document.createElement("recipe");
         recipeElement.setAttribute("name", name);
         recipeElement.setAttribute("visible", visible);
         for(i = 0; i < qEConfiguredLayoutRecipeFields->currentFieldList.size(); i++)
         {
            fieldInfo = qEConfiguredLayoutRecipeFields->currentFieldList.at(i);
            processVariableElement = document.createElement("processvariable");
            processVariableElement.setAttribute("name", fieldInfo->getProcessVariable());
            if (fieldInfo->getType() == QEConfiguredLayout::BUTTON)
            {
            }
            else if (fieldInfo->getType() == QEConfiguredLayout::LABEL)
            {
            }
            else if (fieldInfo->getType() == QEConfiguredLayout::SPINBOX)
            {
               processVariableElement.setAttribute("value", ((QESpinBox *) fieldInfo->qeWidget)->text());
            }
            else if (fieldInfo->getType() == QEConfiguredLayout::COMBOBOX)
            {
               processVariableElement.setAttribute("value", ((QEComboBox *) fieldInfo->qeWidget)->currentText());
            }
            else
            {
               processVariableElement.setAttribute("value", ((QELineEdit *) fieldInfo->qeWidget)->text());
            }
            recipeElement.appendChild(processVariableElement);
         }
         rootElement.appendChild(recipeElement);
         if (saveRecipeList())
         {
            QMessageBox::information(this, "Info", "The recipe '" + name + "' was successfully created!");
         }
         else
         {
            // TODO: restore original document if there is an error
            //rootElement.removeChild(recipeElement);
            QMessageBox::critical(this, "Error", "Unable to create recipe '" + name +
                                  "' in file '" + filename + "'!");
         }
      }
   }
}

//---------------------------------------------------------------------------------
//
void QERecipe::buttonSaveClicked()
{
   QDomElement rootElement;
   QDomElement recipeElement;
   QDomElement processVariableElement;
   QDomNode rootNode;
   _Field *fieldInfo;
   QString currentName;
   QString name;
   int count;
   int i;

   currentName = qComboBoxRecipeList->currentText();

   if (QMessageBox::question(this, "Info", "Do you want to save the values in recipe '" + currentName + "'?",
                             QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
   {
      count = 0;
      rootElement = document.documentElement();
      if (rootElement.tagName() == "epicsqt")
      {
         rootNode = rootElement.firstChild();
         while (rootNode.isNull() == false)
         {
            recipeElement = rootNode.toElement();
            if (recipeElement.tagName() == "recipe")
            {
               if (recipeElement.attribute("name").isEmpty())
               {
                  name= "Recipe #" + QString::number(count);
                  count++;
               }
               else
               {
                  name = recipeElement.attribute("name");
               }
               if (currentName.compare(name) == 0)
               {
                  break;
               }
            }
            rootNode = rootNode.nextSibling();
         }
      }

      while (recipeElement.hasChildNodes())
      {
         recipeElement.removeChild(recipeElement.lastChild());
      }

      for(i = 0; i < qEConfiguredLayoutRecipeFields->currentFieldList.size(); i++)
      {
         fieldInfo = qEConfiguredLayoutRecipeFields->currentFieldList.at(i);
         processVariableElement = document.createElement("processvariable");
         processVariableElement.setAttribute("name", fieldInfo->getProcessVariable());
         if (fieldInfo->getType() == QEConfiguredLayout::BUTTON)
         {
         }
         else if (fieldInfo->getType() == QEConfiguredLayout::LABEL)
         {
         }
         else if (fieldInfo->getType() == QEConfiguredLayout::SPINBOX)
         {
            processVariableElement.setAttribute("value", ((QESpinBox *) fieldInfo->qeWidget)->text());
         }
         else if (fieldInfo->getType() == QEConfiguredLayout::COMBOBOX)
         {
            processVariableElement.setAttribute("value", ((QEComboBox *) fieldInfo->qeWidget)->currentText());
         }
         else
         {
            processVariableElement.setAttribute("value", ((QELineEdit *) fieldInfo->qeWidget)->text());
         }
         recipeElement.appendChild(processVariableElement);
      }

      if (saveRecipeList())
      {
         QMessageBox::information(this, "Info", "The recipe '" + currentName +
                                  "' was successfully saved!");
      }
      else
      {
         // TODO: restore original document if there is an error
         QMessageBox::critical(this, "Error", "Unable to save recipe '" + currentName +
                               "' in file '" + filename + "'!");
      }
   }
}

//---------------------------------------------------------------------------------
//
void QERecipe::buttonDeleteClicked()
{
   QDomElement rootElement;
   QDomElement recipeElement;
   QDomNode rootNode;
   QString currentName;
   QString name;
   int count;

   currentName = qComboBoxRecipeList->currentText();

   if (QMessageBox::question(this, "Info", "Do you want to delete recipe '" + currentName + "'?",
                             QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
   {
      count = 0;
      rootElement = document.documentElement();
      if (rootElement.tagName() == "epicsqt")
      {
         rootNode = rootElement.firstChild();
         while (rootNode.isNull() == false)
         {
            recipeElement = rootNode.toElement();
            if (recipeElement.tagName() == "recipe")
            {
               if (recipeElement.attribute("name").isEmpty())
               {
                  name = "Recipe #" + QString::number(count);
                  count++;
               }
               else
               {
                  name = recipeElement.attribute("name");
               }
               if (currentName.compare(name) == 0)
               {
                  rootElement.removeChild(rootNode);
                  break;
               }
            }
            rootNode = rootNode.nextSibling();
         }
      }
      if (saveRecipeList())
      {
         QMessageBox::information(this, "Info", "The recipe '" + currentName +
                                  "' was successfully delete!");
      }
      else
      {
         // TODO: restore original document if there is an error
         QMessageBox::critical(this, "Error", "Unable to delete recipe '" + currentName +
                               "' in file '" + filename + "'!");
      }
   }
}

//---------------------------------------------------------------------------------
//
void QERecipe::buttonApplyClicked()
{
   _Field *fieldInfo;
   int i;

   if (QMessageBox::question(this, "Info", "Do you want to apply recipe '" +
                             qComboBoxRecipeList->currentText() + "' to process variables?",
                             QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
   {
      for(i = 0; i < qEConfiguredLayoutRecipeFields->currentFieldList.size(); i++)
      {
         fieldInfo = qEConfiguredLayoutRecipeFields->currentFieldList.at(i);
         if (fieldInfo->getVisibility())
         {
            fieldInfo->qeWidget->writeNow();
         }
      }
      QMessageBox::information(this, "Info", "The recipe '" + qComboBoxRecipeList->currentText() +
                               "' was successfully applied to process variables!");
   }
}

//---------------------------------------------------------------------------------
//
void QERecipe::buttonReadClicked()
{
   _Field *fieldInfo;
   int i;

   if (QMessageBox::question(this, "Info", "Do you want to read the values from process variables?",
                             QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
   {
      for(i = 0; i < qEConfiguredLayoutRecipeFields->currentFieldList.size(); i++)
      {
         fieldInfo = qEConfiguredLayoutRecipeFields->currentFieldList.at(i);
         if (fieldInfo->getVisibility())
         {
            fieldInfo->qeWidget->readNow();
         }
      }
      QMessageBox::information(this, "Info", "The values were successfully read from the process variables!");
   }
}

//---------------------------------------------------------------------------------
//
bool QERecipe::saveRecipeList()
{
   QFile *file;

   file = new QFile(filename);
   if (file->open(QFile::WriteOnly | QFile::Text))
   {
      QTextStream stream(file);
      document.save(stream, 3);
      file->close();
      refreshRecipeList();
      return true;
   }
   else
   {
      return false;
   }
}

//---------------------------------------------------------------------------------
//
void QERecipe::refreshRecipeList()
{
   QDomElement rootElement;
   QDomElement recipeElement;
   QDomNode rootNode;
   QString visible;
   QString tmp;
   bool flag;
   int count;
   int i;

   qComboBoxRecipeList->blockSignals(true);
   tmp = qComboBoxRecipeList->currentText();
   qComboBoxRecipeList->clear();
   rootElement = document.documentElement();
   if (rootElement.tagName() == "epicsqt")
   {
      count = 0;
      rootNode = rootElement.firstChild();
      while (rootNode.isNull() == false)
      {
         recipeElement = rootNode.toElement();
         if (recipeElement.tagName() == "recipe")
         {
            visible = recipeElement.attribute("visible").toUpper();
            if (visible.isEmpty())
            {
               flag = true;
            }
            else
            {
               if (visible == "USER")
               {
                  flag = true;
               }
               else if (visible == "SCIENTIST")
               {
                  flag = (this->currentUserType > 0);
               }
               else if (visible == "ENGINEER")
               {
                  flag = (this->currentUserType > 1);
               }
               else
               {
                  flag = false;
               }
            }
            if (flag)
            {
               if (recipeElement.attribute("name").isEmpty())
               {
                  qComboBoxRecipeList->addItem("Recipe #" + QString::number(count));
                  count++;
               }
               else
               {
                  qComboBoxRecipeList->addItem(recipeElement.attribute("name"));
               }
            }
         }
         rootNode = rootNode.nextSibling();
      }
   }
   i = qComboBoxRecipeList->findText(tmp);
   if (i == -1)
   {
      qComboBoxRecipeList->setCurrentIndex(0);
   }
   else
   {
      qComboBoxRecipeList->setCurrentIndex(i);
   }
   refreshButton();
   qComboBoxRecipeList->blockSignals(false);
}

//---------------------------------------------------------------------------------
//
void QERecipe::refreshButton()
{
   QDomElement rootElement;
   QDomElement recipeElement;
   QDomElement processVariableElement;
   QDomNode rootNode;
   _Field *fieldInfo;
   QString currentName;
   QString name;
   int count;
   int i;

   currentName = qComboBoxRecipeList->currentText();

   //    qDebug() << "recipe: " + currentName;

   count = 0;
   rootElement = document.documentElement();
   if (rootElement.tagName() == "epicsqt")
   {
      rootNode = rootElement.firstChild();
      while (rootNode.isNull() == false)
      {
         recipeElement = rootNode.toElement();
         if (recipeElement.tagName() == "recipe")
         {
            if (recipeElement.attribute("name").isEmpty())
            {
               name = "Recipe #" + QString::number(count);
               count++;
            }
            else
            {
               name = recipeElement.attribute("name");
            }

            if (currentName.compare(name) == 0)
            {
               for(i = 0; i < qEConfiguredLayoutRecipeFields->currentFieldList.size(); i++)
               {
                  fieldInfo = qEConfiguredLayoutRecipeFields->currentFieldList.at(i);

                  if (fieldInfo->getVisibility())
                  {
                     rootNode = recipeElement.firstChild();
                     while (rootNode.isNull() == false)
                     {
                        processVariableElement = rootNode.toElement();
                        if (processVariableElement.tagName() == "processvariable")
                        {
                           if (fieldInfo->getProcessVariable() == processVariableElement.attribute("name"))
                           {

                              if (fieldInfo->getType() == QEConfiguredLayout::BUTTON)
                              {
                              }
                              else if (fieldInfo->getType() == QEConfiguredLayout::LABEL)
                              {
                              }
                              else if (fieldInfo->getType() == QEConfiguredLayout::SPINBOX)
                              {
                                 //((QESpinBox *) fieldInfo->qeWidget)->setValue((float) processVariableElement.attribute("value"));
                              }
                              else if (fieldInfo->getType() == QEConfiguredLayout::COMBOBOX)
                              {
                                 ((QEComboBox *) fieldInfo->qeWidget)->setEditText(processVariableElement.attribute("value"));
                              }
                              else
                              {
                                 ((QELineEdit *) fieldInfo->qeWidget)->setText(processVariableElement.attribute("value"));
                              }
                              break;
                           }
                        }
                        rootNode = rootNode.nextSibling();
                     }
                  }

               }

               break;
            }
         }
         rootNode = rootNode.nextSibling();
      }
   }

   qPushButtonSave->setEnabled(qComboBoxRecipeList->currentText().isEmpty() == false);
   qPushButtonDelete->setEnabled(qComboBoxRecipeList->currentText().isEmpty() == false);
   qPushButtonApply->setEnabled(qComboBoxRecipeList->currentText().isEmpty() == false);
}

//---------------------------------------------------------------------------------
//
void QERecipe::userLevelChanged(QE::UserLevels pValue)
{
   setCurrentUserType(pValue);
}

// end
