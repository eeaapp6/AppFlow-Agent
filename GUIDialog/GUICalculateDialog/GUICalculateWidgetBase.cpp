#include "GUICalculateWidgetBase.h"

namespace GUI
{
    GUICalculateWidgetBase::GUICalculateWidgetBase(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUIWidgetBase(parent), _oper(oper)
    {

    }

    GUICalculateWidgetBase::~GUICalculateWidgetBase()
    {

    }
}
