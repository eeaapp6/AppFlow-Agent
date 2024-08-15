#include "TurbulenceWidget.h"
#include "ui_TurbulenceWidget.h"
#include "DataSwitchToWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFSolverData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractParameter.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFTurbulenceData.h"

namespace GUI
{
    TurbulenceWidget::TurbulenceWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUICalculateWidgetBase(oper, parent)
    {
        _ui = new Ui::TurbulenceWidget();
        _ui->setupUi(this);
        init();
    }

    TurbulenceWidget::~TurbulenceWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void TurbulenceWidget::init()
    {
        //自适应布局（更具表头数量平均分配大小）
        _ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        //设置tableWidget的item不可编辑
        _ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        //隐藏行表头
        _ui->tableWidget->verticalHeader()->setVisible(false);
        //隐藏列表头
        _ui->tableWidget->horizontalHeader()->setVisible(false);
        //隐藏网格线
        _ui->tableWidget->setShowGrid(false);

        updateTableWidget();
    }

    void TurbulenceWidget::updateTableWidget()
    {
        if (_solverData == nullptr)return;
        Interface::FITKOFTurbulenceData* turData = _solverData->getTurbulence();
        if (turData == nullptr)return;

        //清空表格
        _ui->tableWidget->clear();
        _ui->tableWidget->setRowCount(0);
        _ui->tableWidget->setColumnCount(0);

        DataGroupSwitchToWidget dataWidget;
        dataWidget.dataToWidget(_ui->tableWidget, turData, this);
    }
}

