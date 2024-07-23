/**
 *
 * @file ParaWidgetInterfaceOperator.h
 * @brief 与参数窗口相关的operator基类

 *
 */
#ifndef _PARAWIDGET_OPERATOR_INTERFACE_H___
#define _PARAWIDGET_OPERATOR_INTERFACE_H___

#include "FITK_Kernel/FITKCore/FITKActionOperator.h"
#include "OperatorsInterfaceAPI.h"

namespace EventOper
{
    /**
     * @brief 与参数窗口相关的widget

     */
    class OperatorsInterfaceAPI ParaWidgetInterfaceOperator : public Core::FITKActionOperator
    {
    public:
        /**
         * @brief Construct a new Para Widget Interface Oper object

         */
        ParaWidgetInterfaceOperator();
        /**
         * @brief Destroy the Para Widget Interface Oper object

         */
        virtual ~ParaWidgetInterfaceOperator();
        //其他事件执行
        virtual void moveToStep(int index, QVariant value = QVariant());
    };
}

#endif
