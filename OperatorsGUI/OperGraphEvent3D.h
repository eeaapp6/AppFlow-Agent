/*****************************************************************//**
 * @file    OperGraphEvent3D.h
 * @brief   三维可视化事件操作器抽象类。
 *  
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-06-12
 *********************************************************************/

#ifndef __OPERGRAPHEVENT3D_H__
#define __OPERGRAPHEVENT3D_H__

#include "OperatorsInterface/GraphEventOperator.h"

// 前置声明
namespace Core
{
    class FITKAbstractGraph3DWidget;
    class FITKAbstractGraphObject;
}

namespace GUIOper
{
    /**
     * @brief   可视化事件操作器。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-06-12
     */
    class OperGraphEvent3D : public EventOper::GraphEventOperator
    {
        Q_OBJECT

    protected:
        /**
         * @brief   构造函数。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        explicit OperGraphEvent3D() = default;

        /**
         * @brief   析构函数。[虚函数]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        virtual ~OperGraphEvent3D() = default;

        /**
         * @brief   获取三维可视化窗口。
         * @return  可视化窗口
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        Core::FITKAbstractGraph3DWidget* getGraphWidget();

        /**
         * @brief   添加可视化对象至三维窗口。
         * @param   obj：可视化对象
         * @param   graphWidget：三维窗口
         * @param   fitView：是否重置视角[缺省]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        void addGraphObjectToWidget(Core::FITKAbstractGraphObject* obj, Core::FITKAbstractGraph3DWidget* graphWidget, bool fitView = false);

    };

}  // namespace GUIOper

#endif // __OPERGRAPHEVENT3D_H__
