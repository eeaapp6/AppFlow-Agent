#ifndef _OperatorsOpenManager_H
#define _OperatorsOpenManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsOpenManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsOpenManager();
        ~OperatorsOpenManager();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionOpen, OperatorsOpenManager);
}

#endif