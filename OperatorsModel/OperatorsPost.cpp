#include "OperatorsPost.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "GUIDialog/GUICalculateDialog/PostWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace ModelOper 
{    
    OperatorsPost::OperatorsPost()
    {

    }

    OperatorsPost::~OperatorsPost()
    {

    }

    bool OperatorsPost::execGUI()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        GUI::PostWidget* widget = new GUI::PostWidget(this, FITKAPP->getGlobalData()->getMainWindow());
        propertyWidget->setWidget(widget);

        return false;
    }

    bool OperatorsPost::execProfession()
    {
        return false;
    }
}

