/**
 * 
 * @file OperatorsMeshGeoManager.h
 * @brief 几何相关网格局部区域尺寸操作器
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _OperatorsMeshGeoManager_H
#define _OperatorsMeshGeoManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    /**
     * @brief 几何相关网格局部区域尺寸操作器
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class OperatorsMeshGeoManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Operators Mesh Geo Manager object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        OperatorsMeshGeoManager();
        /**
         * @brief Destroy the Operators Mesh Geo Manager object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~OperatorsMeshGeoManager();
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
    Register2FITKOperatorRepo(actionMeshGeoEdit, OperatorsMeshGeoManager);
    Register2FITKOperatorRepo(actionMeshGeoDelete, OperatorsMeshGeoManager);
}

#endif