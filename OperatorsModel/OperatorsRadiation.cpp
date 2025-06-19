#include "OperatorsRadiation.h"

#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/TreeEventOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "GUIDialog/GUICalculateDialog/RadiationWidget.h"

namespace ModelOper
{
    OperatorsRadiation::OperatorsRadiation()
    {

    }

    OperatorsRadiation::~OperatorsRadiation()
    {

    }

    bool OperatorsRadiation::execGUI()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        GUI::RadiationWidget* widget = new GUI::RadiationWidget(this, propertyWidget);
        propertyWidget->setWidget(widget);

        return true;
    }

    bool OperatorsRadiation::execProfession()
    {
        return true;
    }
}

