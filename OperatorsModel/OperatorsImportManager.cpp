/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "OperatorsImportManager.h"

#include "OperatorsInterface/TreeEventOperator.h"
#include "OperatorsInterface/GraphEventOperator.h"
#include "GUIFrame/MainWindow.h"
#include "GUIFrame/PropertyWidget.h"
#include "GUIDialog/GUIGeometryDialog/GeometryWidgetBase.h"
#include "GUIDialog/GUIMeshDialog/MeshGeoWidget.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"
#include "FITK_Kernel/FITKAppFramework/FITKMessage.h"
#include "FITK_Kernel/FITKCore/FITKThreadPool.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoInterfaceFactory.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKAbsGeoModelImport.h"
#include "FITK_Interface/FITKInterfaceGeometry/FITKGeoCommandList.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKAbstractMeshSizeInfoGenerator.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKGeometryMeshSize.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKRegionMeshSizeGeom.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKAbstractMeshProcessor.h"
#include "FITK_Interface/FITKInterfaceMesh/FITKUnstructuredFluidMeshVTK.h"

#include <QFileDialog>
#include <QApplication>
#include <QVariant>

namespace ModelOper {
    OperatorsImportManager::OperatorsImportManager()
    {

    }

    OperatorsImportManager::~OperatorsImportManager()
    {

    }

    void OperatorsImportManager::publishAgentManifestImportFinished(
        const QString &manifestPath,
        bool success,
        const QString &message)
    {
        emit agentManifestImportFinished(manifestPath, success, message);
    }

    bool OperatorsImportManager::execGUI()
    {
        this->clearArgs();
        const QString senderName = _senderName.trimmed();
        if (senderName == QStringLiteral("actionImportMesh")) {
            AppFrame::FITKMessageError(tr("Normal mesh import is not implemented."));
            return false;
        }
        if (senderName != QStringLiteral("actionImportGeometry")
            && senderName != QStringLiteral("actionImportOpenFoamMesh")) {
            AppFrame::FITKMessageError(tr("Unknown import action: %1").arg(senderName));
            return false;
        }

        GUI::MainWindow* mainWindow = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;


        QString workDir = "";
        if (FITKAPP->getAppSettings()) {
            workDir = FITKAPP->getAppSettings()->getWorkingDir();
        }
        if (workDir.isEmpty()) workDir = QApplication::applicationDirPath();
        QString fileName;
        QFileDialog fileDialog;
        if (senderName == QStringLiteral("actionImportGeometry")) {
            fileName = fileDialog.getOpenFileName(_mainWindow, tr("Import Geometry"), workDir, tr("File(*.brep ; *.stp ; *.step ; *.igs)"));
            if (fileName.isEmpty())return false;
        }
        else if (senderName == QStringLiteral("actionImportOpenFoamMesh")) {
            fileName = fileDialog.getExistingDirectory(_mainWindow, tr("Import OpenFoam Mesh"), workDir);
            if (fileName.isEmpty())return false;
        }
        this->setArgs("FileName", fileName);
        this->setArgs("SenderName", senderName);

        return true;
    }

    bool OperatorsImportManager::execProfession()
    {
        QString fileName;
        QString senderName;
        const bool fileNameTypeValid = _operArgs.value(QStringLiteral("FileName")).type() == QVariant::String;
        const bool senderNameTypeValid = _operArgs.value(QStringLiteral("SenderName")).type() == QVariant::String;
        const bool fileNameRead = this->argValue<QString>(QStringLiteral("FileName"), fileName);
        const bool senderNameRead = this->argValue<QString>(QStringLiteral("SenderName"), senderName);
        this->clearArgs();

        fileName = fileName.trimmed();
        senderName = senderName.trimmed();
        if (!fileNameRead || !fileNameTypeValid || fileName.isEmpty()) {
            AppFrame::FITKMessageError(tr("Import FileName is missing, invalid, or empty."));
            return false;
        }
        if (!senderNameRead || !senderNameTypeValid || senderName.isEmpty()) {
            AppFrame::FITKMessageError(tr("Import SenderName is missing, invalid, or empty."));
            return false;
        }

        if (senderName == QStringLiteral("actionImportMesh")) {
            AppFrame::FITKMessageError(tr("Normal mesh import is not implemented."));
            return false;
        }
        if (senderName != QStringLiteral("actionImportGeometry")
            && senderName != QStringLiteral("actionImportOpenFoamMesh")) {
            AppFrame::FITKMessageError(tr("Unknown import action: %1").arg(senderName));
            return false;
        }
        if (senderName == QStringLiteral("actionImportOpenFoamMesh")
            && m_openFoamImportPending) {
            AppFrame::FITKMessageError(tr("OpenFOAM mesh import is already running."));
            return false;
        }

        Core::FITKThreadPool* pool = Core::FITKThreadPool::getInstance();
        if (pool == nullptr) {
            AppFrame::FITKMessageError(tr("Import thread pool is unavailable."));
            return false;
        }

        if (senderName == QStringLiteral("actionImportGeometry"))
        {
            ImportReadThread* importThread = new ImportReadThread();
            importThread->_type = ImportType::ImportGeo;
            importThread->_fileName = fileName;
            connect(importThread, SIGNAL(sigImportFinish(bool, int)), this, SLOT(slotGeoImportFinish(bool, int)));
            pool->execTask(importThread);
            return true;
        }

        if (senderName == QStringLiteral("actionImportOpenFoamMesh")) {
            ImportReadThread* importThread = new ImportReadThread();
            importThread->_type = ImportType::ImportOpenFoamMesh;
            importThread->_fileName = fileName;
            connect(importThread, SIGNAL(sigImportFinish(bool, int)), this, SLOT(slotFoamMeshImportFinish(bool, int)));
            m_openFoamImportPending = true;
            m_openFoamImportPath = fileName;
            pool->execTask(importThread);
            return true;
        }

        return false;
    }

    void OperatorsImportManager::slotGeoImportFinish(bool result, int objID)
    {
        if (result == false)return;
        if (objID < 0)return;

        Interface::FITKGeoCommandList* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
        if (geometryData == nullptr) return;

        //默认添加Default面组
        GUI::GeometryWidgetBase::createDefaultFaceGroup(geometryData->getDataByID(objID));

        //更新网格划分区域界面
        GUI::MainWindow* mainWin = dynamic_cast<GUI::MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        if (mainWin) {
            GUI::MeshGeoWidget* widget = dynamic_cast<GUI::MeshGeoWidget*>(mainWin->getPropertyWidget()->getCurrentWidget());
            if (widget)widget->updateWidget();
        }

        //获取模型树控制器
        auto treeOper = Core::FITKOperatorRepo::getInstance()->getOperatorT<EventOper::TreeEventOperator>("ModelTreeEvent");
        if (treeOper == nullptr) return;
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;

        graphOper->updateGraph(objID);
        treeOper->updateTree();
        graphOper->reRender(true);
    }

    void OperatorsImportManager::slotMeshImportFinish(bool result, int objID)
    {
        if (result == false)return;
        if (objID < 0)return;

        // 获取模型树控制器
        auto treeOper = Core::FITKOperatorRepo::getInstance()->getOperatorT<EventOper::TreeEventOperator>("ModelTreeEvent");
        if (treeOper == nullptr) return;
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;

        graphOper->updateGraph(objID);
        treeOper->updateTree();
        graphOper->reRender(true);
    }

    void OperatorsImportManager::slotFoamMeshImportFinish(bool result, int objID)
    {
        m_openFoamImportPending = false;
        const QString caseDir = m_openFoamImportPath;
        m_openFoamImportPath.clear();
        Q_UNUSED(objID);

        const auto finishImport = [this, &caseDir](bool success, const QString &message) {
            if (!success) {
                AppFrame::FITKMessageError(message);
            }
            emit openFoamMeshImportFinished(caseDir, success, message);
        };

        if (!result) {
            finishImport(false, tr("OpenFOAM mesh reader failed for: %1").arg(caseDir));
            return;
        }

        auto app = FITKAPP;
        if (app == nullptr) {
            finishImport(false, tr("OpenFOAM mesh import could not finish because FITKAPP is unavailable."));
            return;
        }
        if (app->getGlobalData() == nullptr) {
            finishImport(false, tr("OpenFOAM mesh import could not finish because GlobalData is unavailable."));
            return;
        }
        //刷新渲染窗口
        auto operatorRepo = Core::FITKOperatorRepo::getInstance();
        EventOper::GraphEventOperator* graphOper = operatorRepo == nullptr
            ? nullptr
            : operatorRepo->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr) {
            finishImport(false, tr("OpenFOAM mesh import could not finish because GraphPreprocess is unavailable."));
            return;
        }
        // 网格对象
        auto mesh = app->getGlobalData()->getMeshData<Interface::FITKUnstructuredFluidMeshVTK>();
        if (mesh == nullptr) {
            finishImport(false, tr("OpenFOAM mesh import could not finish because the target mesh is unavailable."));
            return;
        }
        graphOper->updateGraph(mesh->getDataObjectID(), true);

        // 获取模型树控制器
        auto treeOper = operatorRepo->getOperatorT<EventOper::TreeEventOperator>("ModelTreeEvent");
        if (treeOper) treeOper->updateTree();
        graphOper->reRender(true);
        finishImport(true, tr("OpenFOAM mesh import completed for: %1").arg(caseDir));
    }

    void ImportReadThread::run()
    {
        switch (_type) {
        case ModelOper::ImportType::ImportGeo: {
            Interface::FITKGeoCommandList* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKGeoCommandList>();
            if (geometryData == nullptr) return;

            Interface::FITKInterfaceGeometryFactory* geoFactory = Interface::FITKInterfaceGeometryFactory::getInstance();
            if (geoFactory == nullptr)return;
            auto geoObj = geoFactory->createCommandT<Interface::FITKAbsGeoModelImport>(Interface::FITKGeoEnum::FITKGeometryComType::FGTImport);
            if (geoObj == nullptr)return;
            geoObj->setFileName(_fileName);


            if (geoObj->getDataObjectName().isEmpty()) {
                QFileInfo fileInfo(_fileName);
                // 获取文件名称（不包含路径与文件类型）
                QString name = fileInfo.baseName();
                geoObj->setDataObjectName(name);
            }
            geometryData->appendDataObj(geoObj);
            bool result = geoObj->update();

            emit sigImportFinish(result, geoObj->getDataObjectID());
            break;
        }
        case ModelOper::ImportType::ImportMesh: {
            emit sigImportFinish(false, -1);
            break;
        }
        case ModelOper::ImportType::ImportOpenFoamMesh:{
            // 获取单例
            auto meshGen = Interface::FITKMeshGenInterface::getInstance();
            bool result = false;
            // 读取网格
            if (meshGen != nullptr) {
                auto meshProcessor = meshGen->getMeshProcessor();
                if (meshProcessor != nullptr) {
                    result = meshProcessor->start(QStringList() << _fileName);
                }
            }
            emit sigImportFinish(result, -1);
            break;
        }
        }
    }

}

