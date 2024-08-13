#include "OperatorsSetup.h"

#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/TreeEventOperator.h"
#include "GUIDialog/GUICalculateDialog/SetupWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace ModelOper 
{
    OperatorsSetup::OperatorsSetup()
    {

    }

    OperatorsSetup::~OperatorsSetup()
    {

    }

    bool OperatorsSetup::execGUI()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        GUI::SetupWidget* widget = new GUI::SetupWidget(this, FITKAPP->getGlobalData()->getMainWindow());
        propertyWidget->setWidget(widget);

        return true;
    }

    bool OperatorsSetup::execProfession()
    {
        // 获取模型树控制器
        auto treeOper = Core::FITKOperatorRepo::getInstance()->getOperatorT<EventOper::TreeEventOperator>("ModelTreeEvent");
        if (treeOper == nullptr) return false;
        treeOper->updateTree();

        return true;
    }
}

