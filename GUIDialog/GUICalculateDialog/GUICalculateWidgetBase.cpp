#include "GUICalculateWidgetBase.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFSolverData.h"

namespace GUI
{
    GUICalculateWidgetBase::GUICalculateWidgetBase(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUIWidgetBase(parent), _oper(oper)
    {
        _solverData = FITKAPP->getGlobalData()->getPostData<Interface::FITKOFSolverData>();
    }

    GUICalculateWidgetBase::~GUICalculateWidgetBase()
    {

    }

    void GUICalculateWidgetBase::updateTableWidget()
    {

    }

    //-----------------------------------------------------------------------------------------------------
    
    GUICalculateSubWidgetBase::GUICalculateSubWidgetBase(QWidget* parent)
    {
        if (parent == nullptr)return;
        _calculateWdiget = dynamic_cast<GUICalculateWidgetBase*>(parent->parent());
    }

    GUICalculateSubWidgetBase::~GUICalculateSubWidgetBase()
    {

    }
}
