/**********************************************************************
 * @file   GUIWidgetBoolGroup.h
 * @brief  布尔组界面
 * @author BaGuijun (baguijun@163.com)
 * @date   2024-08-23
 *********************************************************************/
#ifndef _GUIWidgetBoolGroup_H
#define _GUIWidgetBoolGroup_H

#include "GUICalculateDialogAPI.h"
#include <QWidget>

namespace Ui {
    class GUIWidgetBoolGroup;
}

namespace Interface {
    class FITKFlowDataBoolGroup;
}

namespace GUI {
    /**
     * @brief  布尔组界面
     * @author BaGuijun (baguijun@163.com)
     * @date   2024-08-23
     */
    class GUICalculateDialogAPI GUIWidgetBoolGroup :public QWidget
    {
        Q_OBJECT;
    public:
        /**
         * @brief    构造函数
         * @param[i] dataBase 
         * @param[i] parent
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-23
         */
        GUIWidgetBoolGroup(Interface::FITKFlowDataBoolGroup* dataBase, QWidget* parent = nullptr);
        /**
         * @brief    析构函数
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-23
         */
        ~GUIWidgetBoolGroup();
        /**
         * @brief    初始化
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-23
         */
        void init();
    private slots:
        ;
        void on_checkBox_clicked();
    private:
        /**
         * @brief    更新界面
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-23
         */
        void updateWidget();
    private:
        /**
         * @brief  UI
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-23
         */
        Ui::GUIWidgetBoolGroup* _ui = nullptr;
        /**
         * @brief  布尔组数据
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-23
         */
        Interface::FITKFlowDataBoolGroup* _dataBase = nullptr;
    };
}

#endif
