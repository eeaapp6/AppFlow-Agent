#ifndef _OperatorsGeoCylinderManager_H
#define _OperatorsGeoCylinderManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsGeoCylinderManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsGeoCylinderManager();
        ~OperatorsGeoCylinderManager();

        virtual bool execGUI();

        virtual bool execProfession();
        /**
         * @brief 其他事件执行
         * @param[i]  index          事件索引（自定义）
         * @param[i]  value          其他数据
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-01
         */
        void eventProcess(int index, QVariant value) override;

    private slots:
        ;
        void slotReselectOriginPoint();

        void slotSelectFaceGroup();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionGeoCylinderCreate, OperatorsGeoCylinderManager);
    Register2FITKOPeratorRepo(actionGeoCylinderEdit, OperatorsGeoCylinderManager);
    Register2FITKOPeratorRepo(actionGeoCylinderDelete, OperatorsGeoCylinderManager);
    Register2FITKOPeratorRepo(actionGeoCylinderRename, OperatorsGeoCylinderManager);
}

#endif