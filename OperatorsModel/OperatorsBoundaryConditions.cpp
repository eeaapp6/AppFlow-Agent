#include "OperatorsBoundaryConditions.h"

#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/TreeEventOperator.h"
#include "GUIDialog/GUICalculateDialog/BoundaryConditionsWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace ModelOper
{
    OperatorsBoundaryConditions::OperatorsBoundaryConditions()
    {

    }

    OperatorsBoundaryConditions::~OperatorsBoundaryConditions()
    {

    }

    bool OperatorsBoundaryConditions::execGUI()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        switch (_operType){
        case ModelOper::OperManagerBase::Create: {

            break;
        }
        case ModelOper::OperManagerBase::Edit: {
            GUI::BoundaryConditionsWidget* widget = new GUI::BoundaryConditionsWidget(this, propertyWidget);
            propertyWidget->setWidget(widget);
            break;
        }
        }

        return true;
    }

    bool OperatorsBoundaryConditions::execProfession()
    {
        return true;
    }
}

