#include "MaterialPointWidget.h"
#include "ui_MaterialPointWidget.h"
#include "CompMaterialPointWidget.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "GUIWidget/PickedData.h"
#include "GUIWidget/PickedDataProvider.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"
#include "OperatorsInterface/GraphEventOperator.h"
#include "OperatorsInterface/GraphInteractionOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"

namespace GUI
{
    MaterialPointWidget::MaterialPointWidget(EventOper::ParaWidgetInterfaceOperator * oper) :
        GUIWidgetBase(dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow())),
        _oper(oper)
    {
        _ui = new Ui::MaterialPointWidget();
        _ui->setupUi(this);
        init();
    }

    MaterialPointWidget::~MaterialPointWidget()
    {
        if (_ui)delete _ui;
    }

    void MaterialPointWidget::init()
    {
        initTableWidget();
    }

    void MaterialPointWidget::initTableWidget()
    {
        _ui->tableWidget->setRowCount(0);
        _ui->tableWidget->setColumnCount(1);
        QStringList header;
        header << tr("");
        _ui->tableWidget->setHorizontalHeaderLabels(header);
        _ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        //充满表格
        _ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
        //隐藏列表头
        _ui->tableWidget->verticalHeader()->setVisible(false);
        //隐藏行表头
        _ui->tableWidget->horizontalHeader()->setVisible(false);
        //设置不可编辑
        _ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

        connect(_ui->tableWidget, SIGNAL(cellClicked(int, int)), this, SLOT(slotCellTableClicked(int, int)));
    }

    void MaterialPointWidget::on_pushButton_Add_clicked()
    {
        int rowNum = _ui->tableWidget->rowCount();
        _ui->tableWidget->setRowCount(rowNum + 1);

        QString name = tr("none");

        CompMaterialPointWidget* widget = new CompMaterialPointWidget(_ui->tableWidget);
        widget->setName(name);
        _ui->tableWidget->setCellWidget(rowNum, 0, widget);

        connect(widget, SIGNAL(sigDeleteClicked()), this, SLOT(slotMatPointWidgetDeleteClicked()));

        updateFaceWidgetCurrentPos();
    }

    void MaterialPointWidget::slotCellTableClicked(int row, int column)
    {
    }

    void MaterialPointWidget::slotMatPointWidgetDeleteClicked()
    {
        CompMaterialPointWidget* widget = dynamic_cast<CompMaterialPointWidget*>(sender());
        if (widget == nullptr) return;
        _ui->tableWidget->removeRow(widget->getCurrentPos().first);
        //更新界面中存储的位置
        updateFaceWidgetCurrentPos();
        //清除高亮
        clearGraphHight();
    }

    void MaterialPointWidget::updateFaceWidgetCurrentPos()
    {
        for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
            CompMaterialPointWidget* widget = dynamic_cast<CompMaterialPointWidget*>(_ui->tableWidget->cellWidget(i, 0));
            if (widget == nullptr)return;
            widget->setCurrentPos(i, 0);
        }
    }

    void MaterialPointWidget::clearGraphHight()
    {
        //退出选择模式
        GraphData::PickedDataProvider* pickD = GraphData::PickedDataProvider::getInstance();
        if (pickD == nullptr) return;
        //拾取信息设置
        GUI::GUIPickInfoStru pinfo;
        pinfo._pickObjType = GUI::GUIPickInfo::PickObjType::POBJNone;
        pinfo._pickMethod = GUI::GUIPickInfo::PickMethod::PMNone;
        GUI::GUIPickInfo::SetPickInfo(pinfo);
        pickD->clearPickedData();

        //刷新渲染窗口
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;
        graphOper->reRender();
    }

    void MaterialPointWidget::clearTableWidget()
    {
        for (int i = 0; i < _ui->tableWidget->rowCount(); i++) {
            QWidget* widget = _ui->tableWidget->cellWidget(i, 0);
            if (widget == nullptr)continue;
            delete widget;
            widget = nullptr;
        }

        _ui->tableWidget->clear();
    }
}

