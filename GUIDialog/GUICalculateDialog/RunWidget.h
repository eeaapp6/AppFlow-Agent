#ifndef _RunWidget_H
#define _RunWidget_H

#include <QWidget>
#include "GUICalculateDialogAPI.h"

namespace Ui {
    class RunWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class GUICalculateDialogAPI RunWidget : public QWidget
    {
        Q_OBJECT;
    public:
        RunWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        ~RunWidget();
    private:
        Ui::RunWidget* _ui = nullptr;
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
    };
}

#endif
