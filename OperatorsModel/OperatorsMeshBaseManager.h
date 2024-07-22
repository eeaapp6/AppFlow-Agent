#ifndef _OperatorsMeshBaseManager_H
#define _OperatorsMeshBaseManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsMeshBaseManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsMeshBaseManager();
        ~OperatorsMeshBaseManager();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionEditBase, OperatorsMeshBaseManager);
}

#endif