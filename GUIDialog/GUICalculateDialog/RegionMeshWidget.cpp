#include "RegionMeshWidget.h"
#include "ui_RegionMeshWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceMesh/FITKUnstructuredFluidMeshVTK.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFEnum.hpp"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"

#include <QComboBox>

#define MeshObjID Qt::UserRole

namespace GUI {

    RegionMeshWidget::RegionMeshWidget(EventOper::ParaWidgetInterfaceOperator * oper) :
        Core::FITKWidget(FITKAPP->getGlobalData()->getMainWindow()),
        _oper(oper)
    {
        _ui = new Ui::RegionMeshWidget();
        _ui->setupUi(this);
        init();
    }


    RegionMeshWidget::~RegionMeshWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void RegionMeshWidget::init()
    {
        //初始化表格
        _ui->tableWidget_Mesh->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        //充满表格
        _ui->tableWidget_Mesh->horizontalHeader()->setStretchLastSection(true);
        //设置只能单选
        _ui->tableWidget_Mesh->setSelectionMode(QAbstractItemView::SingleSelection);
        //隐藏行表头
        _ui->tableWidget_Mesh->verticalHeader()->setVisible(false);
        //隐藏列表头
        _ui->tableWidget_Mesh->horizontalHeader()->setVisible(false);
        //获取网格数据
        auto globalData = FITKAPP->getGlobalData();
        if (globalData == nullptr)return;
        Interface::FITKUnstructuredFluidMeshVTK* meshData = globalData->getMeshData< Interface::FITKUnstructuredFluidMeshVTK>();
        Interface::FITKOFPhysicsData* physicsData = globalData->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (meshData == nullptr || physicsData == nullptr)return;
        //初始化表格数据
        int count = meshData->getDataCount();
        _ui->tableWidget_Mesh->setRowCount(count);
        for (int i = 0; i < count; i++) {
            Interface::FITKFluidRegionsMesh* mesh = meshData->getDataByIndex(i);
            if (!mesh) continue;
            QString meshName = mesh->getDataObjectName();
            if (meshName.isEmpty()) continue;
            int type = physicsData->getRegionMeshType(mesh->getDataObjectID());
            _ui->tableWidget_Mesh->setColumnCount(2);
            QTableWidgetItem* item = new QTableWidgetItem(meshName);
            item->setData(MeshObjID, mesh->getDataObjectID());
            _ui->tableWidget_Mesh->setItem(i, 0, item);

            QComboBox* comboBox_Region = new QComboBox;
            comboBox_Region->addItem(tr("Fluid"), (int)Interface::FITKOFSolverTypeEnum::FITKOFRegionMeshType::Fluid);
            comboBox_Region->addItem(tr("Solid"), (int)Interface::FITKOFSolverTypeEnum::FITKOFRegionMeshType::Solid);
            comboBox_Region->setCurrentIndex(comboBox_Region->findData(type));
            if (count == 1)
                comboBox_Region->setEnabled(false);
            connect(comboBox_Region, SIGNAL(activated(int)), this, SLOT(setDataFormWidgetSlot(int)));
            _ui->tableWidget_Mesh->setCellWidget(i, 1, comboBox_Region);
        }
    }

    void RegionMeshWidget::closeEvent(QCloseEvent * event)
    {
        Core::FITKWidget::closeEvent(event);
    }

    void RegionMeshWidget::setDataFormWidgetSlot(int index)
    {
        Q_UNUSED(index);
        //获取网格数据和物理数据
        auto globalData = FITKAPP->getGlobalData();
        if (globalData == nullptr)return;
        Interface::FITKUnstructuredFluidMeshVTK* meshData = globalData->getMeshData< Interface::FITKUnstructuredFluidMeshVTK>();
        Interface::FITKOFPhysicsData* physicsData = globalData->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (meshData == nullptr || physicsData == nullptr)return;
        //获取界面数据更新数据层
        int rowCount = _ui->tableWidget_Mesh->rowCount();
        for (int iRow = 0; iRow < rowCount; ++iRow)
        {
            QTableWidgetItem* item = _ui->tableWidget_Mesh->item(iRow, 0);
            QComboBox* comboBox_Region = dynamic_cast<QComboBox*>(_ui->tableWidget_Mesh->cellWidget(iRow, 1));
            if (!item || !comboBox_Region) continue;
            int id = item->data(MeshObjID).toInt();
            Interface::FITKOFSolverTypeEnum::FITKOFRegionMeshType type = (Interface::FITKOFSolverTypeEnum::FITKOFRegionMeshType)comboBox_Region->currentData().toInt();
            physicsData->setRegionMeshType(id, type);
        }
    }
}
