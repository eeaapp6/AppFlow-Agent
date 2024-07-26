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
#include "FITK_Interface/FITKInterfaceMesh/FITKUnstructuredFulidMeshVTK.h"
#include "OperatorsInterface/GraphEventOperator.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
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
        //Interface::FITKUnstructuredFluidMeshVTK* mesh = app->getGlobalData()->getMeshData<Interface::FITKUnstructuredFluidMeshVTK>();
        Interface::FITKUnstructuredFluidMeshVTK* mesh = new Interface::FITKUnstructuredFluidMeshVTK;
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


