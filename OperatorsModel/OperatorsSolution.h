/**
 * 
 * @file OperatorsSolution.h
 * @brief 求解器流场解操作器
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _OperatorsSolution_H
#define _OperatorsSolution_H

#include "OperManagerBase.h"

namespace ModelOper
{
    /**
     * @brief 求解器流场解操作器
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class OperatorsSolution :public OperManagerBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Operators Solution object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        OperatorsSolution();
        /**
         * @brief Destroy the Operators Solution object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~OperatorsSolution();
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
    Register2FITKOperatorRepo(actionSolutionEdit, OperatorsSolution);
}

#endif