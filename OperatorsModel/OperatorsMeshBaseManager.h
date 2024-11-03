/**
 * 
 * @file OperatorsMeshBaseManager.h
 * @brief 网格局部区域尺寸操作器
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _OperatorsMeshBaseManager_H
#define _OperatorsMeshBaseManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    /**
     * @brief 网格局部区域尺寸操作器
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class OperatorsMeshBaseManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Operators Mesh Base Manager object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        OperatorsMeshBaseManager();
        /**
         * @brief Destroy the Operators Mesh Base Manager object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~OperatorsMeshBaseManager();
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
    Register2FITKOperatorRepo(actionMeshBaseEdit, OperatorsMeshBaseManager);
}

#endif