#ifndef _InitialWidget_H
#define _InitialWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUICalculateWidgetBase.h"

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class GUICalculateDialogAPI InitialWidget :public GUICalculateWidgetBase
    {
        Q_OBJECT;
    public:
        InitialWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        ~InitialWidget();
    };
}
#endif
