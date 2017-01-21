/*  This file is part of the EPICS QT Framework, initially developed at
 *  the Australian Synchrotron.
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
 *  Copyright (c) 2012 Australian Synchrotron
 *
 *  Author:
 *    Ricardo Fernandes
 *  Contact details:
 *    ricardo.fernandes@synchrotron.org.au
 */

#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLayout>
#include <QHeaderView>
#include <QFileDialog>
#include <QComboBox>
#include <QMessageBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QInputDialog>
#include <QProcess>
#include <QCoreApplication>
#include <QEScript.h>
#include <UserMessage.h>



// ============================================================
//  QESCRIPT METHODS
// ============================================================
QEScript::QEScript(QWidget *pParent):QWidget(pParent), QEWidget( this )
{

    QFont qFont;

    qComboBoxScriptList = new QComboBox(this);

    qPushButtonNew = new QPushButton(this);
    qPushButtonSave = new QPushButton(this);
    qPushButtonDelete = new QPushButton(this);
    qPushButtonExecute = new QPushButton(this);
    qPushButtonAbort = new QPushButton(this);
    qPushButtonAdd = new QPushButton(this);
    qPushButtonRemove = new QPushButton(this);
    qPushButtonUp = new QPushButton(this);
    qPushButtonDown = new QPushButton(this);
    qPushButtonCopy = new QPushButton(this);
    qPushButtonPaste = new QPushButton(this);
    qTableWidgetScript = new _QTableWidgetScript(this);


    qComboBoxScriptList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    qComboBoxScriptList->setToolTip("Select script");
    QObject::connect(qComboBoxScriptList, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxScriptSelected(int)));

    qPushButtonNew->setText("New");
    qPushButtonNew->setToolTip("Create new script (reset table)");
    QObject::connect(qPushButtonNew, SIGNAL(clicked()), this, SLOT(buttonNewClicked()));

    qPushButtonSave->setText("Save");
    qPushButtonSave->setToolTip("Save script");
    QObject::connect(qPushButtonSave, SIGNAL(clicked()), this, SLOT(buttonSaveClicked()));

    qPushButtonDelete->setText("Delete");
    qPushButtonDelete->setToolTip("Delete selected script");
    QObject::connect(qPushButtonDelete, SIGNAL(clicked()), this, SLOT(buttonDeleteClicked()));

    qPushButtonExecute->setText("Execute");
    qPushButtonExecute->setToolTip("Execute");
    QObject::connect(qPushButtonExecute, SIGNAL(clicked()), this, SLOT(buttonExecuteClicked()));

    qPushButtonAbort->setText("Abort");
    qPushButtonAbort->setToolTip("Abort execution of program(s)");
    QObject::connect(qPushButtonAbort, SIGNAL(clicked()), this, SLOT(buttonAbortClicked()));

    qPushButtonAdd->setText("Add");
    qPushButtonAdd->setToolTip("Add row");
    QObject::connect(qPushButtonAdd, SIGNAL(clicked()), this, SLOT(buttonAddClicked()));

    qPushButtonRemove->setText("Remove");
    qPushButtonRemove->setToolTip("Remove selected row(s)");
    QObject::connect(qPushButtonRemove, SIGNAL(clicked()), this, SLOT(buttonRemoveClicked()));

    qPushButtonUp->setText("Up");
    qPushButtonUp->setToolTip("Move selected row up");
    QObject::connect(qPushButtonUp, SIGNAL(clicked()), this, SLOT(buttonUpClicked()));

    qPushButtonDown->setText("Down");
    qPushButtonDown->setToolTip("Move selected row down");
    QObject::connect(qPushButtonDown, SIGNAL(clicked()), this, SLOT(buttonDownClicked()));

    qPushButtonCopy->setText("Copy");
    qPushButtonCopy->setToolTip("Copy selected row(s)");
    QObject::connect(qPushButtonCopy, SIGNAL(clicked()), this, SLOT(buttonCopyClicked()));

    qPushButtonPaste->setText("Paste");
    qPushButtonPaste->setToolTip("Paste row(s)");
    QObject::connect(qPushButtonPaste, SIGNAL(clicked()), this, SLOT(buttonPasteClicked()));

    qTableWidgetScript->setColumnCount(8);
    qTableWidgetScript->setHorizontalHeaderItem(0, new QTableWidgetItem("#"));
    qTableWidgetScript->setHorizontalHeaderItem(1, new QTableWidgetItem("Enable"));
    qTableWidgetScript->setHorizontalHeaderItem(2, new QTableWidgetItem("Program"));
    qTableWidgetScript->setHorizontalHeaderItem(3, new QTableWidgetItem("Parameters"));
    qTableWidgetScript->setHorizontalHeaderItem(4, new QTableWidgetItem("Directory"));
    qTableWidgetScript->setHorizontalHeaderItem(5, new QTableWidgetItem("Timeout"));
    qTableWidgetScript->setHorizontalHeaderItem(6, new QTableWidgetItem("Stop"));
    qTableWidgetScript->setHorizontalHeaderItem(7, new QTableWidgetItem("Log"));
    qTableWidgetScript->setToolTip("List of programs to execute");
    qTableWidgetScript->setSelectionBehavior(QAbstractItemView::SelectRows);
    qTableWidgetScript->verticalHeader()->hide();
    qFont.setPointSize(9);
    qTableWidgetScript->setFont(qFont);
    QObject::connect(qTableWidgetScript->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(selectionChanged(const QItemSelection &, const QItemSelection &)));

    setScriptType(FROM_FILE);
    setScriptFile("");
    setScriptText("");
    setScriptDefault("");
    setOptionsLayout(TOP);
    isExecuting = false;
    refreshWidgets();
    editableTable = true;

}




void QEScript::setShowScriptList(bool pValue)
{

    qComboBoxScriptList->setVisible(pValue);

}



bool QEScript::getShowScriptList()
{

    return qComboBoxScriptList->isVisible();

}



void QEScript::setShowNew(bool pValue)
{

    qPushButtonNew->setVisible(pValue);

}



bool QEScript::getShowNew()
{

    return qPushButtonNew->isVisible();

}




void QEScript::setShowSave(bool pValue)
{

    qPushButtonSave->setVisible(pValue);

}




bool QEScript::getShowSave()
{

    return qPushButtonSave->isVisible();

}



void QEScript::setShowDelete(bool pValue)
{

    qPushButtonDelete->setVisible(pValue);

}



bool QEScript::getShowDelete()
{

    return qPushButtonDelete->isVisible();

}



void QEScript::setShowExecute(bool pValue)
{

    qPushButtonExecute->setVisible(pValue);

}



bool QEScript::getShowExecute()
{

    return qPushButtonExecute->isVisible();

}



void QEScript::setShowAbort(bool pValue)
{

    qPushButtonAbort->setVisible(pValue);

}



bool QEScript::getShowAbort()
{

    return qPushButtonAbort->isVisible();

}



void QEScript::setEditableTable(bool pValue)
{

    editableTable = pValue;

}



bool QEScript::getEditableTable()
{

    return editableTable;

}



void QEScript::setShowTable(bool pValue)
{

    qTableWidgetScript->setVisible(pValue);

}



bool QEScript::getShowTable()
{

    return qTableWidgetScript->isVisible();

}



void QEScript::setShowTableControl(bool pValue)
{

    qPushButtonAdd->setVisible(pValue);
    qPushButtonRemove->setVisible(pValue);
    qPushButtonUp->setVisible(pValue);
    qPushButtonDown->setVisible(pValue);
    qPushButtonCopy->setVisible(pValue);
    qPushButtonPaste->setVisible(pValue);

}



bool QEScript::getShowTableControl()
{

    return (qPushButtonAdd->isVisible());

}



void QEScript::setShowColumnNumber(bool pValue)
{

    qTableWidgetScript->setColumnHidden(0, pValue == false);
    qTableWidgetScript->refreshSize();

}



bool QEScript::getShowColumnNumber()
{

    return (qTableWidgetScript->isColumnHidden(0) == false);

}



void QEScript::setShowColumnEnable(bool pValue)
{

    qTableWidgetScript->setColumnHidden(1, pValue == false);
    qTableWidgetScript->refreshSize();

}



bool QEScript::getShowColumnEnable()
{

    return (qTableWidgetScript->isColumnHidden(1) == false);

}



void QEScript::setShowColumnProgram(bool pValue)
{

    qTableWidgetScript->setColumnHidden(2, pValue == false);
    qTableWidgetScript->refreshSize();

}



bool QEScript::getShowColumnProgram()
{

    return (qTableWidgetScript->isColumnHidden(2) == false);

}



void QEScript::setShowColumnParameters(bool pValue)
{

    qTableWidgetScript->setColumnHidden(3, pValue == false);
    qTableWidgetScript->refreshSize();

}



bool QEScript::getShowColumnParameters()
{

    return (qTableWidgetScript->isColumnHidden(3) == false);

}



void QEScript::setShowColumnWorkingDirectory(bool pValue)
{

    qTableWidgetScript->setColumnHidden(4, pValue == false);
    qTableWidgetScript->refreshSize();

}



bool QEScript::getShowColumnWorkingDirectory()
{

    return (qTableWidgetScript->isColumnHidden(4) == false);

}



void QEScript::setShowColumnTimeout(bool pValue)
{

    qTableWidgetScript->setColumnHidden(5, pValue == false);
    qTableWidgetScript->refreshSize();

}



bool QEScript::getShowColumnTimeout()
{

    return (qTableWidgetScript->isColumnHidden(5) == false);

}



void QEScript::setShowColumnStop(bool pValue)
{

    qTableWidgetScript->setColumnHidden(6, pValue == false);
    qTableWidgetScript->refreshSize();

}



bool QEScript::getShowColumnStop()
{

    return (qTableWidgetScript->isColumnHidden(6) == false);

}



void QEScript::setShowColumnLog(bool pValue)
{

    qTableWidgetScript->setColumnHidden(7, pValue == false);
    qTableWidgetScript->refreshSize();

}



bool QEScript::getShowColumnLog()
{

    return (qTableWidgetScript->isColumnHidden(7) == false);

}



void QEScript::setScriptType(int pValue)
{

    scriptType = pValue;
    setScriptFile(scriptFile);
    setScriptText(scriptText);

}




int QEScript::getScriptType()
{

    return scriptType;

}




void QEScript::setScriptFile(QString pValue)
{

    QDomElement rootElement;
    QFile *file;
    QString data;
    bool flag;


    scriptFile = pValue;
    if (scriptType == FROM_FILE)
    {
        document.clear();
        if (scriptFile.isEmpty())
        {
            QFileInfo fileInfo;
            fileInfo.setFile(defaultFileLocation(), "QEScript.xml");
            filename = fileInfo.filePath();
        }
        else
        {
            filename = scriptFile;
        }
        file = openQEFile(filename, (QIODevice::OpenModeFlag)((int)(QFile::ReadOnly | QFile::Text)));
        if (file)
        {
            data = file->readAll();
            file->close();
            flag = document.setContent(data);
        }
        else
        {
            flag = false;
        }
        if (flag == false)
        {
            rootElement = document.createElement("epicsqt");
            document.appendChild(rootElement);
        }
        refreshScriptList();
        qComboBoxScriptList->setCurrentIndex(-1);
    }

}



QString QEScript::getScriptFile()
{

    return scriptFile;

}




void QEScript::setScriptText(QString pValue)
{

    QDomElement rootElement;

    scriptText = pValue;
    if (scriptType == FROM_TEXT)
    {
        document.clear();
        if (document.setContent(scriptText) == false)
        {
            rootElement = document.createElement("epicsqt");
            document.appendChild(rootElement);
        }
        refreshScriptList();
        qComboBoxScriptList->setCurrentIndex(-1);
    }

}




QString QEScript::getScriptText()
{

    return scriptText;

}




void QEScript::setScriptDefault(QString pValue)
{
    bool flag;
    int i;

    scriptDefault = pValue;
    flag = true;
    for(i = 0; i < qComboBoxScriptList->count(); i++)
    {
        if (qComboBoxScriptList->itemText(i).compare(scriptDefault) == 0)
        {
            flag = false;
            qComboBoxScriptList->setCurrentIndex(i);
            break;
        }
    }
    if (flag)
    {
        while (qTableWidgetScript->rowCount() > 0)
        {
            qTableWidgetScript->removeRow(0);
        }
        qComboBoxScriptList->setCurrentIndex(-1);
    }

}





QString QEScript::getScriptDefault()
{

    return scriptDefault;

}



void QEScript::setExecuteText(QString pValue)
{

    qPushButtonExecute->setText(pValue);

}




QString QEScript::getExecuteText()
{

    return qPushButtonExecute->text();

}




void QEScript::setOptionsLayout(int pValue)
{

    QLayout *qLayoutMain;
    QLayout *qLayoutChild;

    delete layout();

    switch(pValue)
    {
        case TOP:
            optionsLayout = TOP;
            qLayoutMain = new QVBoxLayout(this);
            qLayoutChild = new QHBoxLayout();
            qLayoutChild->addWidget(qComboBoxScriptList);
            qLayoutChild->addWidget(qPushButtonNew);
            qLayoutChild->addWidget(qPushButtonSave);
            qLayoutChild->addWidget(qPushButtonDelete);
            qLayoutChild->addWidget(qPushButtonExecute);
            qLayoutChild->addWidget(qPushButtonAbort);
            qLayoutMain->addItem(qLayoutChild);
            qLayoutChild = new QHBoxLayout();
            qLayoutChild->addWidget(qPushButtonAdd);
            qLayoutChild->addWidget(qPushButtonRemove);
            qLayoutChild->addWidget(qPushButtonUp);
            qLayoutChild->addWidget(qPushButtonDown);
            qLayoutChild->addWidget(qPushButtonCopy);
            qLayoutChild->addWidget(qPushButtonPaste);
            qLayoutMain->addItem(qLayoutChild);
            qLayoutMain->addWidget(qTableWidgetScript);
            break;

        case BOTTOM:
            optionsLayout = BOTTOM;
            qLayoutMain = new QVBoxLayout(this);
            qLayoutMain->addWidget(qTableWidgetScript);
            qLayoutChild = new QHBoxLayout();
            qLayoutChild->addWidget(qComboBoxScriptList);
            qLayoutChild->addWidget(qPushButtonNew);
            qLayoutChild->addWidget(qPushButtonSave);
            qLayoutChild->addWidget(qPushButtonDelete);
            qLayoutChild->addWidget(qPushButtonExecute);
            qLayoutChild->addWidget(qPushButtonAbort);
            qLayoutMain->addItem(qLayoutChild);
            qLayoutChild = new QHBoxLayout();
            qLayoutChild->addWidget(qPushButtonAdd);
            qLayoutChild->addWidget(qPushButtonRemove);
            qLayoutChild->addWidget(qPushButtonUp);
            qLayoutChild->addWidget(qPushButtonDown);
            qLayoutChild->addWidget(qPushButtonCopy);
            qLayoutChild->addWidget(qPushButtonPaste);
            qLayoutMain->addItem(qLayoutChild);
            break;

        case LEFT:
            optionsLayout = LEFT;
            qLayoutMain = new QHBoxLayout(this);
            qLayoutChild = new QVBoxLayout();
            qLayoutChild->addWidget(qComboBoxScriptList);
            qLayoutChild->addWidget(qPushButtonNew);
            qLayoutChild->addWidget(qPushButtonSave);
            qLayoutChild->addWidget(qPushButtonDelete);
            qLayoutChild->addWidget(qPushButtonExecute);
            qLayoutChild->addWidget(qPushButtonAbort);
            qLayoutChild->addWidget(qPushButtonAdd);
            qLayoutChild->addWidget(qPushButtonRemove);
            qLayoutChild->addWidget(qPushButtonUp);
            qLayoutChild->addWidget(qPushButtonDown);
            qLayoutChild->addWidget(qPushButtonCopy);
            qLayoutChild->addWidget(qPushButtonPaste);
            qLayoutMain->addItem(qLayoutChild);
            qLayoutMain->addWidget(qTableWidgetScript);
            break;

        case RIGHT:
            optionsLayout = RIGHT;
            qLayoutMain = new QHBoxLayout(this);
            qLayoutChild = new QVBoxLayout();
            qLayoutChild->addWidget(qComboBoxScriptList);
            qLayoutChild->addWidget(qPushButtonNew);
            qLayoutChild->addWidget(qPushButtonSave);
            qLayoutChild->addWidget(qPushButtonDelete);
            qLayoutChild->addWidget(qPushButtonExecute);
            qLayoutChild->addWidget(qPushButtonAbort);
            qLayoutChild->addWidget(qPushButtonAdd);
            qLayoutChild->addWidget(qPushButtonRemove);
            qLayoutChild->addWidget(qPushButtonUp);
            qLayoutChild->addWidget(qPushButtonDown);
            qLayoutChild->addWidget(qPushButtonCopy);
            qLayoutChild->addWidget(qPushButtonPaste);
            qLayoutMain->addWidget(qTableWidgetScript);            
            qLayoutMain->addItem(qLayoutChild);
    }

}



int QEScript::getOptionsLayout()
{

    return optionsLayout;

}




void QEScript::comboBoxScriptSelected(int)
{

    QDomElement rootElement;
    QDomElement scriptElement;
    QDomElement rowElement;
    QDomNode rootNode;
    QString currentName;


    currentName = qComboBoxScriptList->currentText();
    rootElement = document.documentElement();
    if (rootElement.tagName() == "epicsqt")
    {
        rootNode = rootElement.firstChild();
        while (rootNode.isNull() == false)
        {
            scriptElement = rootNode.toElement();
            if (scriptElement.tagName() == "script")
            {
                if (currentName.compare(scriptElement.attribute("name")) == 0)
                {
                    while (qTableWidgetScript->rowCount() > 0)
                    {
                        qTableWidgetScript->removeRow(0);
                    }
                    rootNode = scriptElement.firstChild();
                    while (rootNode.isNull() == false)
                    {
                        rowElement = rootNode.toElement();
                        if (rowElement.tagName() == "row")
                        {
                            insertRow(rowElement.attribute("enable").compare("1") == 0, rowElement.attribute("program"), rowElement.attribute("parameters"), rowElement.attribute("directory"), rowElement.attribute("timeout").toInt(), rowElement.attribute("stop").compare("1") == 0, rowElement.attribute("log").compare("1") == 0);
                        }
                        rootNode = rootNode.nextSibling();
                    }

                    break;
                }
            }
            rootNode = rootNode.nextSibling();
        }
    }
    refreshWidgets();

}




void QEScript::buttonNewClicked()
{

    if (QMessageBox::question(this, "Info", "Do you want to create a new script (reset table)?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    {
        while (qTableWidgetScript->rowCount() > 0)
        {
            qTableWidgetScript->removeRow(0);
        }
        qComboBoxScriptList->setCurrentIndex(-1);
    }

}




void QEScript::buttonSaveClicked()
{

    QDomElement rootElement;
    QDomElement scriptElement;
    QDomElement rowElement;
    QDomNode rootNode;
    QString currentName;
    QString name;
    bool flag;
    int i;


    currentName = qComboBoxScriptList->currentText();
    do
    {
        name = QInputDialog::getText(this, "Script name", "Name:", QLineEdit::Normal , currentName, &flag);
    }
    while(flag && name.isEmpty());

    if (flag)
    {
        rootElement = document.documentElement();
        if (rootElement.tagName() == "epicsqt")
        {
            rootNode = rootElement.firstChild();
            while (rootNode.isNull() == false)
            {
                scriptElement = rootNode.toElement();
                if (scriptElement.tagName() == "script")
                {
                    if (name.compare(scriptElement.attribute("name")) == 0)
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
            if (QMessageBox::question(this, "Info", "Do you want to overwrite existing script '" + name + "'?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
            {
                rootElement.removeChild(rootNode);
                flag = true;
            }
        }
        if (flag)
        {
            scriptElement = document.createElement("script");
            scriptElement.setAttribute("name", name);
            for(i = 0; i < qTableWidgetScript->rowCount(); i++)
            {
                rowElement = document.createElement("row");
                rowElement.setAttribute("enable", ((QCheckBox *) qTableWidgetScript->cellWidget(i, 1))->isChecked());
                rowElement.setAttribute("program", qTableWidgetScript->item(i, 2)->text().trimmed());
                rowElement.setAttribute("parameters", qTableWidgetScript->item(i, 3)->text().trimmed());
                rowElement.setAttribute("directory", qTableWidgetScript->item(i, 4)->text().trimmed());
                rowElement.setAttribute("timeout", ((QSpinBox *) qTableWidgetScript->cellWidget(i, 5))->value());
                rowElement.setAttribute("stop", ((QCheckBox *) qTableWidgetScript->cellWidget(i, 6))->isChecked());
                rowElement.setAttribute("log", ((QCheckBox *) qTableWidgetScript->cellWidget(i, 7))->isChecked());
                scriptElement.appendChild(rowElement);
            }
            rootElement.appendChild(scriptElement);
            if (saveScriptList())
            {
                i = qComboBoxScriptList->findText(name);
                qComboBoxScriptList->setCurrentIndex(i);
                QMessageBox::information(this, "Info", "The script '" + name + "' was successfully saved!");
            }
            else
            {
                // TODO: restore original document if there is an error
                QMessageBox::critical(this, "Error", "Unable to save script '" + name + "' in file '" + filename + "'!");
            }
        }
    }

}




void QEScript::buttonDeleteClicked()
{

    QDomElement rootElement;
    QDomElement scriptElement;
    QDomNode rootNode;
    QString currentName;


    currentName = qComboBoxScriptList->currentText();
    if (QMessageBox::question(this, "Info", "Do you want to delete script '" + currentName + "'?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    {
        rootElement = document.documentElement();
        if (rootElement.tagName() == "epicsqt")
        {
            rootNode = rootElement.firstChild();
            while (rootNode.isNull() == false)
            {
                scriptElement = rootNode.toElement();
                if (scriptElement.tagName() == "script")
                {
                    if (currentName.compare(scriptElement.attribute("name")) == 0)
                    {
                        rootElement.removeChild(rootNode);
                        break;
                    }
                }
                rootNode = rootNode.nextSibling();
            }
        }
        if (saveScriptList())
        {
            while (qTableWidgetScript->rowCount() > 0)
            {
                qTableWidgetScript->removeRow(0);
            }
            qComboBoxScriptList->setCurrentIndex(-1);
            QMessageBox::information(this, "Info", "The script '" + currentName + "' was successfully delete!");
        }
        else
        {
            // TODO: restore original document if there is an error
            QMessageBox::critical(this, "Error", "Unable to delete script '" + currentName + "' in file '" + filename + "'!");
        }
    }

}




void QEScript::buttonExecuteClicked()
{
    QProcess *qProcess;
    QStringList qStringList;
    QString program;
    QString parameters;
    QString workingDirectory;
    bool log;
    int exitCode;
    int timeOut;
    int i;
    int j;


    qProcess = new QProcess(this);

    isExecuting = true;
    refreshWidgets();
    for(i = 0; isExecuting == true && i < qTableWidgetScript->rowCount(); i++)
    {
        qTableWidgetScript->selectRow(i);
        if (((QCheckBox *) qTableWidgetScript->cellWidget(i, 1))->isChecked() == false)
        {
            sendMessage("Skipping execution of program #" + QString::number(i + 1) + " since it is disabled");
        }
        else
        {
            program = qTableWidgetScript->item(i, 2)->text().trimmed();
            log = ((QCheckBox *) qTableWidgetScript->cellWidget(i, 7))->isChecked();
            if (program.isEmpty())
            {
                if (log)
                {
                    sendMessage("Skipping execution of program #" + QString::number(i + 1) + " since it is undefined", message_types(MESSAGE_TYPE_WARNING));
                }
            }
            else
            {
                timeOut = ((QSpinBox *) qTableWidgetScript->cellWidget(i, 5))->value();
                if (timeOut == 0)
                {
                    if (log)
                    {
                        sendMessage("Executing program #" + QString::number(i + 1) + " until it finishes");
                    }
                    j = -1;
                }
                else
                {
                    if (log)
                    {
                        sendMessage("Executing program #" + QString::number(i + 1) + " for " + QString::number(timeOut) + " seconds");
                    }
                    j = timeOut * 50;
                }
                workingDirectory = qTableWidgetScript->item(i, 4)->text().trimmed();
                if (workingDirectory.isEmpty() == false)
                {
                    qProcess->setWorkingDirectory(workingDirectory);
                }
                parameters = qTableWidgetScript->item(i, 3)->text().trimmed();
                if (parameters.isEmpty())
                {
                    qProcess->start(program);
                }
                else
                {
                    qStringList.append(parameters);
                    qProcess->start(program, qStringList);
                }
                while(true)
                {
                    QCoreApplication::processEvents();
                    qProcess->waitForFinished(20);
                    if (isExecuting == true)
                    {
                        if (qProcess->state() == QProcess::NotRunning)
                        {
                            exitCode = qProcess->exitCode();
                            if (log)
                            {
                                if (exitCode == 0)
                                {
                                    sendMessage("Finished executing program #" + QString::number(i + 1) + " with exit code '" + QString::number(exitCode) + "'");
                                }
                                else
                                {
                                    if (((QCheckBox *) qTableWidgetScript->cellWidget(i, 6))->isChecked())
                                    {
                                        sendMessage("Stop execution since program #" + QString::number(i + 1) + " has finished with exit code '" + QString::number(exitCode) + "'", message_types(MESSAGE_TYPE_WARNING));
                                        isExecuting = false;
                                    }
                                }
                            }
                            break;
                        }
                        else
                        {
                            if (j == 0)
                            {
                                if (log)
                                {
                                    sendMessage("Aborting execution of program #" + QString::number(i + 1) + " since " + QString::number(timeOut) + " seconds have passed", message_types(MESSAGE_TYPE_WARNING));
                                }
                                qProcess->kill();
                                break;
                            }
                            else if (j > 0)
                            {
                                j--;
                            }
                        }
                    }
                    else
                    {
                        sendMessage("Abort execution of program(s)", message_types(MESSAGE_TYPE_WARNING));
                        qProcess->kill();
                        break;
                    }
                }
            }
        }
    }

    isExecuting = false;
    refreshWidgets();

}




void QEScript::buttonAbortClicked()
{

    isExecuting = false;

}




void QEScript::buttonAddClicked()
{

    insertRow(true, "", "", "", 0, false, true);
    refreshWidgets();

}



void QEScript::buttonRemoveClicked()
{

    QTableWidgetItem *qTableWidgetItem;
    int rowSelectedCount;
    int rowSelected;
    int i;

    rowSelectedCount = qTableWidgetScript->selectionModel()->selectedRows().count();
    rowSelected = qTableWidgetScript->selectedItems().at(0)->row();

    for(i = rowSelectedCount; i > 0; i--)
    {
        qTableWidgetScript->removeRow(qTableWidgetScript->selectedItems().at(i - 1)->row());
    }

    for(i = 0; i < qTableWidgetScript->rowCount(); i++)
    {
        qTableWidgetItem = new QTableWidgetItem(QString::number(i + 1));
        qTableWidgetScript->setItem(i, 0, qTableWidgetItem);
    }

    if (rowSelected < qTableWidgetScript->rowCount())
    {
        qTableWidgetScript->selectRow(rowSelected);
    }
    else
    {
        qTableWidgetScript->selectRow(qTableWidgetScript->rowCount() - 1);
    }

    refreshWidgets();

}



void QEScript::buttonUpClicked()
{

    QModelIndexList qModelIndexList;
    _CopyPaste *copyPaste;
    int row;

    qModelIndexList = qTableWidgetScript->selectionModel()->selectedRows();
    row = qModelIndexList.at(0).row();

    copyPaste = new _CopyPaste();
    copyPaste->setEnable(((QCheckBox *) qTableWidgetScript->cellWidget(row - 1, 1))->isChecked());
    copyPaste->setProgram(qTableWidgetScript->item(row - 1, 2)->text());
    copyPaste->setParameters(qTableWidgetScript->item(row - 1, 3)->text());
    copyPaste->setWorkingDirectory(qTableWidgetScript->item(row - 1, 4)->text());
    copyPaste->setTimeOut(((QSpinBox *) qTableWidgetScript->cellWidget(row - 1, 5))->value());
    copyPaste->setStop(((QCheckBox *) qTableWidgetScript->cellWidget(row - 1, 6))->isChecked());
    copyPaste->setLog(((QCheckBox *) qTableWidgetScript->cellWidget(row - 1, 7))->isChecked());

    ((QCheckBox *) qTableWidgetScript->cellWidget(row - 1, 1))->setChecked(((QCheckBox *) qTableWidgetScript->cellWidget(row, 1))->isChecked());
    qTableWidgetScript->item(row - 1, 2)->setText(qTableWidgetScript->item(row, 2)->text());
    qTableWidgetScript->item(row - 1, 3)->setText(qTableWidgetScript->item(row, 3)->text());
    qTableWidgetScript->item(row - 1, 4)->setText(qTableWidgetScript->item(row, 4)->text());
    ((QSpinBox *) qTableWidgetScript->cellWidget(row - 1, 5))->setValue(((QSpinBox *) qTableWidgetScript->cellWidget(row, 5))->value());
    ((QCheckBox *) qTableWidgetScript->cellWidget(row - 1, 6))->setChecked(((QCheckBox *) qTableWidgetScript->cellWidget(row, 6))->isChecked());
    ((QCheckBox *) qTableWidgetScript->cellWidget(row - 1, 7))->setChecked(((QCheckBox *) qTableWidgetScript->cellWidget(row, 7))->isChecked());

    ((QCheckBox *) qTableWidgetScript->cellWidget(row, 1))->setChecked(copyPaste->getEnable());
    qTableWidgetScript->item(row, 2)->setText(copyPaste->getProgram());
    qTableWidgetScript->item(row, 3)->setText(copyPaste->getParameters());
    qTableWidgetScript->item(row, 4)->setText(copyPaste->getWorkingDirectory());
    ((QSpinBox *) qTableWidgetScript->cellWidget(row, 5))->setValue(copyPaste->getTimeOut());
    ((QCheckBox *) qTableWidgetScript->cellWidget(row, 6))->setChecked(copyPaste->getStop());
    ((QCheckBox *) qTableWidgetScript->cellWidget(row, 7))->setChecked(copyPaste->getLog());

    qTableWidgetScript->selectRow(row - 1);

    refreshWidgets();

}



void QEScript::buttonDownClicked()
{

    QModelIndexList qModelIndexList;
    _CopyPaste *copyPaste;
    int row;

    qModelIndexList = qTableWidgetScript->selectionModel()->selectedRows();
    row = qModelIndexList.at(0).row();

    copyPaste = new _CopyPaste();
    copyPaste->setEnable(((QCheckBox *) qTableWidgetScript->cellWidget(row + 1, 1))->isChecked());
    copyPaste->setProgram(qTableWidgetScript->item(row + 1, 2)->text());
    copyPaste->setParameters(qTableWidgetScript->item(row + 1, 3)->text());
    copyPaste->setWorkingDirectory(qTableWidgetScript->item(row + 1, 4)->text());
    copyPaste->setTimeOut(((QSpinBox *) qTableWidgetScript->cellWidget(row + 1, 5))->value());
    copyPaste->setStop(((QCheckBox *) qTableWidgetScript->cellWidget(row + 1, 6))->isChecked());
    copyPaste->setLog(((QCheckBox *) qTableWidgetScript->cellWidget(row + 1, 7))->isChecked());

    ((QCheckBox *) qTableWidgetScript->cellWidget(row + 1, 1))->setChecked(((QCheckBox *) qTableWidgetScript->cellWidget(row, 1))->isChecked());
    qTableWidgetScript->item(row + 1, 2)->setText(qTableWidgetScript->item(row, 2)->text());
    qTableWidgetScript->item(row + 1, 3)->setText(qTableWidgetScript->item(row, 3)->text());
    qTableWidgetScript->item(row + 1, 4)->setText(qTableWidgetScript->item(row, 4)->text());
    ((QSpinBox *) qTableWidgetScript->cellWidget(row + 1, 5))->setValue(((QSpinBox *) qTableWidgetScript->cellWidget(row, 5))->value());
    ((QCheckBox *) qTableWidgetScript->cellWidget(row + 1, 6))->setChecked(((QCheckBox *) qTableWidgetScript->cellWidget(row, 6))->isChecked());
    ((QCheckBox *) qTableWidgetScript->cellWidget(row + 1, 7))->setChecked(((QCheckBox *) qTableWidgetScript->cellWidget(row, 7))->isChecked());

    ((QCheckBox *) qTableWidgetScript->cellWidget(row, 1))->setChecked(copyPaste->getEnable());
    qTableWidgetScript->item(row, 2)->setText(copyPaste->getProgram());
    qTableWidgetScript->item(row, 3)->setText(copyPaste->getParameters());
    qTableWidgetScript->item(row, 4)->setText(copyPaste->getWorkingDirectory());
    ((QSpinBox *) qTableWidgetScript->cellWidget(row, 5))->setValue(copyPaste->getTimeOut());
    ((QCheckBox *) qTableWidgetScript->cellWidget(row, 6))->setChecked(copyPaste->getStop());
    ((QCheckBox *) qTableWidgetScript->cellWidget(row, 7))->setChecked(copyPaste->getLog());

    qTableWidgetScript->selectRow(row + 1);

    refreshWidgets();

}



void QEScript::buttonCopyClicked()
{

    QModelIndexList qModelIndexList;
    _CopyPaste *copyPaste;
    int row;
    int i;

    while (copyPasteList.isEmpty() == false)
    {
        delete copyPasteList.at(0);
        copyPasteList.removeAt(0);
    }

    qModelIndexList = qTableWidgetScript->selectionModel()->selectedRows();
    for(i = 0; i < qModelIndexList.count(); i++)
    {
        row = qModelIndexList.at(i).row();
        copyPaste = new _CopyPaste();
        copyPaste->setEnable(((QCheckBox *) qTableWidgetScript->cellWidget(row, 1))->isChecked());
        copyPaste->setProgram(qTableWidgetScript->item(row, 2)->text());
        copyPaste->setParameters(qTableWidgetScript->item(row, 3)->text());
        copyPaste->setWorkingDirectory(qTableWidgetScript->item(row, 4)->text());
        copyPaste->setTimeOut(((QSpinBox *) qTableWidgetScript->cellWidget(row, 5))->value());
        copyPaste->setStop(((QCheckBox *) qTableWidgetScript->cellWidget(row, 6))->isChecked());
        copyPaste->setLog(((QCheckBox *) qTableWidgetScript->cellWidget(row, 7))->isChecked());
        copyPasteList.append(copyPaste);
    }

    refreshWidgets();

}



void QEScript::buttonPasteClicked()
{

    QCheckBox *qCheckBox;
    QSpinBox *qSpinBox;
    QModelIndexList qModelIndexList;
    QTableWidgetItem *qTableWidgetItem;
    int row;
    int i;


    qModelIndexList = qTableWidgetScript->selectionModel()->selectedRows();
    if (qModelIndexList.isEmpty())
    {
        row = 0;
    }
    else
    {
        row = qModelIndexList.at(0).row();
    }

    for(i = 0; i < copyPasteList.count(); i++)
    {
        qTableWidgetScript->insertRow(row + i);

        qCheckBox = new QCheckBox();
        qCheckBox->setChecked(copyPasteList.at(i)->getEnable());
        qTableWidgetScript->setCellWidget(row + i, 1, qCheckBox);

        qTableWidgetItem = new QTableWidgetItem(copyPasteList.at(i)->getProgram());
        qTableWidgetScript->setItem(row + i, 2, qTableWidgetItem);

        qTableWidgetItem = new QTableWidgetItem(copyPasteList.at(i)->getParameters());
        qTableWidgetScript->setItem(row + i, 3, qTableWidgetItem);

        qTableWidgetItem = new QTableWidgetItem(copyPasteList.at(i)->getWorkingDirectory());
        qTableWidgetScript->setItem(row + i, 4, qTableWidgetItem);

        qSpinBox = new QSpinBox();
        qSpinBox->setSuffix(" s");
        qSpinBox->setValue(copyPasteList.at(i)->getTimeOut());
        qTableWidgetScript->setCellWidget(row + i, 5, qSpinBox);

        qCheckBox = new QCheckBox();
        qCheckBox->setChecked(copyPasteList.at(i)->getStop());
        qTableWidgetScript->setCellWidget(row + i, 6, qCheckBox);

        qCheckBox = new QCheckBox();
        qCheckBox->setChecked(copyPasteList.at(i)->getLog());
        qTableWidgetScript->setCellWidget(row + i, 7, qCheckBox);
    }

    for(i = 0; i < qTableWidgetScript->rowCount(); i++)
    {
        qTableWidgetItem = new QTableWidgetItem(QString::number(i + 1));
        qTableWidgetItem->setFlags(qTableWidgetItem->flags() ^ Qt::ItemIsEditable);
        qTableWidgetScript->setItem(i, 0, qTableWidgetItem);
    }

    refreshWidgets();

}



void QEScript::selectionChanged(const QItemSelection &, const QItemSelection &)
{

    refreshWidgets();

}




void QEScript::insertRow(bool pEnable, QString pProgram, QString pParameters, QString pWorkingDirectory, int pTimeOut, bool pStop, bool pLog)
{

    QTableWidgetItem *qTableWidgetItem;
    QCheckBox *qCheckBox;
    QSpinBox *qSpinBox;
    int row;
    int i;


    if (qTableWidgetScript->selectionModel()->selectedRows().count() == 0)
    {
        row = qTableWidgetScript->rowCount();
        qTableWidgetScript->insertRow(row);
    }
    else
    {
        row = qTableWidgetScript->selectedItems().at(0)->row();
        qTableWidgetScript->insertRow(row);
        qTableWidgetScript->selectRow(row);
    }

    qCheckBox = new QCheckBox();
    qCheckBox->setChecked(pEnable);
    qCheckBox->setEnabled(editableTable);
    qTableWidgetScript->setCellWidget(row, 1, qCheckBox);

    qTableWidgetItem = new QTableWidgetItem(pProgram);
    if (editableTable == true)
    {
        qTableWidgetItem->setFlags(qTableWidgetItem->flags() | Qt::ItemIsEditable);
    }
    else
    {
        qTableWidgetItem->setFlags(qTableWidgetItem->flags() ^ Qt::ItemIsEditable);
    }
    qTableWidgetScript->setItem(row, 2, qTableWidgetItem);

    qTableWidgetItem = new QTableWidgetItem(pParameters);
    if (editableTable == true)
    {
        qTableWidgetItem->setFlags(qTableWidgetItem->flags() | Qt::ItemIsEditable);
    }
    else
    {
        qTableWidgetItem->setFlags(qTableWidgetItem->flags() ^ Qt::ItemIsEditable);
    }
    qTableWidgetScript->setItem(row, 3, qTableWidgetItem);

    qTableWidgetItem = new QTableWidgetItem(pWorkingDirectory);
    if (editableTable == true)
    {
        qTableWidgetItem->setFlags(qTableWidgetItem->flags() | Qt::ItemIsEditable);
    }
    else
    {
        qTableWidgetItem->setFlags(qTableWidgetItem->flags() ^ Qt::ItemIsEditable);
    }
    qTableWidgetScript->setItem(row, 4, qTableWidgetItem);

    qSpinBox = new QSpinBox();
    qSpinBox->setValue(pTimeOut);
    qSpinBox->setSuffix(" s");
    qSpinBox->setEnabled(editableTable);
    qTableWidgetScript->setCellWidget(row, 5, qSpinBox);

    qCheckBox = new QCheckBox();
    qCheckBox->setChecked(pStop);
    qCheckBox->setEnabled(editableTable);
    qTableWidgetScript->setCellWidget(row, 6, qCheckBox);

    qCheckBox = new QCheckBox();
    qCheckBox->setChecked(pLog);
    qCheckBox->setEnabled(editableTable);
    qTableWidgetScript->setCellWidget(row, 7, qCheckBox);

    for(i = row; i < qTableWidgetScript->rowCount(); i++)
    {
        qTableWidgetItem = new QTableWidgetItem(QString::number(i + 1));
        qTableWidgetItem->setFlags(qTableWidgetItem->flags() ^ Qt::ItemIsEditable);
        qTableWidgetScript->setItem(i, 0, qTableWidgetItem);
    }

}



bool QEScript::saveScriptList()
{

    QFile *file;

    file = new QFile(filename);
    if (file->open(QFile::WriteOnly | QFile::Text))
    {
        QTextStream stream(file);
        document.save(stream, 3);
        file->close();
        refreshScriptList();
        return true;
    }
    else
    {
        return false;
    }

}



void QEScript::refreshScriptList()
{

    QDomElement rootElement;
    QDomElement scriptElement;
    QDomNode rootNode;
    QString tmp;
    int i;


    qComboBoxScriptList->blockSignals(true);
    tmp = qComboBoxScriptList->currentText();
    qComboBoxScriptList->clear();
    rootElement = document.documentElement();
    if (rootElement.tagName() == "epicsqt")
    {
        rootNode = rootElement.firstChild();
        while (rootNode.isNull() == false)
        {
            scriptElement = rootNode.toElement();
            if (scriptElement.tagName() == "script")
            {
                qComboBoxScriptList->addItem(scriptElement.attribute("name"));
            }
            rootNode = rootNode.nextSibling();
        }
    }
    i = qComboBoxScriptList->findText(tmp);
    if (i == -1)
    {
        qComboBoxScriptList->setCurrentIndex(0);
    }
    else
    {
       qComboBoxScriptList->setCurrentIndex(i);
    }
    refreshWidgets();
    qComboBoxScriptList->blockSignals(false);

}




void QEScript::refreshWidgets()
{

    int rowCount;
    int rowSelectedCount;

    rowCount = qTableWidgetScript->rowCount();
    rowSelectedCount = qTableWidgetScript->selectionModel()->selectedRows().count();

    qComboBoxScriptList->setEnabled(isExecuting == false);
    qPushButtonNew->setEnabled(scriptType == FROM_FILE && isExecuting == false);
    qPushButtonSave->setEnabled(scriptType == FROM_FILE && isExecuting == false && qTableWidgetScript->rowCount() > 0);
    qPushButtonDelete->setEnabled(scriptType == FROM_FILE && isExecuting == false && qComboBoxScriptList->currentText().isEmpty() == false);
    qPushButtonExecute->setEnabled(isExecuting == false && rowCount > 0);
    qPushButtonAbort->setEnabled(isExecuting == true);

    qPushButtonAdd->setEnabled(isExecuting == false && rowSelectedCount <= 1);
    qPushButtonRemove->setEnabled(isExecuting == false && rowSelectedCount > 0);
    qPushButtonUp->setEnabled(isExecuting == false && rowSelectedCount == 1 && qTableWidgetScript->selectionModel()->selectedRows().at(0).row() > 0);
    qPushButtonDown->setEnabled(isExecuting == false && rowSelectedCount == 1 && qTableWidgetScript->selectionModel()->selectedRows().at(0).row() < rowCount - 1);
    qPushButtonCopy->setEnabled(isExecuting == false && rowSelectedCount > 0);
    qPushButtonPaste->setEnabled(isExecuting == false && copyPasteList.isEmpty() == false);

    qTableWidgetScript->setEnabled(isExecuting == false);

}




// ============================================================
//  _QCOPYPASTE CLASS
// ============================================================
_CopyPaste::_CopyPaste()
{

    setEnable(false);

    setProgram("");

    setParameters("");

    setWorkingDirectory("");

    setTimeOut(0);

    setStop(false);

    setLog(false);

};



_CopyPaste::_CopyPaste(bool pEnable, QString pProgram, QString pParameters, QString pWorkingDirectory, int pTimeOut, bool pStop, bool pLog)
{

    setEnable(pEnable);

    setProgram(pProgram);

    setParameters(pParameters);

    setWorkingDirectory(pWorkingDirectory);

    setTimeOut(pTimeOut);

    setStop(pStop);

    setLog(pLog);

};




void _CopyPaste::setEnable(bool pEnable)
{

    enable = pEnable;

}



bool _CopyPaste::getEnable()
{

    return enable;

}



void _CopyPaste::setProgram(QString pProgram)
{

    program = pProgram;

}



QString _CopyPaste::getProgram()
{

    return program;

}




void _CopyPaste::setParameters(QString pParameters)
{

    parameters = pParameters;

}



QString _CopyPaste::getParameters()
{

    return parameters;

}



void _CopyPaste::setWorkingDirectory(QString pWorkingDirectory)
{

    workingDirectory = pWorkingDirectory;

}



QString _CopyPaste::getWorkingDirectory()
{

    return workingDirectory;

}



void _CopyPaste::setTimeOut(int pTimeOut)
{

    timeOut = pTimeOut;

}



int _CopyPaste::getTimeOut()
{

    return timeOut;

}




void _CopyPaste::setStop(bool pStop)
{

    stop = pStop;

}



bool _CopyPaste::getStop()
{

    return stop;

}




void _CopyPaste::setLog(bool pLog)
{

    log = pLog;

}



bool _CopyPaste::getLog()
{

    return log;

}






// ============================================================
//  _QTABLEWIDGETSCRIPT METHODS
// ============================================================
_QTableWidgetScript::_QTableWidgetScript(QWidget *pParent):QTableWidget(pParent)
{

    initialized = false;

}



void _QTableWidgetScript::refreshSize()
{

    int i;
    int hidden;


    hidden = 0;
    for(i = 0; i < this->columnCount(); i++)
    {
        if (this->isColumnHidden(i))
        {
            hidden++;
        }
    }


    for(i = 0; i < this->columnCount(); i++)
    {
        this->setColumnWidth(i, this->width() / (this->columnCount() - hidden));
    }

}



void _QTableWidgetScript::resizeEvent(QResizeEvent *)
{

    // TODO: this condition should always be execute when inside Qt Designer
    if (initialized == false)
    {
        refreshSize();
        initialized = true;
    }

}


