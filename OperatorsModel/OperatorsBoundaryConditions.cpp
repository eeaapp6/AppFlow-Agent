#include "OperatorsBoundaryConditions.h"

#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/TreeEventOperator.h"
#include "GUIDialog/GUICalculateDialog/BoundaryConditionsWidget.h"
#include "GUIDialog/GUICalculateDialog/BoundaryConditionsCreateDialog.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFBoundary.h"

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
        auto physicsData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (physicsData == nullptr)return false;
        auto boundaryManager = physicsData->getBoundaryManager();
        if (boundaryManager == nullptr)return false;

        switch (_operType){
        case ModelOper::OperManagerBase::Create: {
            GUI::BoundaryConditionsCreateDialog* dialog = new GUI::BoundaryConditionsCreateDialog(this);
            dialog->show();
            break;
        }
        case ModelOper::OperManagerBase::Edit: {
            int objID = -1;
            this->argValue<int>("objID", objID);
            Interface::FITKOFBoundary* boundaryObj = boundaryManager->getDataByID(objID);
            GUI::BoundaryConditionsWidget* widget = new GUI::BoundaryConditionsWidget(boundaryObj, this, propertyWidget);
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

