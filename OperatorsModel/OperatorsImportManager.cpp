#include "OperatorsImportManager.h"

#include "OperatorsInterface/TreeEventOperator.h"
#include "OperatorsInterface/GraphEventOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoInterfaceFactory.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoModelImport.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoCommandList.h"

#include <QFileDialog>
#include <QApplication>

namespace ModelOper {
    OperatorsImportManager::OperatorsImportManager()
    {

    }

    OperatorsImportManager::~OperatorsImportManager()
    {
    }

    bool OperatorsImportManager::execGUI()
    {
        // 获取模型树控制器
        auto treeOper = Core::FITKOperatorRepo::getInstance()->getOperatorT<EventOper::TreeEventOperator>("ModelTreeEvent");
        if (treeOper == nullptr) return false;
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return false;
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        QString workDir = QApplication::applicationDirPath();
        QFileDialog fileDialog;
        if (_senderName == "actionImportGeometry") {
            Interface::FITKGeoCommandList* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
            if (geometryData == nullptr) return false;

            QString fileName = fileDialog.getOpenFileName(_mainWindow, tr("Import Geometry"), workDir, tr("File(*.stp ; *.igs ; *.stl)"));
            if (fileName.isEmpty())return false;

            Interface::FITKInterfaceGeometryFactory* geoFactory = Interface::FITKInterfaceGeometryFactory::getInstance();
            if (geoFactory == nullptr)return false;
            auto GeoImport = geoFactory->createCommandT<Interface::FITKAbsGeoModelImport>(Interface::FITKGeoEnum::FITKGeometryComType::FGTImport);
            if (GeoImport == nullptr)return false;
            GeoImport->setFileName(fileName);
            GeoImport->update();
            geometryData->appendDataObj(GeoImport);

            graphOper->updateGraph(GeoImport->getDataObjectID(), true);
            treeOper->updateTree();
            graphOper->reRender(true);
        }
        else  if (_senderName == "actionImportMesh") {
            fileDialog.getOpenFileName(_mainWindow, tr("Import Mesh"), workDir);
        }
        return true;
    }

    bool OperatorsImportManager::execProfession()
    {
        return true;
    }

}

