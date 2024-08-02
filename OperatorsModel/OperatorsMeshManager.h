#ifndef _OperatorsMeshManager_H
#define _OperatorsMeshManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsMeshManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsMeshManager() = default;
        ~OperatorsMeshManager() = default;

        virtual bool execGUI();

        virtual bool execProfession();

    protected:
        void readMesh();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionClearMesh, OperatorsMeshManager);
    Register2FITKOPeratorRepo(actionMesh, OperatorsMeshManager);
}

#endif