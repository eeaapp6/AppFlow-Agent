#ifndef _OperatorsMeshCalManager_H
#define _OperatorsMeshCalManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsMeshCalManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsMeshCalManager();
        ~OperatorsMeshCalManager();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionMeshCalc, OperatorsMeshCalManager);
}

#endif