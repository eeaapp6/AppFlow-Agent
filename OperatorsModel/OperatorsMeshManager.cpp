#include "OperatorsMeshManager.h"

#include "FITK_Component/FITKOFDictWriter/FITKOFBlockMeshDictWriter.h"
#include "FITK_Kernel/FITKCore/FITKDirFileTools.h"

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

        return true;
    }

    bool OperatorsMeshManager::execProfession()
    {
        return true;
    }

}


