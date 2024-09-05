/**
 * 
 * @file InitialWidget.h
 * @brief 求解器初始化参数界面
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _InitialWidget_H
#define _InitialWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUICalculateWidgetBase.h"

namespace Ui {
    class InitialWidget;
}

namespace Interface {
    class FITKOFInitialConditions;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class InitialCreatePatchDialog;
    /**
     * @brief 求解器初始化参数界面
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUICalculateDialogAPI InitialWidget :public GUICalculateWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Initial Widget object
         * @param[i]  oper           操作器对象
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        InitialWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        /**
         * @brief Destroy the Initial Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~InitialWidget();
        /**
         * @brief 初始化
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
        /**
         * @brief    更新界面
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-04
         */
        void updateWidget();
    private slots:
        ;
        /**
         * @brief    patch添加
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-04
         */
        void on_pushButton_PatchAdd_clicked();
    private:
        /**
         * @brief    更新Basic界面
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-23
         */
        void updateBasicWidget();
        /**
         * @brief    更新Patch界面
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-09-05
         */
        void updatePatchWidget();
    private:
        /**
         * @brief ui
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Ui::InitialWidget* _ui = nullptr;
        /**
         * @brief  初始化对象
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-23
         */
        Interface::FITKOFInitialConditions* _initValue = nullptr;
    };
}
#endif
