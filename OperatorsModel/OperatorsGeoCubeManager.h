#ifndef _OperatorsGeoCubeManager_H
#define _OperatorsGeoCubeManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsGeoCubeManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsGeoCubeManager();
        ~OperatorsGeoCubeManager();

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
    private slots:
        ;
        void slotReselectBasePoint();

        void slotSelectFaceGroup();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionGeoCubeCreate, OperatorsGeoCubeManager);
    Register2FITKOPeratorRepo(actionGeoCubeEdit, OperatorsGeoCubeManager);
    Register2FITKOPeratorRepo(actionGeoCubeDelete, OperatorsGeoCubeManager);
    Register2FITKOPeratorRepo(actionGeoCubeRename, OperatorsGeoCubeManager);
}

#endif