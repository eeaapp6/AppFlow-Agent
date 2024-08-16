/**
 * 
 * @file TurbulenceWidget.h
 * @brief 求解器湍流界面 
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _TurbulenceWidget_H
#define _TurbulenceWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUICalculateWidgetBase.h"

class QButtonGroup;

namespace Ui {
    class TurbulenceWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    /**
     * @brief 求解器湍流界面
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUICalculateDialogAPI TurbulenceWidget :public GUICalculateWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Turbulence Widget object
         * @param[i]  oper           操作器对象
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        TurbulenceWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        /**
         * @brief Destroy the Turbulence Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~TurbulenceWidget();
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
        void updateWidget();
    private slots:
        ;
        /**
         * @brief RadioButton按钮点击事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void slotRadioButtonClicked();
        void on_comboBox_Model_activated(int index);

        void on_pushButton_ModelUnfold_clicked();

        void on_comboBox_Delta_activated(int index);

        void on_pushButton_DeltaUnfold_clicked();

    private:
        /**
         * @brief ui
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Ui::TurbulenceWidget* _ui = nullptr;
        /**
         * @brief RadioButton group
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        QButtonGroup* _radioGroup = nullptr;
    };
}
#endif
