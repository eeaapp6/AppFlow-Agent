#ifndef _MaterialPointWidget_H
#define _MaterialPointWidget_H

#include "GUIMeshDialogAPI.h"
#include "GUIWidget/GUIWidgetBase.h"

namespace Ui {
    class MaterialPointWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class GUIMeshDialogAPI MaterialPointWidget :public GUIWidgetBase
    {
        Q_OBJECT;
    public:
        MaterialPointWidget(EventOper::ParaWidgetInterfaceOperator* oper);
        ~MaterialPointWidget();

        void init();

        void initTableWidget();
    private slots:
        ;
        void on_pushButton_Add_clicked();
        /**
         * @brief 表格点击事件
         * @param[i]  item           表格对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void slotCellTableClicked(int row, int column);

        void slotMatPointWidgetDeleteClicked();
    private:
        /**
         * @brief 刷新面组界面记录的位置
         * （为解决点击界面控件时，QTableWidget未触发不知道当前界面所在的位置问题）
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void updateFaceWidgetCurrentPos();
        /**
         * @brief 清除所有高亮
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void clearGraphHight();
        /**
         * @brief 清除Table
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void clearTableWidget();
    private:
        Ui::MaterialPointWidget* _ui = nullptr;
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
    };
}

#endif
