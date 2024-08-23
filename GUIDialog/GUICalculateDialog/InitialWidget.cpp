#include "InitialWidget.h"
#include "ui_InitialWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace GUI
{
    InitialWidget::InitialWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUICalculateWidgetBase(oper, parent)
    {
        _ui = new Ui::InitialWidget();
        _ui->setupUi(this);
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

    }
}

