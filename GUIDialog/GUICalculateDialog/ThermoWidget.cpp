#include "ThermoWidget.h"
#include "ui_ThermoWidget.h"
#include "CalculateDriver.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKSignalTransfer.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"
#include "FITK_Kernel/FITKAppFramework/FITKProgramTaskManager.h"
#include "FITK_Kernel/FITKCore/FITKDirFileTools.h"
#include "FITK_Kernel/FITKEasyParam/FITKWidgetComLine.h"
#include "FITK_Kernel/FITKEasyParam/FITKEasyParamWidgetFactory.h"
#include "FITK_Kernel/FITKEasyParam/FITKParameter.h"

#include "FITK_Component/FITKOFDriver/FITKOFInputInfo.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFThermo.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractOFSolver.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowPhysicsHandlerFactory.h"

#include <QButtonGroup>
#include <QProcess>
#include <QDir>
#include <QTextStream>
#include <QTabBar>


namespace GUI
{

    ThermoWidget::ThermoWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent) :
        GUICalculateWidgetBase(oper,parent)
    {
        _ui = new Ui::ThermoWidget();
        _ui->setupUi(this);

        init();
    }

    ThermoWidget::~ThermoWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void ThermoWidget::init()
    {

    }

    void ThermoWidget::showEvent(QShowEvent * event)
    {
        Q_UNUSED(event);
        //int width = _ui->tabWidget->width();
        //int tabCount = _ui->tabWidget->count();
        //int tabWidth = width / tabCount;
        //_ui->tabWidget->tabBar()->setStyleSheet(QString("QTabBar::tab{width:%1px;height:30px;}").arg(tabWidth));
    }

    void ThermoWidget::resizeEvent(QResizeEvent * event)
    {
        Q_UNUSED(event);
        //int width = _ui->tabWidget->width();
        //int tabCount = _ui->tabWidget->count();
        //int tabWidth = width / tabCount;
        //_ui->tabWidget->tabBar()->setStyleSheet(QString("QTabBar::tab{width:%1px;height:30px;}").arg(tabWidth));
    }

}
