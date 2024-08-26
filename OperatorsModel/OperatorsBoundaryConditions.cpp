#include "OperatorsBoundaryConditions.h"

#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/TreeEventOperator.h"
#include "GUIDialog/GUICalculateDialog/BoundaryConditionsWidget.h"
#include "GUIDialog/GUICalculateDialog/BoundaryConditionsCreateDialog.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace ModelOper
{
    OperatorsBoundaryConditions::OperatorsBoundaryConditions()
    {

    }

    OperatorsBoundaryConditions::~OperatorsBoundaryConditions()
    {

    }

    bool OperatorsBoundaryConditions::execGUI()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        switch (_operType){
        case ModelOper::OperManagerBase::Create: {
            GUI::BoundaryConditionsCreateDialog* dialog = new GUI::BoundaryConditionsCreateDialog(this);
            dialog->show();
            break;
        }
        case ModelOper::OperManagerBase::Edit: {
            GUI::BoundaryConditionsWidget* widget = new GUI::BoundaryConditionsWidget(this, propertyWidget);
            propertyWidget->setWidget(widget);
            break;
        }
        }

        return false;
    }

    bool OperatorsBoundaryConditions::execProfession()
    {
        switch (_operType){
        case ModelOper::OperManagerBase::Create:{
            // 获取模型树控制器
            auto treeOper = Core::FITKOperatorRepo::getInstance()->getOperatorT<EventOper::TreeEventOperator>("ModelTreeEvent");
            if (treeOper == nullptr) return false;
            treeOper->updateTree();
            break;
        }
        case ModelOper::OperManagerBase::Edit: {
            break;
        }
        }

        return true;
    }
}

