#include "OperatorsThermo.h"

#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/TreeEventOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "GUIDialog/GUICalculateDialog/ThermoWidget.h"

namespace ModelOper
{
    OperatorsThermo::OperatorsThermo()
    {

    }

    OperatorsThermo::~OperatorsThermo()
    {

    }

    bool OperatorsThermo::execGUI()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        GUI::ThermoWidget* widget = new GUI::ThermoWidget(this, propertyWidget);
        propertyWidget->setWidget(widget);

        return true;
    }

    bool OperatorsThermo::execProfession()
    {
        return true;
    }
}

