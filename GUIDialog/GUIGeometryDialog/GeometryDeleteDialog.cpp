#include "GeometryDeleteDialog.h"
#include "ui_GeometryDeleteDialog.h"
#include "CudeInfoWidget.h"
#include "CylinderInfoWidget.h"
#include "SphereInfoWidget.h"
#include "BoolInfoWidget.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"

#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoCommand.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKGeometryMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeGeom.h"

namespace GUI
{
    GeometryDeleteDialog::GeometryDeleteDialog(Interface::FITKAbsGeoCommand * obj, EventOper::ParaWidgetInterfaceOperator * oper) :
        GUIDialogBase(FITKAPP->getGlobalData()->getMainWindow()), _oper(oper), _obj(obj)
    {
        _ui = new Ui::GeometryDeleteDialog();
        _ui->setupUi(this);

        if (_obj) {
            QString text = tr("Delete %1").arg(_obj->getDataObjectName());
            _ui->label_Name->setText(text);
        }

        setWindowTitle(tr("Geometry Delete"));
    }

    GeometryDeleteDialog::~GeometryDeleteDialog()
    {
        if (_ui)delete _ui;
    }

    void GeometryDeleteDialog::on_pushButton_OK_clicked()
    {
        if (_obj == nullptr)return;
        Interface::FITKGeoCommandList* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
        if (geometryData == nullptr) return;
        auto meshSizeManager = Interface::FITKMeshGenInterface::getInstance()->getGeometryMeshSizeManager();
        if (meshSizeManager == nullptr)return;

        //判断删除的数据是否是当前界面
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return;

        auto type = _obj->getGeometryCommandType();
        QWidget* widget = nullptr;
        int objID = -1;
        switch (type){
        case Interface::FITKGeoEnum::FGTBox: {
            GUI::CudeInfoWidget* cudeWidget = dynamic_cast<GUI::CudeInfoWidget*>(propertyWidget->getCurrentWidget());
            if (cudeWidget == nullptr)break;
            if (cudeWidget->getCurrentGeoCommand()) {
                objID = cudeWidget->getCurrentGeoCommand()->getDataObjectID();
                widget = cudeWidget;
            }
            break;
        }
        case Interface::FITKGeoEnum::FGTCylinder: {
            GUI::CylinderInfoWidget* cudeWidget = dynamic_cast<GUI::CylinderInfoWidget*>(propertyWidget->getCurrentWidget());
            if (cudeWidget == nullptr)break;
            if (cudeWidget->getCurrentGeoCommand()) {
                objID = cudeWidget->getCurrentGeoCommand()->getDataObjectID();
                widget = cudeWidget;
            }
            break;
        }
        case Interface::FITKGeoEnum::FGTSphere: {
            GUI::SphereInfoWidget* cudeWidget = dynamic_cast<GUI::SphereInfoWidget*>(propertyWidget->getCurrentWidget());
            if (cudeWidget == nullptr)break;
            if (cudeWidget->getCurrentGeoCommand()) {
                objID = cudeWidget->getCurrentGeoCommand()->getDataObjectID();
                widget = cudeWidget;
            }
            break;
        }
        case Interface::FITKGeoEnum::FGTBool:
        case Interface::FITKGeoEnum::FGTImport:{
            GUI::BoolInfoWidget* cudeWidget = dynamic_cast<GUI::BoolInfoWidget*>(propertyWidget->getCurrentWidget());
            if (cudeWidget == nullptr)break;
            if (cudeWidget->getCurrentGeoCommand()) {
                objID = cudeWidget->getCurrentGeoCommand()->getDataObjectID();
                widget = cudeWidget;
            }
            break;
        }
        }

        //如果删除的数据是当前界面,删除当前界面
        if (objID == _obj->getDataObjectID() && widget) {
            propertyWidget->init();
        }

        //清除与当前几何相关的网格边界参数类
        QList<int> meshSizeIds = {};
        for (int i = 0; i < meshSizeManager->getDataCount(); i++) {
            auto meshSize = meshSizeManager->getDataByIndex(i);
            if(meshSize == nullptr)continue;
            if (meshSize->getGeoModel() == _obj) {
                meshSizeIds.append(meshSize->getDataObjectID());
            }
        }
        for (int id : meshSizeIds) {
            meshSizeManager->removeDataByID(id);
        }

        
        //清除当前几何关联的网格区域尺寸
        auto RegionMeshSizeManager = Interface::FITKMeshGenInterface::getInstance()->getRegionMeshSizeMgr();
        for (auto RegionMeshSize : RegionMeshSizeManager->getRigonByType(Interface::FITKAbstractRegionMeshSize::RegionType::RigonGeom))
        {
            auto RegionGeoMeshSize = dynamic_cast<Interface::FITKRegionMeshSizeGeom*>(RegionMeshSize);
            if (RegionGeoMeshSize == nullptr)continue;
            if (RegionGeoMeshSize->getGeomID() == _obj->getDataObjectID()) {
                RegionMeshSizeManager->removeDataObj(RegionGeoMeshSize);
                break;
            }
        }

        //清除几何对象
        geometryData->removeDataByID(_obj->getDataObjectID());
        _oper->execProfession();

        //刷新几何关联的网格区域尺寸界面
        EventOper::ParaWidgetInterfaceOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::ParaWidgetInterfaceOperator>("actionMeshGeoDelete");
        if (graphOper) {
            QObject* object = new QObject();
            object->setObjectName("actionMeshGeoDelete");
            graphOper->setEmitter(object);
            graphOper->actionTriggered();
        }

        this->accept();
    }

    void GeometryDeleteDialog::on_pushButton_Cancel_clicked()
    {
        this->reject();
    }
}
