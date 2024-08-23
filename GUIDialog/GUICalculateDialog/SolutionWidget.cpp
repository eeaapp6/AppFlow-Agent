#include "SolutionWidget.h"
#include "ui_SolutionWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace GUI
{
    SolutionWidget::SolutionWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUICalculateWidgetBase(oper, parent)
    {
        _ui = new Ui::SolutionWidget();
        _ui->setupUi(this);

        init();
    }

    SolutionWidget::~SolutionWidget()
    {
        if (_ui) delete _ui;
    }

    void SolutionWidget::init()
    {

    }

    void SolutionWidget::showEvent(QShowEvent * event)
    {
        int width = _ui->tabWidget->width();
        int tabCount = _ui->tabWidget->count();
        int tabWidth = width / tabCount;
        this->setStyleSheet(QString("QTabBar::tab{width:%1px;height:30px;}").arg(tabWidth));
    }

    void SolutionWidget::resizeEvent(QResizeEvent * event)
    {
        int width = _ui->tabWidget->width();
        int tabCount = _ui->tabWidget->count();
        int tabWidth = width / tabCount;
        this->setStyleSheet(QString("QTabBar::tab{width:%1px;height:30px;}").arg(tabWidth));
    }
}

