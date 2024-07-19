#include "OperatorsCylinderManager.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/GraphEventOperator.h"
#include "OperatorsInterface/TreeEventOperator.h"
#include "GUIDialog/GUIGeometryDialog/CylinderInfoWidget.h"
#include "GUIDialog/GUIGeometryDialog/GeometryDeleteDialog.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoCommand.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoModelCylinder.h"

namespace ModelOper
{
    OperatorsCylinderManager::OperatorsCylinderManager()
    {

    }

    OperatorsCylinderManager::~OperatorsCylinderManager()
    {

    }

    bool OperatorsCylinderManager::execGUI()
    {
        QWidget* widget = nullptr;
        QDialog* dialog = nullptr;

        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;
        Interface::FITKOFGeometryData* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKOFGeometryData>();
        if (geometryData == nullptr) return false;

        int objID = -1;
        this->argValue("objID", objID);

        switch (_operType) {
        case ModelOper::OperManagerBase::Create:
            widget = new GUI::CylinderInfoWidget(this);
            break;
        case ModelOper::OperManagerBase::Edit: {
            Interface::FITKAbsGeoModelCylinder* obj = dynamic_cast<Interface::FITKAbsGeoModelCylinder*>(geometryData->getDataByID(objID));
            widget = new GUI::CylinderInfoWidget(obj, this);
            break;
        }
        case ModelOper::OperManagerBase::Copy:
            break;
        case ModelOper::OperManagerBase::Delete:
            dialog = new GUI::GeometryDeleteDialog(dynamic_cast<Interface::FITKAbsGeoCommand*>(geometryData->getDataByID(objID)), this);
            break;
        case ModelOper::OperManagerBase::Rename:
            break;
        }

        if (mainWindow->getPropertyWidget() && widget) {
            propertyWidget->setWidget(widget);
        }

        if (dialog) {
            dialog->show();
        }

        return false;
    }

    bool OperatorsCylinderManager::execProfession()
    {
        // 获取模型树控制器
        auto treeOper = Core::FITKOperatorRepo::getInstance()->getOperatorT<EventOper::TreeEventOperator>("ModelTreeEvent");
        if (treeOper == nullptr) return false;
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return false;
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        int objID = -1;
        this->argValue("objID", objID);

        switch (_operType) {
        case ModelOper::OperManagerBase::Create:
            graphOper->updateGraph(objID);
            treeOper->updateTree();
            break;
        case ModelOper::OperManagerBase::Edit:
            graphOper->updateGraph(objID);
            treeOper->updateTree();
            break;
        case ModelOper::OperManagerBase::Copy:
            break;
        case ModelOper::OperManagerBase::Delete:
            graphOper->updateGraph(objID);
            treeOper->updateTree();
            break;
        case ModelOper::OperManagerBase::Rename:
            break;
        }

        propertyWidget->init();

        return true;
    }
}
