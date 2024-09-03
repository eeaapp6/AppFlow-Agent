#include "DiscretizationWidget.h"
#include "ui_DiscretizationWidget.h"
#include "CompCalLineWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowPhysicsHandlerFactory.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFDiscretization.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractParameter.h"

#include <QToolBox>
#include <QTabBar>

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
        updateConvectionWidget();
        updateGradientsWidget();
        updateInterpolationWidget();
    }

    void DiscretizationWidget::showEvent(QShowEvent * event)
    {
        Q_UNUSED(event);
        int width = _ui->tabWidget->width();
        int tabCount = _ui->tabWidget->count();
        int tabWidth = width / tabCount;
        _ui->tabWidget->tabBar()->setStyleSheet(QString("QTabBar::tab{width:%1px;height:30px;}").arg(tabWidth));
    }

    void DiscretizationWidget::resizeEvent(QResizeEvent * event)
    {
        Q_UNUSED(event);
        int width = _ui->tabWidget->width();
        int tabCount = _ui->tabWidget->count();
        int tabWidth = width / tabCount;
        _ui->tabWidget->tabBar()->setStyleSheet(QString("QTabBar::tab{width:%1px;height:30px;}").arg(tabWidth));
    }

    void DiscretizationWidget::updateTimeWidget()
    {
        if (_disValue == nullptr)return;
        auto timeValue = _disValue->getTimeOption();
        if (timeValue == nullptr)return;
        
        for (auto dataBase : timeValue->getParameter())
        {
            if (dataBase == nullptr)continue;
            QWidget* widget = CompCalLineWidget::DataSwitchToWidget(dataBase, this);
            _ui->verticalLayout_Time->addWidget(widget);
        }
    }

    void DiscretizationWidget::updateConvectionWidget()
    {
        if (_disValue == nullptr)return;
        QToolBox* toolBox = CompCalLineWidget::CreateToolBox(this);

        int conNum = _disValue->getConvectionCount();
        for (int i = 0; i < conNum; i++) {
            auto conValue = _disValue->getConvectionVPara(i);
            QString conName = _disValue->getConvectionVName(i);
            if(conValue == nullptr)continue;

            for (auto subValue : conValue->getParameter()){
                if (subValue == nullptr)continue;
                QWidget* widget = CompCalLineWidget::DataSwitchToWidget(subValue, this);
                toolBox->addItem(widget, conName);
            }
        }

        _ui->verticalLayout_Convection->addWidget(toolBox);
    }

    void DiscretizationWidget::updateGradientsWidget()
    {
        if (_disValue == nullptr)return;
        auto gradValue = _disValue->getGradients();
        if (gradValue == nullptr)return;

        QToolBox* toolBox = CompCalLineWidget::CreateToolBox(this);

        for (auto value : gradValue->getParameter()) {
            if (value == nullptr)continue;
            QWidget* widget = CompCalLineWidget::DataSwitchToWidget(value, this);
            toolBox->addItem(widget, value->getDataObjectName());
        }

        _ui->verticalLayout_Gradients->addWidget(toolBox);
    }

    void DiscretizationWidget::updateInterpolationWidget()
    {
        if (_disValue == nullptr)return;
        auto intValue = _disValue->getInterpolation();
        if (intValue == nullptr)return;

        for (auto value : intValue->getParameter()) {
            QWidget* widget = new CompCalLineWidget(value, this);
            _ui->verticalLayout_Interpolation->addWidget(widget);
        }
    }
}

