#include "OperatorsCubeManager.h"

#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "GUIDialog/GUIGeometryDialog/CudeInfoWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace ModelOper
{
    OperatorsCubeManager::OperatorsCubeManager()
    {

    }

    OperatorsCubeManager::~OperatorsCubeManager()
    {

    }

    bool OperatorsCubeManager::execGUI()
    {
        QWidget* widget = nullptr;

        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        switch (_operType){
        case ModelOper::OperManagerBase::None:
            break;
        case ModelOper::OperManagerBase::Manage:
            break;
        case ModelOper::OperManagerBase::Create:
            widget = new GUI::CudeInfoWidget(this);
            break;
        case ModelOper::OperManagerBase::Edit:
            break;
        case ModelOper::OperManagerBase::Copy:
            break;
        case ModelOper::OperManagerBase::Delete:
            break;
        case ModelOper::OperManagerBase::Rename:
            break;
        case ModelOper::OperManagerBase::Show:
            break;
        case ModelOper::OperManagerBase::Hide:
            break;
        default:
            break;
        }

        if (mainWindow->getPropertyWidget()) {
            propertyWidget->setWidget(widget);
        }

        return false;
    }
    bool OperatorsCubeManager::execProfession()
    {
        return false;
    }
}
