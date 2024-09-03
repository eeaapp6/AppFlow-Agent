#include "TransportWidget.h"
#include "ui_TransportWidget.h"
#include "CompTranPhasesWidget.h"
#include "CompCalLineWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFTransportProp.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractParameter.h"
#include "FITK_Component/FITKWidget/FITKTabWidget.h"

#include <QToolBox>

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
        
        Interface::FITKAbstractParameter* paraData =_tranData->getTransportAdditionalData();
        if (paraData) {
            for (auto d : paraData->getParameter()){
                if (d == nullptr)continue;
                _ui->verticalLayout->addWidget(new CompCalLineWidget(d, this));
            }
        }

        Comp::FITKTabWidget* tabWidget = new Comp::FITKTabWidget(Comp::FITKTabWidgetType::FITKTab_Auto, this);
        int dataNum = _tranData->getPhasesCount();
        for (int i = 0; i < dataNum; i++) {
            if (_tranData->getPhase(i) == nullptr)continue;;
            CompTranPhasesWidget* widget = new CompTranPhasesWidget(_tranData->getPhase(i), i, this);
            tabWidget->addTab(widget, _tranData->getPhase(i)->getPhaseName());
        }
        _ui->verticalLayout->addWidget(tabWidget);
    }
}

