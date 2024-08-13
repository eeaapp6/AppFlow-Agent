#ifndef _OperatorsTurbulence_H
#define _OperatorsTurbulence_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsTurbulence :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsTurbulence();
        ~OperatorsTurbulence();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionTurbulenceEdit, OperatorsTurbulence);
}

#endif