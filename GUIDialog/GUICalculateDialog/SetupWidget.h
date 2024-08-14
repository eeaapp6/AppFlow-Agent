/**
 * 
 * @file SetupWidget.h
 * @brief 求解器类型选择界面
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _SetupWidget_H
#define _SetupWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUIWidget/GUIWidgetBase.h"/*
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFEnum.hpp"*/

namespace Ui {
    class SetupWidget;
}

namespace Interface {
    class FITKOFSetUpCase;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    /**
     * @brief 求解器类型选择界面
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUICalculateDialogAPI SetupWidget : public GUIWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Setup Widget object
         * @param[i]  oper           操作器对象
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        SetupWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        /**
         * @brief Destroy the Setup Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~SetupWidget();
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
        void updateTableWidget();
    private:
        /**
         * @brief 求解器类型转名称
         * @param[i]  type           求解器类型
         * @return QString           求解器名称
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        /*QString typeToName(Interface::FITKOFPostProcessEnum::FITKOFSolverType type);*/
        /**
         * @brief 初始化设置类型
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void initSetupType();
        /**
         * @brief 初始化当前的求解器类型
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void initCurrentType();
    private slots:
        void on_radioButton_SteadyState_clicked();

        void on_radioButton_Transient_clicked();

        void on_radioButton_Incompressible_clicked();

        void on_radioButton_Compressible_clicked();

        void on_pushButton_Select_clicked();

    private:
        Interface::FITKOFSetUpCase* _setUpCase = nullptr;
        /**
         * @brief 操作器对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        /**
         * @brief ui对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Ui::SetupWidget* _ui = nullptr;/*
        QList<Interface::FITKOFPostProcessEnum::FITKOFSolverType> _types = {};*/
    };
}

#endif
