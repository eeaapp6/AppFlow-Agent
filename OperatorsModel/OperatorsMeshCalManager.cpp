#include "OperatorsMeshCalManager.h"

#include "FITK_Component/FITKOFDictWriter/FITKOFBlockMeshDictWriter.h"
#include "FITK_Kernel/FITKCore/FITKDirFileTools.h"

#include <QApplication>

namespace ModelOper
{
    OperatorsMeshCalManager::OperatorsMeshCalManager()
    {
    }

    OperatorsMeshCalManager::~OperatorsMeshCalManager()
    {
    }

    bool OperatorsMeshCalManager::execGUI()
    {
        QString path = QApplication::applicationDirPath() + "/../WorkDir";
        Core::CreateDir(path);
        IO::FITKOFBlockMeshDictWriter meshDickWri;
        meshDickWri.setFilePath(path);
        meshDickWri.run();

        return true;
    }

    bool OperatorsMeshCalManager::execProfession()
    {
        return true;
    }

}


