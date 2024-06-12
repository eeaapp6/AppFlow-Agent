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