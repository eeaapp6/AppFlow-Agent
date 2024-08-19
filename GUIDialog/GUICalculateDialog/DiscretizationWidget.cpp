#include "DiscretizationWidget.h"
#include "ui_DiscretizationWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace GUI
{
    DiscretizationWidget::DiscretizationWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUICalculateWidgetBase(oper, parent)
    {
        _ui = new Ui::DiscretizationWidget();
        _ui->setupUi(this);
    }

    DiscretizationWidget::~DiscretizationWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void DiscretizationWidget::init()
    {
        updateWidget();
    }

    void DiscretizationWidget::updateWidget()
    {

    }

    void DiscretizationWidget::showEvent(QShowEvent * event)
    {
        int width = _ui->tabWidget->width();
        int tabCount = _ui->tabWidget->count();
        int tabWidth = width / tabCount;
        this->setStyleSheet(QString("QTabBar::tab{width:%1px;height:30px;}").arg(tabWidth));
    }

    void DiscretizationWidget::resizeEvent(QResizeEvent * event)
    {
        int width = _ui->tabWidget->width();
        int tabCount = _ui->tabWidget->count();
        int tabWidth = width / tabCount;
        this->setStyleSheet(QString("QTabBar::tab{width:%1px;height:30px;}").arg(tabWidth));
    }
}

