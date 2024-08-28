/**********************************************************************
 * @file   BoundaryCreateDialog.h
 * @brief  边界参数创建对话框
 * @author BaGuijun (baguijun@163.com)
 * @date   2024-08-26
 *********************************************************************/
#ifndef _BoundaryCreateDialog_H
#define _BoundaryCreateDialog_H

#include "GUIWidget/GUIDialogBase.h"
#include "GUICalculateDialogAPI.h"

namespace Ui {
    class BoundaryCreateDialog;
}

namespace Interface {
    class FITKOFPhysicsData;
    class FITKFlowPhysicsHandlerFactory;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    /**
     * @brief  边界参数创建对话框
     * @author BaGuijun (baguijun@163.com)
     * @date   2024-08-26
     */
    class GUICalculateDialogAPI BoundaryCreateDialog : public GUIDialogBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief    构造函数
         * @param[i] oper 操作器对象
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        BoundaryCreateDialog(EventOper::ParaWidgetInterfaceOperator* oper);
        /**
         * @brief    析构函数
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        ~BoundaryCreateDialog();
        /**
         * @brief    初始化
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        void init();
    protected:
        /**
         * @brief    隐藏事件重写
         * @param[i] event 事件对象
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        void hideEvent(QHideEvent *event) override;
        /**
         * @brief    显示事件重写
         * @param[i] event 事件对象
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        void showEvent(QShowEvent *event) override;
    private slots:
        ;
        void on_comboBox_Boundary_activated(int index);

        void on_pushButton_OK_clicked();

        void on_pushButton_Cancel_clicked();
    private:
        /**
         * @brief    高亮网格边界
         * @param[i] meshBoundID   网格边界id
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-26
         */
        void highlightMeshBoundary(int meshBoundID);
    private:
        /**
         * @brief  ui
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-26
         */
        Ui::BoundaryCreateDialog* _ui = nullptr;
        /**
         * @brief  操作器
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-26
         */
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        /**
         * @brief  物理数据对象
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-21
         */
        Interface::FITKOFPhysicsData* _physicsData = nullptr;
        /**
         * @brief  工厂对象
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-21
         */
        Interface::FITKFlowPhysicsHandlerFactory* _factoryData = nullptr;
    };
}

#endif
