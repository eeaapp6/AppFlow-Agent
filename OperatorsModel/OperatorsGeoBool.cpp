#include "OperatorsGeoBool.h"

#include "GUIFrame/PropertyWidget.h"
#include "OperatorsInterface/GraphEventOperator.h"
#include "OperatorsInterface/TreeEventOperator.h"
#include "GUIDialog/GUIGeometryDialog/GeometryBoolWidget.h"
#include "GUIDialog/GUIGeometryDialog/GeometryDeleteDialog.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoCommandList.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoCommand.h"

namespace ModelOper {
    OperatorsGeoBool::OperatorsGeoBool()
    {

    }

    OperatorsGeoBool::~OperatorsGeoBool()
    {

    }

    bool OperatorsGeoBool::execGUI()
    {
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;
        Interface::FITKGeoCommandList* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
        if (geometryData == nullptr) return false;

        QWidget* widget = nullptr;
        QDialog* dialog = nullptr;
        int objID = -1;
        this->argValue("objID", objID);

        GUI::BoolType type = GUI::BoolType::GeoBoolNone;
        if (_senderName == "actionGeoBoolFause") {
            type = GUI::BoolType::GeoBoolFause;
            widget = new GUI::GeometryBoolWidget(type, this);
        }
        else if (_senderName == "actionGeoBoolCut") {
            type = GUI::BoolType::GeoBoolCut;
            widget = new GUI::GeometryBoolWidget(type, this);
        }
        else if (_senderName == "actionGeoBoolCommon") {
            type = GUI::BoolType::GeoBoolCommon;
            widget = new GUI::GeometryBoolWidget(type, this);
        }
        else if (_senderName == "actionGeoBoolDelete") {
            dialog = new GUI::GeometryDeleteDialog(dynamic_cast<Interface::FITKAbsGeoCommand*>(geometryData->getDataByID(objID)), this);
        }

        if (widget) {
            propertyWidget->setWidget(widget);
        }
        if (dialog) {
            dialog->show();
        }
        return false;
    }

    bool OperatorsGeoBool::execProfession()
    {
        // 获取模型树控制器
        auto treeOper = Core::FITKOperatorRepo::getInstance()->getOperatorT<EventOper::TreeEventOperator>("ModelTreeEvent");
        if (treeOper == nullptr) return false;
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return false;

        _mainWindow->getPropertyWidget()->init();

        int objID = -1;
        this->argValue("objID", objID);

        graphOper->updateGraph(objID);
        treeOper->updateTree();
        graphOper->reRender(true);

        return true;
    }
}

