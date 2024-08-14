/**
 * 
 * @file OperatorsBoundary.h
 * @brief 网格边界操作器
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _OperatorsBoundary_H
#define _OperatorsBoundary_H

#include "OperManagerBase.h"

namespace ModelOper
{
    /**
     * @brief 网格边界操作器
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class OperatorsBoundary :public OperManagerBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Operators Boundary object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        OperatorsBoundary();
        /**
         * @brief Destroy the Operators Boundary object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~OperatorsBoundary();
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
    Register2FITKOPeratorRepo(actionBoundaryEdit, OperatorsBoundary);
}

#endif