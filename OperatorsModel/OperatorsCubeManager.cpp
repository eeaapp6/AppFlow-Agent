#include "OperatorsCubeManager.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/GraphEventOperator.h"
#include "OperatorsInterface/TreeEventOperator.h"
#include "GUIDialog/GUIGeometryDialog/CudeInfoWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoCommand.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoModelBox.h"

namespace ModelOper
{
    OperatorsCubeManager::OperatorsCubeManager()
    {

    }

    OperatorsCubeManager::~OperatorsCubeManager()
    {

    }

    bool OperatorsCubeManager::execGUI()
    {
        QWidget* widget = nullptr;

        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;
        Interface::FITKOFGeometryData* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKOFGeometryData>();
        if (geometryData == nullptr) return false;

        int objID = -1;
        this->argValue("objID", objID);

        switch (_operType){
        case ModelOper::OperManagerBase::Create:
            widget = new GUI::CudeInfoWidget(this);
            break;
        case ModelOper::OperManagerBase::Edit: {
            Interface::FITKAbsGeoModelBox* obj = dynamic_cast<Interface::FITKAbsGeoModelBox*>(geometryData->getDataByID(objID));
            widget = new GUI::CudeInfoWidget(obj, this);
            break;
        }
        case ModelOper::OperManagerBase::Copy:
            break;
        case ModelOper::OperManagerBase::Delete:
            break;
        case ModelOper::OperManagerBase::Rename:
            break;
        }

        if (mainWindow->getPropertyWidget()) {
            propertyWidget->setWidget(widget);
        }

        return false;
    }

    bool OperatorsCubeManager::execProfession()
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
            break;
        case ModelOper::OperManagerBase::Rename:
            break;
        }

        propertyWidget->init();

        return true;
    }
}
