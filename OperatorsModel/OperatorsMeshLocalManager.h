#ifndef _OperatorsMeshLocalManager_H
#define _OperatorsMeshLocalManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsMeshLocalManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsMeshLocalManager();
        ~OperatorsMeshLocalManager();

        virtual bool execGUI();

        virtual bool execProfession();
        /**
         * @brief 其他事件执行
         * @param[i]  index          事件索引（自定义）
         * @param[i]  value          其他数据
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-01
         */
        void eventProcess(int index, QVariant value = QVariant()) override;
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionMeshLocalSelectGroup, OperatorsMeshLocalManager);
    Register2FITKOPeratorRepo(actionMeshLocalEdit, OperatorsMeshLocalManager);
    Register2FITKOPeratorRepo(actionMeshLocalDelete, OperatorsMeshLocalManager);
}
#endif

