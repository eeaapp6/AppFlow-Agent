#include "BoundaryWidget.h"
#include "ui_BoundaryWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace GUI
{
    BoundaryWidget::BoundaryWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUICalculateWidgetBase(oper, parent)
    {
        _ui = new Ui::BoundaryWidget();
        _ui->setupUi(this);
    }

    BoundaryWidget::~BoundaryWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }
    void BoundaryWidget::init()
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
    void BoundaryWidget::updateTableWidget()
    {
    }
}

