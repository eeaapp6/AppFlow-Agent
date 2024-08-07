#ifndef _OperatorsGeoBool_H
#define _OperatorsGeoBool_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsGeoBool :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsGeoBool();
        ~OperatorsGeoBool();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionGeoBoolFause, OperatorsGeoBool);
    Register2FITKOPeratorRepo(actionGeoBoolCut, OperatorsGeoBool);
    Register2FITKOPeratorRepo(actionGeoBoolCommon, OperatorsGeoBool);
    Register2FITKOPeratorRepo(actionGeoBoolDelete, OperatorsGeoBool);
}

#endif