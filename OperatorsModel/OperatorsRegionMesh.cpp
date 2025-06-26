#include "OperatorsRegionMesh.h"

#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/TreeEventOperator.h"
#include "GUIDialog/GUICalculateDialog/RegionMeshWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"

namespace ModelOper
{
    OperatorsRegionMesh::OperatorsRegionMesh()
    {

    }

    OperatorsRegionMesh::~OperatorsRegionMesh()
    {

    }

    bool OperatorsRegionMesh::execGUI()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        GUI::RegionMeshWidget* widget = new GUI::RegionMeshWidget(this);
        propertyWidget->setWidget(widget);

        return true;
    }

    bool OperatorsRegionMesh::execProfession()
    {
        auto treeOper = Core::FITKOperatorRepo::getInstance()->getOperatorT<EventOper::TreeEventOperator>("ModelTreeEvent");
        if (treeOper == nullptr) return false;
        treeOper->updateTree();
        return true;
    }
}

