#include "BoundaryWidget.h"
#include "ui_BoundaryWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace GUI
{
    BoundaryWidget::BoundaryWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUICalculateWidgetBase(oper, parent)
    {
        _ui = new Ui::BoundaryWidget();
        _ui->setupUi(this);
    }

    BoundaryWidget::~BoundaryWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }
    void BoundaryWidget::init()
    {

    }
}

