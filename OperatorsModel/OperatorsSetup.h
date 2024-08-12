#ifndef _OperatorsSetup_H
#define _OperatorsSetup_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsSetup :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsSetup();
        ~OperatorsSetup();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionSetupEdit, OperatorsSetup);
}

#endif