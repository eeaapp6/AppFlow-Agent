#include "GUICalculateWidgetBase.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFCasePhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowSolverProcessFactory.h"

namespace GUI
{
    GUICalculateWidgetBase::GUICalculateWidgetBase(EventOper::ParaWidgetInterfaceOperator * oper, QWidget * parent) :
        GUIWidgetBase(parent), _oper(oper)
    {
        _solverData = FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFCasePhysicsData>();
        _factor = dynamic_cast<Interface::FITKFlowSolverProcessFactory*>(FITKAPP->getComponents()->getComponentByName("FITKFlowSolverProcess"));
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
