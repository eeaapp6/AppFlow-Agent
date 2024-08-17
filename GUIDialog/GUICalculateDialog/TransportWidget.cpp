#include "TransportWidget.h"
#include "ui_TransportWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace GUI
{
    TransportWidget::TransportWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUICalculateWidgetBase(oper, parent)
    {
        _ui = new Ui::TransportWidget();
        _ui->setupUi(this);
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
        updateTableWidget();
    }
    void TransportWidget::updateTableWidget()
    {
    }
}

