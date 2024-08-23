#include "InitialWidget.h"
#include "ui_InitialWidget.h"
#include "compCalLineWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFInitialConditions.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractParameter.h"

namespace GUI
{
    InitialWidget::InitialWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUICalculateWidgetBase(oper, parent)
    {
        _ui = new Ui::InitialWidget();
        _ui->setupUi(this);

        init();
    }

    InitialWidget::~InitialWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }
    void InitialWidget::init()
    {
        if (_physicsData == nullptr)return;
        _initValue = _physicsData->getInitialConditions();
        updateBasicWidget();
    }

    void InitialWidget::updateBasicWidget()
    {
        if (_initValue == nullptr)return;
        auto basicValue = _initValue->getBasicData();
        if (basicValue == nullptr)return;

        for (auto v : basicValue->getParameter()) {
            if (v == nullptr)continue;
            QWidget* widget = new compCalLineWidget(v, this);
            _ui->verticalLayout_Basic->addWidget(widget);
        }
    }
}

