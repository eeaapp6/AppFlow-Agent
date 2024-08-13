#ifndef _DiscretizationWidget_H
#define _DiscretizationWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUICalculateWidgetBase.h"

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class GUICalculateDialogAPI DiscretizationWidget :public GUICalculateWidgetBase
    {
        Q_OBJECT;
    public:
        DiscretizationWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        ~DiscretizationWidget();
    };
}
#endif
