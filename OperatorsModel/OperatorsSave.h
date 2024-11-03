#ifndef _OperatorsSave_H
#define _OperatorsSave_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsSave :public Core::FITKActionOperator
    {
        Q_OBJECT;
    public:
        OperatorsSave();
        ~OperatorsSave();

        virtual bool execGUI();

        virtual bool execProfession();

    private slots:
        /**
         * @brief 读取线程结束执行该槽函数
         * @author LiBaojun (libaojunqd@foxmail.com)
         * @date 2024-03-26
         */
        void ioThreadFinishedSlot();
    };

    // 按钮注册相关操作
    Register2FITKOperatorRepo(actionSave, OperatorsSave);
}

#endif