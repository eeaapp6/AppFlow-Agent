#include "OperatorsSphereManager.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/GraphEventOperator.h"
#include "OperatorsInterface/TreeEventOperator.h"
#include "GUIDialog/GUIGeometryDialog/SphereInfoWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoCommand.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoModelSphere.h"

namespace ModelOper
{
    OperatorsSphereManager::OperatorsSphereManager()
    {

    }

    OperatorsSphereManager::~OperatorsSphereManager()
    {

    }

    bool OperatorsSphereManager::execGUI()
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

        switch (_operType) {
        case ModelOper::OperManagerBase::Create:
            widget = new GUI::SphereInfoWidget(this);
            break;
        case ModelOper::OperManagerBase::Edit: {
            Interface::FITKAbsGeoModelSphere* obj = dynamic_cast<Interface::FITKAbsGeoModelSphere*>(geometryData->getDataByID(objID));
            widget = new GUI::SphereInfoWidget(obj, this);
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

    bool OperatorsSphereManager::execProfession()
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
