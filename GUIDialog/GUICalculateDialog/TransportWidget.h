#ifndef _TransportWidget_H
#define _TransportWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUICalculateWidgetBase.h"

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class GUICalculateDialogAPI TransportWidget :public GUICalculateWidgetBase
    {
        Q_OBJECT;
    public:
        TransportWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        ~TransportWidget();
    };
}
#endif
