/**********************************************************************
 * @file   OperatorsOperCondition.h
 * @brief  求解器运行条件操作器
 * @author BaGuijun (baguijun@163.com)
 * @date   2024-09-05
 *********************************************************************/
#ifndef _OperatorsOperCondition_H
#define _OperatorsOperCondition_H

#include "OperManagerBase.h"

namespace ModelOper
{
    /**
     * @brief  求解器运行条件操作器
     * @author BaGuijun (baguijun@163.com)
     * @date   2024-09-05
     */
    class OperatorsOperCondition :public OperManagerBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief    构造函数
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-05
         */
        OperatorsOperCondition();
        /**
         * @brief    析构函数
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-05
         */
        ~OperatorsOperCondition();
        /**
         * @brief    操作器执行
         * @return   bool    是否执行成功
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-05
         */
        virtual bool execGUI();
        /**
         * @brief    操作器执行结果
         * @return   bool   是否执行成功
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-05
         */
        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOperatorRepo(actionOperConditionEdit, OperatorsOperCondition);
}

#endif