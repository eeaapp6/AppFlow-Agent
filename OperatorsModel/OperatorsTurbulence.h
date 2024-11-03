/**
 * 
 * @file OperatorsTurbulence.h
 * @brief 求解器湍流操作器
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _OperatorsTurbulence_H
#define _OperatorsTurbulence_H

#include "OperManagerBase.h"

namespace ModelOper
{
    /**
     * @brief 求解器湍流操作器
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class OperatorsTurbulence :public OperManagerBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Operators Turbulence object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        OperatorsTurbulence();
        /**
         * @brief Destroy the Operators Turbulence object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~OperatorsTurbulence();
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
    Register2FITKOperatorRepo(actionTurbulenceEdit, OperatorsTurbulence);
}

#endif