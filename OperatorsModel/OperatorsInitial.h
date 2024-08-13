#ifndef _OperatorsInitial_H
#define _OperatorsInitial_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsInitial :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsInitial();
        ~OperatorsInitial();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionInitialEdit, OperatorsInitial);
}

#endif