#include "OperatorsRun.h"

#include "GUIFrame/PropertyWidget.h"
#include "GUIDialog/GUICalculateDialog/RunWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace ModelOper
{
    OperatorsRun::OperatorsRun()
    {

    }

    OperatorsRun::~OperatorsRun()
    {

    }

    bool OperatorsRun::execGUI()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        GUI::RunWidget* widget = new GUI::RunWidget(this, FITKAPP->getGlobalData()->getMainWindow());
        propertyWidget->setWidget(widget);

        return true;
    }

    bool OperatorsRun::execProfession()
    {
        return false;
    }
}
