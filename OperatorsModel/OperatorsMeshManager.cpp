#include "OperatorsMeshManager.h"

#include "FITK_Component/FITKOFDictWriter/FITKOFBlockMeshDictWriter.h"
#include "FITK_Component/FITKOFDictWriter/FITKOFSnappyHexMeshDictWriter.h"
#include "FITK_Kernel/FITKCore/FITKDirFileTools.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKAbstractCommandRunner.h"
#include "FITK_Kernel/FITKAppFramework/FITKAbstractProgramDriver.h"
#include "FITK_Kernel/FITKAppFramework/FITKProgramTaskManager.h"
#include "FITK_Component/FITKOFDriver/FITKOFInputInfo.h"
#include "FITK_Component/FITKOFMeshIO/FITKOpenFOAMMeshReader.h"
#include "FITK_Interface/FITKInterfaceMesh/FITKUnstructuredFluidMeshVTK.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFGeometryData.h"
#include "OperatorsInterface/GraphEventOperator.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
//#include "FITK_Interface/FITKInterfaceModel/FITKAbstractGeoModel.h"
#include "FITK_Component/FITKGeoCompOCC/FITKAbstractOCCModel.h"
#include <QStringList>

#include <QApplication>

namespace ModelOper
{
    OperatorsMeshManager::OperatorsMeshManager()
    {
    }

    OperatorsMeshManager::~OperatorsMeshManager()
    {
    }

    bool OperatorsMeshManager::execGUI()
    {
        // 工作路径
        QString path = QApplication::applicationDirPath() + "/../WorkDir";
        Core::CreateDir(path);
        // 写出字典文件
        IO::FITKOFBlockMeshDictWriter blockMeshDictWriter;
        blockMeshDictWriter.setFilePath(path);
        if (!blockMeshDictWriter.run()) return false;

        // 写出STL文件
        //@{
        QString stlFolder = path + "/constant/geometry";
        Core::CreateDir(stlFolder);

        Interface::FITKOFGeometryData* geometryData = FITKAPP->getGlobalData()->getGeometryData<Interface::FITKOFGeometryData>();
        int nModel = geometryData->getDataCount();

        for (int i = 0; i < nModel; i++)
        {
            // Interface::FITKAbstractGeoModel* model = geometryData->getDataByIndexT<Interface::FITKAbstractGeoModel>(i);
            OCC::FITKAbstractOCCModel* model = geometryData->getDataByIndexT<OCC::FITKAbstractOCCModel>(i);
            if (!model)
            {
                continue;
            }

            model->writeSTLFile(stlFolder);
        }
        //@}

        /*IO::FITKOFSnappyHexMeshDictWriter snappyHexMeshDictWriter;
        snappyHexMeshDictWriter.setFilePath(path);
        if (!snappyHexMeshDictWriter.run()) return false;*/


        // 调用blockMesh
        auto app = dynamic_cast<AppFrame::FITKApplication*>(qApp);
        if (!app) return false;
#ifndef Q_OS_WIN
        auto proGramManager = app->getProgramTaskManager();
        if (!proGramManager) return false;
        AppFrame::FITKProgramInputInfo* info = new FoamDriver::FITKOFInputInfo();
        QStringList args;
        args << "-case" << path;
        info->setArgs(args);
        proGramManager->startProgram(1, "FITKOFBlockMeshDriver", info);
        // 调用snappyHexMeshDriver
        /*QStringList args1;
        args1 << "-overwrite -case" << path;
        info->setArgs(args1);
        proGramManager->startProgram(1, "FITKOFSnappyHexMeshDriver", info);*/
#endif
        // 读取网格
        Interface::FITKUnstructuredFluidMeshVTK* mesh = app->getGlobalData()->getMeshData<Interface::FITKUnstructuredFluidMeshVTK>();
        mesh->clearMesh();
        IO::FITKOpenFOAMMeshReader openFOAMMeshReader;
        openFOAMMeshReader.setFileName(path + "/constant/polyMesh/");
        openFOAMMeshReader.setDataObject(mesh);
        openFOAMMeshReader.run();
        // 渲染网格
        //刷新渲染窗口
        EventOper::GraphEventOperator* graphOper = FITKOPERREPO->getOperatorT<EventOper::GraphEventOperator>("GraphPreprocess");
        if (graphOper == nullptr)return false;
        graphOper->updateGraph(mesh->getDataObjectID());

        return true;
    }

    bool OperatorsMeshManager::execProfession()
    {
        return true;
    }

}


