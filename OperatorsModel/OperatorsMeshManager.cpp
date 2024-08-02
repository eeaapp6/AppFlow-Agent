#include "OperatorsMeshManager.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKMeshGenInterface.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKAbstractMesherDriver.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKAbstractMeshProcessor.h"
#include "FITK_Interface/FITKInterfaceMeshGen/FITKGeometryMeshSize.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Interface/FITKInterfaceMesh/FITKUnstructuredFluidMeshVTK.h"

#include "OperatorsInterface/GraphEventOperator.h"
namespace ModelOper
{
    bool OperatorsMeshManager::execGUI()
    {
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return false;

        if (_emitter == nullptr)return false;
        QString actionName = _emitter->objectName();
        if (actionName == "actionMesh") {
            // 获取单例
            auto meshGen = Interface::FITKMeshGenInterface::getInstance();
            auto manager = meshGen->getGeometryMeshSizeManager();
            // 网格划分
            auto meshDriver = meshGen->getMesherDriver();
            if (meshDriver == nullptr) return false;
            meshDriver->setValue("WorkDir", QApplication::applicationDirPath() + "/../WorkDir");
            meshDriver->setValue("HasGeoMeshSize", manager->getDataCount() > 0);
            meshDriver->startMesher();
            connect(meshDriver, &Interface::FITKAbstractMesherDriver::mesherFinished, [this] {
                readMesh();
            });
        }
        else if (actionName == "actionClearMesh") {
            auto globalData = FITKAPP->getGlobalData();
            if (globalData == nullptr)return false;
            Interface::FITKUnstructuredFluidMeshVTK* meshData = globalData->getMeshData< Interface::FITKUnstructuredFluidMeshVTK>();
            if (meshData == nullptr)return false;
            meshData->clearMesh();
            graphOper->reRender();
        }
        return true;
    }

    bool OperatorsMeshManager::execProfession()
    {

        return true;
    }
    void OperatorsMeshManager::readMesh()
    {
        // 获取单例
        auto meshGen = Interface::FITKMeshGenInterface::getInstance();
        // 读取网格
        auto meshProcessor = meshGen->getMeshProcessor();
        if (meshProcessor == nullptr) return;
        meshProcessor->setValue("WorkDir", QApplication::applicationDirPath() + "/../WorkDir");
        meshProcessor->start();
        //刷新渲染窗口
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return;
        // 网格对象
        auto mesh = FITKAPP->getGlobalData()->getMeshData<Interface::FITKUnstructuredFluidMeshVTK>();
        graphOper->updateGraph(mesh->getDataObjectID());
    }
}