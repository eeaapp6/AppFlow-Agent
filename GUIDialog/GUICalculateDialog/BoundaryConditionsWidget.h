/**
 * 
 * @file BoundaryConditionsWidget.h
 * @brief 求解器参数边界界面
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _BoundaryConditionsWidget_H
#define _BoundaryConditionsWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUICalculateWidgetBase.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFEnum.hpp"

namespace Ui {
    class BoundaryConditionsWidget;
}

namespace Interface {
    class FITKOFBoundary;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    /**
     * @brief 求解器参数边界界面
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUICalculateDialogAPI BoundaryConditionsWidget :public GUICalculateWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Boundary Widget object
         * @param[i]  oper           操作器对象
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        BoundaryConditionsWidget(Interface::FITKOFBoundary* boundaryObj, EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        /**
         * @brief Destroy the Boundary Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~BoundaryConditionsWidget();
        /**
         * @brief 初始化
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
        /**
         * @brief    更新
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-27
         */
        void update();
    protected:
        /**
         * @brief    显示事件
         * @param[i] event 事件
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-27
         */
        void showEvent(QShowEvent *event) override;
        /**
         * @brief    隐藏事件
         * @param[i] event 事件
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-27
         */
        void hideEvent(QHideEvent *event) override;
    private:
        void updateFlow();
        void updateTurbulence();
        void updatePhases();
    private:
        /**
         * @brief ui
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
         Ui::BoundaryConditionsWidget* _ui = nullptr;
         /**
          * @brief  边界数据对象
          * @author BaGuijun (baguijun@163.com)
          * @date   2024-08-27
          */
         Interface::FITKOFBoundary* _boundaryObj = nullptr;
         /**
          * @brief  当前边界类型
          * @author BaGuijun (baguijun@163.com)
          * @date   2024-08-27
          */
         Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType _currentType = Interface::FITKOFSolverTypeEnum::FITKOFBoundaryType::BNone;
    };
}
#endif
