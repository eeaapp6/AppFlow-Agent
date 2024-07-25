#ifndef _OperatorsLocalManager_H
#define _OperatorsLocalManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsLocalManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsLocalManager();
        ~OperatorsLocalManager();

        virtual bool execGUI();

        virtual bool execProfession();

        void moveToStep(int index, QVariant value = QVariant()) override;
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionLocalSelectGroup, OperatorsLocalManager);
    Register2FITKOPeratorRepo(actionLocalEdit, OperatorsLocalManager);
    Register2FITKOPeratorRepo(actionLocalDelete, OperatorsLocalManager);
}
#endif

