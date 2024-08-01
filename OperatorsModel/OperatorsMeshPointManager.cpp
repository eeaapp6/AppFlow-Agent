#include "OperatorsMeshPointManager.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "GUIWidget/GUIPickInfo.h"
#include "GUIWidget/PickedDataProvider.h"
#include "GUIWidget/PickedData.h"
#include "OperatorsInterface/GraphEventOperator.h"
#include "OperatorsInterface/GraphInteractionOperator.h"
#include "GUIDialog/GUIMeshDialog/MaterialPointWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
namespace ModelOper
{
    OperatorsMeshPointManager::OperatorsMeshPointManager()
    {

    }

    OperatorsMeshPointManager::~OperatorsMeshPointManager()
    {

    }

    bool OperatorsMeshPointManager::execGUI()
    {
        QWidget* widget = nullptr;
        QDialog* dialog = nullptr;

        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        switch (_operType)
        {
        case ModelOper::OperManagerBase::Edit:
            widget = new GUI::MaterialPointWidget(this);
            break;
        }

        if (mainWindow->getPropertyWidget() && widget) {
            propertyWidget->setWidget(widget);
        }

        if (dialog) {
            dialog->show();
        }

        return false;
    }

    bool OperatorsMeshPointManager::execProfession()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        switch (_operType)
        {
        case ModelOper::OperManagerBase::Edit:
            break;
        }

        return true;
    }

    void OperatorsMeshPointManager::eventProcess(int index, QVariant value)
    {

    }
}

