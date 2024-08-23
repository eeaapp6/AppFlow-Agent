#include "SolutionWidget.h"
#include "ui_SolutionWidget.h"
#include "compCalLineWidget.h"
#include "compVBoxWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractOFSolver.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFSolution.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractParameter.h"

#include <QToolBox>

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
        if (_physicsData == nullptr)return;
        _solValue = _physicsData->getSolution();

        updateSlovers();
        updateSlover();
        updateResiduals();
        updateRelaxation();
        updateLimits();
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

    void SolutionWidget::updateSlovers()
    {
        if (_solValue == nullptr)return;
    }

    void SolutionWidget::updateSlover()
    {
        if (_physicsData == nullptr)return;
        auto solver = _physicsData->getSolver();
        if (solver == nullptr)return;
        _ui->tabWidget->setTabText(1, solver->getDataObjectName());
        if (_solValue == nullptr)return;

        auto solverValue = _solValue->getSolverSpeciallyDataPara();
        if (solverValue == nullptr)return;
        
        for (auto value : solverValue->getParameter()) {
            if (value == nullptr)continue;
            QWidget* widget = new compCalLineWidget(value, this);
            _ui->verticalLayout_Solver->addWidget(widget);
        }
    }

    void SolutionWidget::updateResiduals()
    {
        if (_solValue == nullptr)return;

        QToolBox* toolBox = compCalLineWidget::CreateToolBox(this);

        int resNum = _solValue->getResidualsCount();
        for (int i = 0; i < resNum; i++) {
            QString name = _solValue->getResidualVariableName(i);
            auto value = _solValue->getResidualVariablePara(i);
            QList<QWidget*> widgets = {};
            for (auto v : value->getParameter()) {
                if (v == nullptr)return;
                QWidget* widget = new compCalLineWidget(v, this);
                widgets.append(widget);
            }
            compVBoxWidget* VBoxWidget = new compVBoxWidget(widgets, this);
            toolBox->addItem(VBoxWidget, name);
        }

        _ui->verticalLayout_Residuals->addWidget(toolBox);
    }

    void SolutionWidget::updateRelaxation()
    {
        if (_solValue == nullptr)return;
        auto relValue = _solValue->getRelaxation();
        if (relValue == nullptr)return;

        for (auto v : relValue->getParameter()) {
            if (v == nullptr)continue;
            QWidget* widget = new compCalLineWidget(v, this);
            _ui->verticalLayout_Relaxation->addWidget(widget);
        }
    }

    void SolutionWidget::updateLimits()
    {
        if (_solValue == nullptr)return;
        auto limValue = _solValue->getLimits();
        if (limValue == nullptr)return;

        for (auto v : limValue->getParameter()) {
            if (v == nullptr)continue;
            QWidget* widget = compCalLineWidget::DataSwitchToWidget(v, this);
            _ui->verticalLayout_Limits->addWidget(widget);
        }
    }
}

