/**
 * 
 * @file OperatorsSetup.h
 * @brief 求解器设置操作器
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _OperatorsSetup_H
#define _OperatorsSetup_H

#include "OperManagerBase.h"

namespace ModelOper
{
    /**
     * @brief 求解器设置操作器
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class OperatorsSetup :public OperManagerBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Operators Setup object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        OperatorsSetup();
        /**
         * @brief Destroy the Operators Setup object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~OperatorsSetup();
        /**
         * @brief 执行
         * @return true 成功
         * @return false 失败
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        virtual bool execGUI();
        /**
         * @brief 执行结果处理
         * @return true 成功
         * @return false 失败
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOperatorRepo(actionSetupEdit, OperatorsSetup);
}

#endif