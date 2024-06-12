/*****************************************************************//**
 * @file    GraphEventOperator.h
 * @brief   三维可视对象数据操作器接口类。
 *  
 * @author  ChengHaotian (yeguangbaozi@foxmail.com)
 * @date    2024-06-12
 *********************************************************************/

#ifndef __GRAPHEVENTOPERATOR_H__
#define __GRAPHEVENTOPERATOR_H__

#include "FITK_Kernal/FITKCore/FITKAbstractOperator.h"

#include "OperatorsInterfaceAPI.h"

#include <QColor>

namespace EventOper
{
    /**
     * @brief   三维可视对象数据操作器接口类。
     * @author  ChengHaotian (yeguangbaozi@foxmail.com)
     * @date    2024-06-12
     */
    class OperatorsInterfaceAPI GraphEventOperator : public Core::FITKAbstractOperator
    {
        Q_OBJECT

    public:
        /**
         * @brief   构造函数。
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        explicit GraphEventOperator() = default;

        /**
         * @brief   析构函数。[虚函数]
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        virtual ~GraphEventOperator() = default;

        // 前处理渲染与窗口功能。
        //@{
        /**
         * @brief   根据提供的数据对象ID更新对应可视化对象。（没有则创建）[虚函数]
         * @param   id：数据对象ID
         * @author  ChengHaotian (yeguangbaozi@foxmail.com)
         * @date    2024-06-12
         */
        virtual void updateGraph(int id);
        //@}

        // 高亮功能接口。
        //@{
        
        //@}

        // 渲染专用接口。
        //@{

        //@}

        // 工具方法。
        //@{

        //@}

    };
}   // namespace EventOper

#endif // __GRAPHEVENTOPERATOR_H__