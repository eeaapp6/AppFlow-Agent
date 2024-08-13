#ifndef _OperatorsDiscretization_H
#define _OperatorsDiscretization_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsDiscretization :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsDiscretization();
        ~OperatorsDiscretization();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionDiscretizationEdit, OperatorsDiscretization);
}

#endif