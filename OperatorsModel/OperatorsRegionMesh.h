/**********************************************************************
 * @file   OperatorsRegionMesh.h
 * @brief  区域网格操作器
 * @author liuzhonghua (liuzhonghuaszch@163.com)
 * @date   2025-06-23
 *********************************************************************/
#ifndef _OperatorsRegionMesh_H
#define _OperatorsRegionMesh_H

#include "OperManagerBase.h"

namespace ModelOper
{
    /**
     * @brief 求解器湍流操作器
     * @author liuzhonghua (liuzhonghuaszch@163.com)
     * @date 2024-08-14
     */
    class OperatorsRegionMesh :public OperManagerBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Operators RegionMesh object
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date 2024-08-14
         */
        OperatorsRegionMesh();
        /**
         * @brief Destroy the Operators RegionMesh object
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date 2024-08-14
         */
        ~OperatorsRegionMesh();
        /**
         * @brief 执行
         * @return true 成功
         * @return false 失败
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date 2024-08-14
         */
        virtual bool execGUI();
        /**
         * @brief 执行结果处理
         * @return true 成功
         * @return false 失败
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date 2024-08-14
         */
        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOperatorRepo(actionRegionMeshEdit, OperatorsRegionMesh);
}

#endif