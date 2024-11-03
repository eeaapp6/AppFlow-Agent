/**
 * 
 * @file OperatorsInitial.h
 * @brief 求解器初始化操作器
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _OperatorsInitial_H
#define _OperatorsInitial_H

#include "OperManagerBase.h"

namespace ModelOper
{
    /**
     * @brief 求解器初始化操作器
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class OperatorsInitial :public OperManagerBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Operators Initial object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        OperatorsInitial();
        /**
         * @brief Destroy the Operators Initial object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~OperatorsInitial();
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
    Register2FITKOperatorRepo(actionInitialEdit, OperatorsInitial);
    Register2FITKOperatorRepo(actionInitialDeleteGeo, OperatorsInitial);
}

#endif