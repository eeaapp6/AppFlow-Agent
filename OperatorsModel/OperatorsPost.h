#ifndef _OperatorsPost_H
#define _OperatorsPost_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsPost :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsPost();
        ~OperatorsPost();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOperatorRepo(actionPost, OperatorsPost);
}

#endif