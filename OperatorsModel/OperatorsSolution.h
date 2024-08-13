#ifndef _OperatorsSolution_H
#define _OperatorsSolution_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsSolution :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsSolution();
        ~OperatorsSolution();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionSolutionEdit, OperatorsSolution);
}

#endif