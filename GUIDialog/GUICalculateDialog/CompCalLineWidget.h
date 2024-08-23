/**
 * 
 * @file CompCalLineWidget.h
 * @brief 求解器单个参数组件界面
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-16
 * 
 */
#ifndef _CompCalLineWidget_H
#define _CompCalLineWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUIWidget/GUIWidgetBase.h"

class QToolBox;

namespace Ui {
    class CompCalLineWidget;
}

namespace Interface {
    class FITKFlowDataBase;
}

namespace GUI
{
    /**
     * @brief 求解器单个参数组件界面
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-16
     */
    class GUICalculateDialogAPI CompCalLineWidget : public GUIWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new comp Cal Line Widget object
         * @param[i]  data           数据对象
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-16
         */
        CompCalLineWidget(Interface::FITKFlowDataBase* data, QWidget* parent = nullptr);
        /**
         * @brief Destroy the comp Cal Line Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-16
         */
        ~CompCalLineWidget();
        /**
         * @brief 初始化函数
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-16
         */
        void init();
        /**
         * @brief 数据类型转换为控件【静态函数】
         * @param[i]  data           数据对象
         * @param[i]  parent         父对象
         * @return GUICalculateDialogAPI*
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        static QWidget* DataSwitchToWidget(Interface::FITKFlowDataBase* data, QWidget* parent = nullptr, QString name = "");
        /**
         * @brief    创建QToolBox
         * @param[i] parent 父对象
         * @return   QToolBox *  QToolBox对象
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-23
         */
        static QToolBox* CreateToolBox(QWidget* parent = nullptr);
    private:
        /**
         * @brief ui对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-16
         */
        Ui::CompCalLineWidget* _ui = nullptr;
        /**
         * @brief 数据对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-16
         */
        Interface::FITKFlowDataBase* _data = nullptr;
    };
}

#endif
