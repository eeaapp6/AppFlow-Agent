#include "OperatorsLocalManager.h"

#include "GUIFrame/PropertyWidget.h"
#include "GUIDialog/GUIMeshDialog/LocalSelectGroupWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

#include <QDialog>

namespace ModelOper 
{
    OperatorsLocalManager::OperatorsLocalManager()
    {

    }

    OperatorsLocalManager::~OperatorsLocalManager()
    {

    }

    bool OperatorsLocalManager::execGUI()
    {
        QWidget* widget = nullptr;
        QDialog* dialog = nullptr;

        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        if (_emitter == nullptr)return false;
        QString sendName = _emitter->objectName();

        if (sendName == "actionLocalSelectGroup") {
            widget = new GUI::LocalSelectGroupWidget(this);
        }
        
        if (mainWindow->getPropertyWidget() && widget) {
            propertyWidget->setWidget(widget);
        }

        if (dialog) {
            dialog->show();
        }

        return false;
    }

    bool OperatorsLocalManager::execProfession()
    {
        return false;
    }

    void OperatorsLocalManager::moveToStep(int index, QVariant value)
    {

    }
}