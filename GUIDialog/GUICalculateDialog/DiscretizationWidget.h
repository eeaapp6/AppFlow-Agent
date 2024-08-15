/**
 * 
 * @file DiscretizationWidget.h
 * @brief 求解器离散数据界面
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _DiscretizationWidget_H
#define _DiscretizationWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUICalculateWidgetBase.h"

namespace Ui {
    class DiscretizationWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    /**
     * @brief 求解器离散数据界面
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUICalculateDialogAPI DiscretizationWidget :public GUICalculateWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Discretization Widget object
         * @param[i]  oper           操作器对象
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        DiscretizationWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        /**
         * @brief Destroy the Discretization Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~DiscretizationWidget();
        /**
         * @brief 初始化
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
        /**
         * @brief 更新表格
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        virtual void updateTableWidget() override;
    private:
        /**
         * @brief ui
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Ui::DiscretizationWidget* _ui = nullptr;
    };
}
#endif
