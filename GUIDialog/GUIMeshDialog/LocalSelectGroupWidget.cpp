#include "LocalSelectGroupWidget.h"
#include "ui_LocalSelectGroupWidget.h"

#include "GUIFrame/MainWindow.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoModelManager.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoCommand.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"

namespace GUI
{
    LocalSelectGroupWidget::LocalSelectGroupWidget(EventOper::ParaWidgetInterfaceOperator* oper) :
        GUIWidgetBase(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _oper(oper)
    {
        _ui = new Ui::LocalSelectGroupWidget();
        _ui->setupUi(this);

        _ui->tableWidget->setRowCount(0);
        _ui->tableWidget->setColumnCount(1);
        //自适应
        _ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        //隐藏列表头
        _ui->tableWidget->verticalHeader()->setVisible(false);
        //隐藏行表头
        _ui->tableWidget->horizontalHeader()->setVisible(false);
        //设置不可编辑
        _ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

        connect(_ui->tableWidget, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(itemTableClickedSlot(QTableWidgetItem*)));

        init();
    }

    LocalSelectGroupWidget::~LocalSelectGroupWidget()
    {
        if (_ui)delete _ui;
    }

    void LocalSelectGroupWidget::init()
    {
        _ui->tableWidget->setRowCount(0);
        int currentRow = 0;

        Interface::FITKOFGeometryData* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKOFGeometryData>();
        if (geometryData == nullptr) return;
        for (int i = 0; i < geometryData->getDataCount(); i++) {
            auto geometryObj = dynamic_cast<Interface::FITKAbsGeoCommand*>(geometryData->getDataByIndex(i));
            if (geometryObj == nullptr)continue;
            QString geoName = geometryObj->getDataObjectName();
            //FITKAbsGeoCommand无法直接获取到FITKGeoComponentManager，通过FITKAbstractGeoModel获取
            auto geoModelData = dynamic_cast<Interface::FITKAbstractGeoModel*>(geometryData->getDataByID(geometryObj->getDataObjectID()));
            if (geoModelData == nullptr)continue;
            Interface::FITKGeoComponentManager* compManager = geoModelData->getGeoComponentManager();
            if (compManager == nullptr)continue;
            for (int j = 0; j < compManager->getDataCount(); j++) {
                Interface::FITKGeoComponent* geoCom = compManager->getDataByIndex(j);
                if (geoCom == nullptr)continue;
                QString comName = geoCom->getDataObjectName();

                QTableWidgetItem* item = new QTableWidgetItem(geoName + "." + comName);
                item->setData(Qt::UserRole, geoCom->getDataObjectID());
                item->setData(Qt::UserRole + 1, geoCom->getDataObjectID());
                _ui->tableWidget->insertRow(currentRow);
                _ui->tableWidget->setItem(currentRow, 0, item);
                currentRow++;
            }
        }
    }

    void LocalSelectGroupWidget::on_pushButton_Cancel_clicked()
    {
        if (_oper) {
            _oper->execProfession();
        }
    }

    void LocalSelectGroupWidget::on_pushButton_OK_clicked()
    {
        int currentRow = _ui->tableWidget->currentRow();
        QTableWidgetItem* item = _ui->tableWidget->item(currentRow, 0);
        /*if
        if (_oper) {
            _oper->execProfession();
        }*/
    }
}

