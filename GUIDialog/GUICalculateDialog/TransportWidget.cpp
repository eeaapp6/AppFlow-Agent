#include "TransportWidget.h"
#include "ui_TransportWidget.h"
#include "CompTranPhasesWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFTransportProp.h"

namespace GUI
{
    TransportWidget::TransportWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUICalculateWidgetBase(oper, parent)
    {
        _ui = new Ui::TransportWidget();
        _ui->setupUi(this);

        init();

        _ui->pushButton_MaterialDataBase->hide();
    }

    TransportWidget::~TransportWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }

    void TransportWidget::init()
    {
        if (_physicsData == nullptr)return;
        _tranData = _physicsData->getTransportProp();
        if (_tranData == nullptr)return;
        int dataNum = _tranData->getPhasesCount();
        for (int i = 0; i < dataNum; i++) {
            CompTranPhasesWidget* widget = new CompTranPhasesWidget(_tranData->getPhase(i), i, this);
            _ui->verticalLayout->addWidget(widget);
        }
    }
}

