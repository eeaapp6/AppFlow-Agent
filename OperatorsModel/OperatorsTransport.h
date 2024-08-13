#ifndef _OperatorsTransport_H
#define _OperatorsTransport_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsTransport :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsTransport();
        ~OperatorsTransport();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionTransportEdit, OperatorsTransport);
}

#endif