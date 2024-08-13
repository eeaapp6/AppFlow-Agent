#ifndef _OperatorsBoundary_H
#define _OperatorsBoundary_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsBoundary :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsBoundary();
        ~OperatorsBoundary();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionBoundaryEdit, OperatorsBoundary);
}

#endif