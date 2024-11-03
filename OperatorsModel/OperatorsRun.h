#ifndef _OperatorsRun_H
#define _OperatorsRun_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsRun :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsRun();
        ~OperatorsRun();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOperatorRepo(actionRun, OperatorsRun);
}

#endif