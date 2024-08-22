#include "DiscretizationWidget.h"
#include "ui_DiscretizationWidget.h"
#include "compCalLineWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowPhysicsHandlerFactory.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFDiscretization.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractParameter.h"

namespace GUI
{
    DiscretizationWidget::DiscretizationWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUICalculateWidgetBase(oper, parent)
    {
        _ui = new Ui::DiscretizationWidget();
        _ui->setupUi(this);

        init();
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
        if (_physicsData == nullptr)return;
        _disValue = _physicsData->getDiscretization();
        updateWidget();
    }

    void DiscretizationWidget::updateWidget()
    {
        updateTimeWidget();
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

    void DiscretizationWidget::updateTimeWidget()
    {
        if (_disValue == nullptr)return;
        auto timeValue = _disValue->getTimeOption();
        if (timeValue == nullptr)return;
        
        for (auto dataBase : timeValue->getParameter())
        {
            if (dataBase == nullptr)continue;
            QWidget* widget = compCalLineWidget::DataSwitchToWidget(dataBase, this);
            _ui->verticalLayout_Time->addWidget(widget);
        }
    }
}

