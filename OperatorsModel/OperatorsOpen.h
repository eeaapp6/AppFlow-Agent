#ifndef _OperatorsOpen_H
#define _OperatorsOpen_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsOpen :public Core::FITKActionOperator
    {
        Q_OBJECT;
    public:
        OperatorsOpen();
        ~OperatorsOpen();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionOpen, OperatorsOpen);
}

#endif