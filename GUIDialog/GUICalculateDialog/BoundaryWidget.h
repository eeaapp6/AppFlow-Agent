#ifndef _BoundaryWidget_H
#define _BoundaryWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUICalculateWidgetBase.h"

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class GUICalculateDialogAPI BoundaryWidget :public GUICalculateWidgetBase
    {
        Q_OBJECT;
    public:
        BoundaryWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        ~BoundaryWidget();
    };
}
#endif
