#ifndef _OperatorsSave_H
#define _OperatorsSave_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsSave :public Core::FITKActionOperator
    {
        Q_OBJECT;
    public:
        OperatorsSave();
        ~OperatorsSave();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionSave, OperatorsSave);
}

#endif