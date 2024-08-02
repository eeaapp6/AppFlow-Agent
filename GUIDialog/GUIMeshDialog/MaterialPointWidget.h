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
    protected:
        /**
         * @brief 隐藏事件
         * @param[i]  event           事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void hideEvent(QHideEvent* event);
    private slots:
        ;
        /**
         * @brief 清除事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void on_pushButton_Clear_clicked();
        /**
         * @brief 添加事件点击
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void on_pushButton_Add_clicked();
        /**
         * @brief 表格点击事件
         * @param[i]  item           表格对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void slotCellTableClicked(int row, int column);
        /**
         * @brief 材料点组件界面删除事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void slotMatPointWidgetDeleteClicked();
        /**
         * @brief 材料点组件点位数据更新时间
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void slotMatPointWidgetPointChange();
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
        /**
         * @brief 数据检查
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        bool checkValue();
        /**
         * @brief 设置数据至界面
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void setDataToWidget();
        /**
         * @brief 使用界面参数存储数据
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void getDataFromWidget();
        /**
         * @brief 检查名称是否存在
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        bool checkName(const QString& name);
        /**
         * @brief 更新渲染窗口
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void updateGraph(bool isShow = true);
    private:
        Ui::MaterialPointWidget* _ui = nullptr;
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
    };
}

#endif
