#ifndef _OperatorsSaveManager_H
#define _OperatorsSaveManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsSaveManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsSaveManager();
        ~OperatorsSaveManager();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionSave, OperatorsSaveManager);
}

#endif