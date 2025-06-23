#include "BoundaryCreateDialog.h"
#include "ui_BoundaryCreateDialog.h"

#include "OperatorsInterface/GraphEventOperator.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"

#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Interface/FITKInterfaceMesh/FITKUnstructuredFluidMeshVTK.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFEnum.hpp"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFBoundary.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowPhysicsHandlerFactory.h"

//类型注册
Q_DECLARE_METATYPE(Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType)

namespace GUI
{
    BoundaryCreateDialog::BoundaryCreateDialog(EventOper::ParaWidgetInterfaceOperator * oper) :
        GUIDialogBase(FITKAPP->getGlobalData()->getMainWindow()), _oper(oper)
    {
        _ui = new Ui::BoundaryCreateDialog();
        _ui->setupUi(this);

        _physicsData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        _meshData = FITKAPP->getGlobalData()->getMeshData< Interface::FITKUnstructuredFluidMeshVTK>();
        _factoryData = FITKAPP->getComponents()->getComponentTByName<Interface::FITKFlowPhysicsHandlerFactory>("FITKFlowPhysicsHandlerFactory");
        init();

        this->setWindowTitle(tr("Create Boundary"));
    }

    BoundaryCreateDialog::~BoundaryCreateDialog()
    {
        if (_ui)delete _ui;
    }

    void BoundaryCreateDialog::init()
    {
        if (_physicsData == nullptr || _meshData == nullptr)return;
        //获取边界属性数据管理器
        Interface::FITKOFBoundaryManager* boundaryManager = _physicsData->getBoundaryManager();
        if (boundaryManager == nullptr)return;

        //界面名称添加
        _ui->lineEdit_Name->setText(boundaryManager->checkName("Boundary-0"));

        //初始化网格区域下拉框
        int count = _meshData->getDataCount();
        for (int i = 0; i < count; ++i)
        {
            Interface::FITKFluidRegionsMesh* region = _meshData->getDataByIndex(i);
            if (!region) continue;
            _ui->comboBox_MeshRegion->addItem(region->getDataObjectName(), region->getDataObjectID());
        }
        //初始化边界域下拉框
        for (int i = 0; i < _ui->comboBox_MeshRegion->count(); ++i)
        {
            _ui->comboBox_MeshRegion->setCurrentIndex(i);
            this->initBoundaryComboBox();
            if (_ui->comboBox_Boundary->count() > 0)
                break;
        }
        
        //边界类型添加
        _ui->comboBox_Type->addItem(tr("Wall"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BWall);
        _ui->comboBox_Type->addItem(tr("Pressure Inlet"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BPressureInlet);
        _ui->comboBox_Type->addItem(tr("Velocity Inlet"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BVelocityInlet);
        _ui->comboBox_Type->addItem(tr("Pressure Outlet"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BPressureOutlet);
        _ui->comboBox_Type->addItem(tr("Outflow"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BOutflow);
        _ui->comboBox_Type->addItem(tr("Symmetry"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BSymmetry);
        _ui->comboBox_Type->addItem(tr("Wedge"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BWedge);
        _ui->comboBox_Type->addItem(tr("Mapped Wall"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BMappedWall);
    }

    void BoundaryCreateDialog::hideEvent(QHideEvent * event)
    {
        GUIDialogBase::hideEvent(event);
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;
        graphOper->clearHighlight();
    }

    void BoundaryCreateDialog::showEvent(QShowEvent * event)
    {
        GUIDialogBase::showEvent(event);
        int id = -1;
        if (_ui->comboBox_Boundary->count() > 0) {
            id = _ui->comboBox_Boundary->currentData().toInt();
        }
        highlightMeshBoundary(id);
    }

    void BoundaryCreateDialog::on_comboBox_Boundary_activated(int index)
    {
        Q_UNUSED(index);
        highlightMeshBoundary(_ui->comboBox_Boundary->currentData().toInt());
    }

    void BoundaryCreateDialog::on_comboBox_MeshRegion_activated(int index)
    {
        Q_UNUSED(index);
        //更新ComboBox_boundary组件
        this->initBoundaryComboBox();
        if (_ui->comboBox_Boundary->count() > 0)
            highlightMeshBoundary(_ui->comboBox_Boundary->currentData().toInt());
    }

    void BoundaryCreateDialog::on_pushButton_OK_clicked()
    {
        if (_ui->comboBox_Boundary->count() == 0)return;

        if (_physicsData == nullptr)return;
        auto boundManager = _physicsData->getBoundaryManager();
        if (boundManager == nullptr)return;
        if (_factoryData == nullptr)return;

        QString name = _ui->lineEdit_Name->text();
        _factoryData->setBoundary(_ui->comboBox_Boundary->currentData().toInt(), 
            _ui->comboBox_Type->currentData().value<Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType>());
        auto boundary = boundManager->getBoundary(_ui->comboBox_Boundary->currentData().toInt());
        if (boundary) {
            boundary->setDataObjectName(name);
        }
        
        if (_oper) {
            _oper->execProfession();
        }

        this->accept();
    }

    void BoundaryCreateDialog::on_pushButton_Cancel_clicked()
    {
        this->reject();
    }

    void BoundaryCreateDialog::initBoundaryComboBox()
    {
        _ui->comboBox_Boundary->clear();
        if (_physicsData == nullptr || _meshData == nullptr)return;
        //获取已存在的边界属性数据管理器
        Interface::FITKOFBoundaryManager* boundaryManager = _physicsData->getBoundaryManager();
        if (boundaryManager == nullptr)return;
        //获取边界网格管理器
        int regionMeshID = _ui->comboBox_MeshRegion->currentData().toInt();
        Interface::FITKFluidRegionsMesh* region = _meshData->getDataByID(regionMeshID);//获取区域数据
        if (!region) return;
        Interface::FITKBoundaryMeshVTKManager* boundMeshManager = region->getBoundaryMeshManager();
        if (boundMeshManager == nullptr)return;
        //初始化边界下拉框
        for (int i = 0; i < boundMeshManager->getDataCount(); i++) {
            Interface::FITKBoundaryMeshVTK* boundMesh = boundMeshManager->getDataByIndex(i);
            if (boundMesh == nullptr)continue;
            if (boundaryManager->getBoundary(boundMesh->getDataObjectID()))continue;
            _ui->comboBox_Boundary->addItem(boundMesh->getDataObjectName(), boundMesh->getDataObjectID());
        }
        _ui->comboBox_Boundary->setCurrentIndex(0);
    }

    void BoundaryCreateDialog::highlightMeshBoundary(int meshBoundID)
    {
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;
        graphOper->clearHighlight();
        graphOper->highlight(meshBoundID);
    }
}