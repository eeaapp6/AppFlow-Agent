#include "OperatorsMeshManager.h"

#include "FITK_Component/FITKOFDictWriter/FITKOFBlockMeshDictWriter.h"
#include "FITK_Kernel/FITKCore/FITKDirFileTools.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKAbstractCommandRunner.h"
#include "FITK_Kernel/FITKAppFramework/FITKAbstractProgramDriver.h"
#include "FITK_Kernel/FITKAppFramework/FITKProgramTaskManager.h"
#include "FITK_Component/FITKOFDriver/FITKOFInputInfo.h"
#include "FITK_Component/FITKOFMeshIO/FITKOpenFOAMMeshReader.h"
#include "FITK_Interface/FITKInterfaceMesh/FITKUnstructuredFulidMeshVTK.h"
#include "OperatorsInterface/GraphEventOperator.h"
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
#ifndef Q_OS_WIN
        Core::CreateDir(path);
        // 写出字典文件
        IO::FITKOFBlockMeshDictWriter meshDickWri;
        meshDickWri.setFilePath(path);
        if (!meshDickWri.run()) return false;
        // 调用blockMesh
        auto app = dynamic_cast<AppFrame::FITKApplication*>(qApp);
        if (!app) return false;
        auto proGramManager = app->getProgramTaskManager();
        if (!proGramManager) return false;
        AppFrame::FITKProgramInputInfo* info = new FoamDriver::FITKOFInputInfo();
        QStringList args;
        args << "-case" << path;
        info->setArgs(args);
        proGramManager->startProgram(1, "FITKOFBlockMeshDriver", info);
        // 调用snappyHexMeshDriver
        //proGramManager->startProgram(1, "FITKOFSnappyHexMeshDriver", info);
#endif
        // 读取网格
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


