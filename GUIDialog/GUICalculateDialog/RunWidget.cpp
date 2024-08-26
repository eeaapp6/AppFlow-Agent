#include "RunWidget.h"
#include "ui_RunWidget.h"

namespace GUI
{
    RunWidget::RunWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent) :
        QWidget(parent), _oper(oper)
    {
        _ui = new Ui::RunWidget();
        _ui->setupUi(this);
    }

    RunWidget::~RunWidget()
    {
        if (_ui)delete _ui;
    }
}