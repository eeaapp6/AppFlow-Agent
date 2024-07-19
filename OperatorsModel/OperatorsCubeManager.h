#ifndef _OperatorsCubeManager_H
#define _OperatorsCubeManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsCubeManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsCubeManager();
        ~OperatorsCubeManager();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionCreateCube, OperatorsCubeManager);
    Register2FITKOPeratorRepo(actionEditCube, OperatorsCubeManager);
    Register2FITKOPeratorRepo(actionDeleteCube, OperatorsCubeManager);
    Register2FITKOPeratorRepo(actionRenameCube, OperatorsCubeManager);
}

#endif