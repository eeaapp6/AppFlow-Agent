#include "OperatorsMeshManager.h"

#include "FITK_Component/FITKOFDictWriter/FITKOFBlockMeshDictWriter.h"
#include "FITK_Kernel/FITKCore/FITKDirFileTools.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKAbstractCommandRunner.h"
#include "FITK_Kernel/FITKAppFramework/FITKAbstractProgramDriver.h"
#include "FITK_Kernel/FITKAppFramework/FITKProgramTaskManager.h"
#include "FITK_Component/FITKOFDriver/FITKOFInputInfo.h"
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
        QString path = QApplication::applicationDirPath() + "/../WorkDir";
        Core::CreateDir(path);
        IO::FITKOFBlockMeshDictWriter meshDickWri;
        meshDickWri.setFilePath(path);
        meshDickWri.run();
        auto app = dynamic_cast<AppFrame::FITKApplication*>(qApp);
        if (!app) return false;
        auto proGramManager = app->getProgramTaskManager();
        if (!proGramManager) return false;
        AppFrame::FITKProgramInputInfo* info = new FoamDriver::FITKOFInputInfo();
        QStringList args;
        args << "-case" << path;
        info->setArgs(args);
        proGramManager->startProgram(1, "FITKOFBlockMeshDriver", info);
        return true;
    }

    bool OperatorsMeshManager::execProfession()
    {
        return true;
    }

}


