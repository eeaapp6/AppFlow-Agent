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
        void slotSelectFaceGroup();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionGeoBoolFause, OperatorsGeoBool);
    Register2FITKOPeratorRepo(actionGeoBoolCut, OperatorsGeoBool);
    Register2FITKOPeratorRepo(actionGeoBoolCommon, OperatorsGeoBool);
    Register2FITKOPeratorRepo(actionGeoBoolOrImportEdit, OperatorsGeoBool);
    Register2FITKOPeratorRepo(actionGeoBoolOrImportDelete, OperatorsGeoBool);
}

#endif