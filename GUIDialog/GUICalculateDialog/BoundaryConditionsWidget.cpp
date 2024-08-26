#include "BoundaryConditionsWidget.h"
#include "ui_BoundaryConditionsWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace GUI
{
    BoundaryConditionsWidget::BoundaryConditionsWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUICalculateWidgetBase(oper, parent)
    {
        _ui = new Ui::BoundaryConditionsWidget();
        _ui->setupUi(this);
    }

    BoundaryConditionsWidget::~BoundaryConditionsWidget()
    {
        if (_ui) {
            delete _ui;
            _ui = nullptr;
        }
    }
    void BoundaryConditionsWidget::init()
    {

    }
}

