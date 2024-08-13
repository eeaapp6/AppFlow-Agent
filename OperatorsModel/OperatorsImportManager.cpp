#include "OperatorsImportManager.h"

#include "OperatorsInterface/TreeEventOperator.h"
#include "OperatorsInterface/GraphEventOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"
#include "FITK_Kernel/FITKCore/FITKThreadPool.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoInterfaceFactory.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoModelImport.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoCommandList.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKAbstractMeshSizeInfoGenerator.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKGeometryMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeGeom.h"

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
        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        //获取线程池
        Core::FITKThreadPool* pool = Core::FITKThreadPool::getInstance();
        if (pool == nullptr)return false;

        QString workDir = "";
        if (FITKAPP->getAppSettings()) {
            workDir = FITKAPP->getAppSettings()->getWorkingDir();
        }
        if (workDir.isEmpty()) workDir = QApplication::applicationDirPath();

        QFileDialog fileDialog;
        if (_senderName == "actionImportGeometry") {
            QString fileName = fileDialog.getOpenFileName(_mainWindow, tr("Import Geometry"), workDir, tr("File(*.stp ; *.step ; *.igs ; *.stl)"));
            if (fileName.isEmpty())return false;

            ImportReadThread* importThread = new ImportReadThread();
            importThread->_type = ImportType::ImportGeo;
            importThread->_fileName = fileName;
            pool->execTask(importThread);
            connect(importThread, SIGNAL(sigImportFinish(bool, int)), this, SLOT(slotGeoImportFinish(bool, int)));
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

    void OperatorsImportManager::slotGeoImportFinish(bool result, int objID)
    {
        if (result == false)return;
        if (objID < 0)return;

        auto meshSizeGen = Interface::FITKMeshGenInterface::getInstance()->getMeshSizeGenerator();
        auto meshSizeManager = Interface::FITKMeshGenInterface::getInstance()->getRegionMeshSizeMgr();
        if (meshSizeGen && meshSizeManager) {
            auto meshSizeGeo = dynamic_cast<Interface::FITKRegionMeshSizeGeom*>
                (meshSizeGen->createRegionMeshSize(Interface::FITKAbstractRegionMeshSize::RegionType::RigonGeom));
            if (meshSizeGeo) {
                meshSizeGeo->setGeomID(objID);
                meshSizeManager->appendDataObj(meshSizeGeo);
            }
        }

        // 获取模型树控制器
        auto treeOper = Core::FITKOperatorRepo::getInstance()->getOperatorT<EventOper::TreeEventOperator>("ModelTreeEvent");
        if (treeOper == nullptr) return;
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;

        graphOper->updateGraph(objID);
        treeOper->updateTree();
        graphOper->reRender(true);
    }

    void ImportReadThread::run()
    {
        switch (_type){
        case ModelOper::ImportType::ImportGeo: {
            Interface::FITKGeoCommandList* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
            if (geometryData == nullptr) return;

            Interface::FITKInterfaceGeometryFactory* geoFactory = Interface::FITKInterfaceGeometryFactory::getInstance();
            if (geoFactory == nullptr)return;
            auto geoObj = geoFactory->createCommandT<Interface::FITKAbsGeoModelImport>(Interface::FITKGeoEnum::FITKGeometryComType::FGTImport);
            if (geoObj == nullptr)return;
            geoObj->setFileName(_fileName);
            bool result = geoObj->update();

            if (geoObj->getDataObjectName().isEmpty()) {
                QFileInfo fileInfo(_fileName);
                // 获取文件名称（不包含路径与文件类型）
                QString name = fileInfo.baseName();
                geoObj->setDataObjectName(name);
            }
            geometryData->appendDataObj(geoObj);

            emit sigImportFinish(result, geoObj->getDataObjectID());
            break;
        }
        case ModelOper::ImportType::ImportMesh:
            break;
        }
    }

}

