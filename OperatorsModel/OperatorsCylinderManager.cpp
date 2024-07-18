#include "OperatorsCylinderManager.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/GraphEventOperator.h"
#include "GUIDialog/GUIGeometryDialog/CylinderInfoWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace ModelOper
{
    OperatorsCylinderManager::OperatorsCylinderManager()
    {

    }

    OperatorsCylinderManager::~OperatorsCylinderManager()
    {

    }

    bool OperatorsCylinderManager::execGUI()
    {
        QWidget* widget = nullptr;

        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        switch (_operType) {
        case ModelOper::OperManagerBase::Create:
            widget = new GUI::CylinderInfoWidget(this);
            break;
        case ModelOper::OperManagerBase::Edit:
            break;
        case ModelOper::OperManagerBase::Copy:
            break;
        case ModelOper::OperManagerBase::Delete:
            break;
        case ModelOper::OperManagerBase::Rename:
            break;
        }

        if (mainWindow->getPropertyWidget()) {
            propertyWidget->setWidget(widget);
        }

        return false;
    }

    bool OperatorsCylinderManager::execProfession()
    {
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return false;
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        int objID = -1;
        this->argValue("objID", objID);

        switch (_operType) {
        case ModelOper::OperManagerBase::Create:
            graphOper->updateGraph(objID);
            break;
        case ModelOper::OperManagerBase::Edit:
            graphOper->updateGraph(objID);
            break;
        case ModelOper::OperManagerBase::Copy:
            break;
        case ModelOper::OperManagerBase::Delete:
            break;
        case ModelOper::OperManagerBase::Rename:
            break;
        }

        propertyWidget->init();

        return true;
    }
}
