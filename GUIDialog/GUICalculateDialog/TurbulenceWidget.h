#ifndef _TurbulenceWidget_H
#define _TurbulenceWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUICalculateWidgetBase.h"

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class GUICalculateDialogAPI TurbulenceWidget :public GUICalculateWidgetBase
    {
        Q_OBJECT;
    public:
        TurbulenceWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        ~TurbulenceWidget();
    };
}
#endif
