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
        if (_physicsData == nullptr)return;
        auto boundManager = _physicsData->getBoundaryManager();
        if (boundManager == nullptr)return;
        auto globalData = FITKAPP->getGlobalData();
        if (globalData == nullptr)return;
        Interface::FITKUnstructuredFluidMeshVTK* meshData = globalData->getMeshData< Interface::FITKUnstructuredFluidMeshVTK>();
        if (meshData == nullptr)return;
        Interface::FITKBoundaryMeshVTKManager* boundMeshManager = meshData->getBoundaryMeshManager();
        if (boundMeshManager == nullptr)return;

        //名称添加
        _ui->lineEdit_Name->setText(boundManager->checkName("Boundary-0"));

        //边界选择添加
        for (int i = 0; i < boundMeshManager->getDataCount(); i++) {
            Interface::FITKBoundaryMeshVTK* boundMesh = boundMeshManager->getDataByIndex(i);
            if (boundMesh == nullptr)continue;
            if (boundManager->getBoundary(boundMesh->getDataObjectID()))continue;
            _ui->comboBox_Boundary->addItem(boundMesh->getDataObjectName(), boundMesh->getDataObjectID());
        }
        if (boundManager->getDataCount() > 0) _ui->comboBox_Boundary->setCurrentIndex(0);

        //边界类型添加
        _ui->comboBox_Type->addItem(tr("Wall"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BWall);
        _ui->comboBox_Type->addItem(tr("Pressure Inlet"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BPressureInlet);
        _ui->comboBox_Type->addItem(tr("Velocity Inlet"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BVelocityInlet);
        _ui->comboBox_Type->addItem(tr("Pressure Outlet"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BPressureOutlet);
        _ui->comboBox_Type->addItem(tr("Outflow"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BOutflow);
        _ui->comboBox_Type->addItem(tr("Symmetry"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BSymmetry);
        _ui->comboBox_Type->addItem(tr("Wedge"), Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BWedge);
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

    void BoundaryCreateDialog::highlightMeshBoundary(int meshBoundID)
    {
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;
        graphOper->clearHighlight();
        graphOper->highlight(meshBoundID);
    }
}