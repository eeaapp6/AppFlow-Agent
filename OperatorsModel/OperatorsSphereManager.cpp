#include "OperatorsSphereManager.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "GUIWidget/GUIPickInfo.h"
#include "GUIWidget/PickedDataProvider.h"
#include "GUIWidget/PickedData.h"
#include "GUIWidget/WidgetOCCEvent.h"
#include "OperatorsInterface/GraphEventOperator.h"
#include "OperatorsInterface/TreeEventOperator.h"
#include "GUIDialog/GUIGeometryDialog/SphereInfoWidget.h"
#include "GUIDialog/GUIGeometryDialog/GeometryDeleteDialog.h"

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
            graphOper->updateGraph(objID);
            treeOper->updateTree();
            break;
        case ModelOper::OperManagerBase::Rename:
            break;
        }

        propertyWidget->init();

        return true;
    }

    void OperatorsSphereManager::moveToStep(int index, QVariant value)
    {
        //几何基点重选择事件
        if (index == 0) {
            //拾取信息设置
            GUI::GUIPickInfoStru pinfo;
            pinfo._pickObjType = GUI::GUIPickInfo::PickObjType::POBJVert;
            pinfo._pickMethod = GUI::GUIPickInfo::PickMethod::PMSingle;
            //保存参数
            GUI::GUIPickInfo::SetPickInfo(pinfo);

            //拾取对象获取事件绑定
            GraphData::PickedDataProvider* pickD = GraphData::PickedDataProvider::getInstance();
            if (pickD == nullptr) return;
            connect(pickD, SIGNAL(sig_dataPicked()), this, SLOT(slotReselectCurrentPoint()));
        }
    }

    void OperatorsSphereManager::slotReselectCurrentPoint()
    {
        GraphData::PickedDataProvider* pickD = GraphData::PickedDataProvider::getInstance();
        if (pickD == nullptr) return;
        disconnect(pickD, SIGNAL(sig_dataPicked()), this, SLOT(slotReselectCurrentPoint()));

        //拾取信息设置
        GUI::GUIPickInfoStru pinfo;
        pinfo._pickObjType = GUI::GUIPickInfo::PickObjType::POBJNone;
        pinfo._pickMethod = GUI::GUIPickInfo::PickMethod::PMNone;
        //保存参数
        GUI::GUIPickInfo::SetPickInfo(pinfo);

        QList<GraphData::PickedData*> pickData = pickD->getPickedList();
        if (pickData.size() == 0)return;
        if (!pickData[0])return;

        double point[3] = { 0,0,0 };
        GUI::WidgetOCCEvent::getPoint(pickData[0], point);

        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return;

        GUI::SphereInfoWidget* cudeWidget = dynamic_cast<GUI::SphereInfoWidget*>(propertyWidget->getCurrentWidget());
        if (cudeWidget == nullptr)return;
        cudeWidget->setCenterPoint(point);

        pickD->clearPickedData();
    }
}
