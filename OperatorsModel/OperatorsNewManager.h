#ifndef _OperatorsNewManager_H
#define _OperatorsNewManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsNewManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsNewManager();
        ~OperatorsNewManager();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionNew, OperatorsNewManager);
}

#endif