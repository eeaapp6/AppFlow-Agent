#ifndef _OperatorsMeshManager_H
#define _OperatorsMeshManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsMeshManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsMeshManager();
        ~OperatorsMeshManager();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionMesh, OperatorsMeshManager);
}

#endif