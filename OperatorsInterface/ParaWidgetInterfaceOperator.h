/**
 *
 * @file ParaWidgetInterfaceOperator.h
 * @brief 与参数窗口相关的operator基类
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-01
 *
 */
#ifndef _PARAWIDGET_OPERATOR_INTERFACE_H___
#define _PARAWIDGET_OPERATOR_INTERFACE_H___

#include "FITK_Kernel/FITKCore/FITKActionOperator.h"
#include "OperatorsInterfaceAPI.h"

namespace EventOper
{
    /**
     * @brief 参数窗口相关的widget
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-01
     */
    class OperatorsInterfaceAPI ParaWidgetInterfaceOperator : public Core::FITKActionOperator
    {
    public:
        /**
         * @brief Construct a new Para Widget Interface Oper object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-01
         */
        ParaWidgetInterfaceOperator();
        /**
         * @brief Destroy the Para Widget Interface Oper object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-01
         */
        virtual ~ParaWidgetInterfaceOperator();
        /**
         * @brief 其他事件执行
         * @param[i]  index          事件索引（自定义）
         * @param[i]  value          其他数据
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-01
         */
        virtual void eventProcess(int index, QVariant value = QVariant());
    };
}

#endif
