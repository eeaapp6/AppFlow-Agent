#ifndef _SolutionWidget_H
#define _SolutionWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUICalculateWidgetBase.h"

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class GUICalculateDialogAPI SolutionWidget :public GUICalculateWidgetBase
    {
        Q_OBJECT;
    public:
        SolutionWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        ~SolutionWidget();
    };
}
#endif
