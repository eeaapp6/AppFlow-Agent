/**
 * 
 * @file GUICalculateWidgetBase.h
 * @brief 求解器参数界面基类
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _GUICalculateWidgetBase_H
#define _GUICalculateWidgetBase_H

#include "GUICalculateDialogAPI.h"
#include "GUIWidget/GUIWidgetBase.h"

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace Interface {
    class FITKOFSolverData;
    class FITKFlowSolverProcessFactory;
}

namespace GUI
{
    /**
     * @brief 求解器参数界面基类
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUICalculateDialogAPI GUICalculateWidgetBase :public GUIWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new GUICalculateWidgetBase object
         * @param[i]  oper           操作器对象
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        GUICalculateWidgetBase(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        /**
         * @brief Destroy the GUICalculateWidgetBase object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~GUICalculateWidgetBase();
        /**
         * @brief 更新表格
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        virtual void updateTableWidget();
    protected:
        /**
         * @brief 操作器对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        EventOper::ParaWidgetInterfaceOperator*  _oper = nullptr;
        /**
         * @brief 求解器数据对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Interface::FITKOFSolverData* _solverData = nullptr;
        /**
         * @brief 工厂对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Interface::FITKFlowSolverProcessFactory* _factor = nullptr;
    };
    //---------------------------------------------------------------------------------------------------------------
    class GUICalculateDialogAPI GUICalculateSubWidgetBase
    {
    public:
        GUICalculateSubWidgetBase(QWidget* parent = nullptr);
        virtual ~GUICalculateSubWidgetBase();
    protected:
        GUICalculateWidgetBase* _calculateWdiget = nullptr;
    };
}
#endif
