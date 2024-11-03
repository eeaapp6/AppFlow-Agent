#ifndef _OperatorsNew_H
#define _OperatorsNew_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsNew :public Core::FITKActionOperator
    {
        Q_OBJECT;
    public:
        OperatorsNew();
        ~OperatorsNew();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOperatorRepo(actionNew, OperatorsNew);
}

#endif