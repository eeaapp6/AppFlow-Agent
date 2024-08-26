#include "SolutionWidget.h"
#include "ui_SolutionWidget.h"
#include "CompCalLineWidget.h"
#include "CompVBoxWidget.h"
#include "CompSelectComBoxWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractOFSolver.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFSolution.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractParameter.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsManager.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFSolutionSolverManager.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowPhysicsHandlerFactory.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFSolutionSolver.h"

#include <QToolBox>

namespace GUI
{
    Interface::FITKAbstractParameter* solutionGetSubData(const QString & type, int index)
    {
        auto phyFactory = FITKAPP->getComponents()->getComponentTByName<Interface::FITKFlowPhysicsHandlerFactory>("FITKFlowPhysicsHandlerFactory");
        if (phyFactory == nullptr)return nullptr;
        auto phyData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (phyData == nullptr)return nullptr;

        phyFactory->setSolutionSolver(index, type);
        Interface::FITKOFAbsSolutionSolver* solution = phyData->getSolution()->getSolverVariablePara(index);
        if (solution == nullptr)return nullptr;

        return solution->getSolverSolutionPara();
    }

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
        Q_UNUSED(event);
        int width = _ui->tabWidget->width();
        int tabCount = _ui->tabWidget->count();
        int tabWidth = width / tabCount;
        this->setStyleSheet(QString("QTabBar::tab{width:%1px;height:30px;}").arg(tabWidth));
    }

    void SolutionWidget::resizeEvent(QResizeEvent * event)
    {
        Q_UNUSED(event);
        int width = _ui->tabWidget->width();
        int tabCount = _ui->tabWidget->count();
        int tabWidth = width / tabCount;
        this->setStyleSheet(QString("QTabBar::tab{width:%1px;height:30px;}").arg(tabWidth));
    }

    void SolutionWidget::updateSlovers()
    {
        if (_physicsManager == nullptr)return;
        auto solutionManager = _physicsManager->getSolutionSolverManager();
        if (solutionManager == nullptr)return;
        if (_factoryData == nullptr)return;
        if (_solValue == nullptr)return;
        int solversNum = _solValue->getSolversCount();

        QToolBox* toolBox = CompCalLineWidget::CreateToolBox(this);

        for (int i = 0; i < solversNum; i++) {
            Interface::FITKOFAbsSolutionSolver* solversData = _solValue->getSolverVariablePara(i);
            if(solversData == nullptr)continue;
            QString type = _solValue->getSolverVariableName(i);
            QStringList options = solutionManager->filterSolutionSolvers(type, _physicsData->getSolver()->getSolverType());
            CompSelectComBoxWidget* comp = new CompSelectComBoxWidget(type, toolBox);
            comp->setFunction(&solutionGetSubData, i);
            comp->setOptions(options);
            comp->setCurrentText(solversData->getDataObjectName());
            comp->setSubWidgetData(solversData->getSolverSolutionPara());
            toolBox->addItem(comp, type);
        }
        _ui->verticalLayout_Solvers->addWidget(toolBox);
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
            QWidget* widget = new CompCalLineWidget(value, this);
            _ui->verticalLayout_Solver->addWidget(widget);
        }
    }

    void SolutionWidget::updateResiduals()
    {
        if (_solValue == nullptr)return;

        QToolBox* toolBox = CompCalLineWidget::CreateToolBox(this);

        int resNum = _solValue->getResidualsCount();
        for (int i = 0; i < resNum; i++) {
            QString name = _solValue->getResidualVariableName(i);
            auto value = _solValue->getResidualVariablePara(i);
            QList<QWidget*> widgets = {};
            for (auto v : value->getParameter()) {
                if (v == nullptr)return;
                QWidget* widget = new CompCalLineWidget(v, this);
                widgets.append(widget);
            }
            CompVBoxWidget* VBoxWidget = new CompVBoxWidget(widgets, this);
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
            QWidget* widget = new CompCalLineWidget(v, this);
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
            QWidget* widget = CompCalLineWidget::DataSwitchToWidget(v, this);
            _ui->verticalLayout_Limits->addWidget(widget);
        }
    }
}

