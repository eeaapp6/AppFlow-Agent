/**********************************************************************
 * @file   CompVBoxWidget.h
 * @brief  水平布局组件界面
 * @author BaGuijun (baguijun@163.com)
 * @date   2024-08-22
 *********************************************************************/
#ifndef _CompVBoxWidget_H
#define _CompVBoxWidget_H

#include "GUICalculateDialogAPI.h"
#include <QWidget>

namespace Ui {
    class CompVBoxWidget;
}

namespace Interface {
    class FITKFlowDataBase;
}

class QRadioButton;

namespace GUI
{
    /**
     * @brief  水平布局组件界面
     * @author BaGuijun (baguijun@163.com)
     * @date   2024-08-22
     */
    class GUICalculateDialogAPI CompVBoxWidget :public QWidget
    {
        Q_OBJECT;
    public:
        CompVBoxWidget(QList<QWidget*> widgetList, QWidget* parent = nullptr);
        /**
         * @brief    析构函数
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-22
         */
        ~CompVBoxWidget();
        /**
         * @brief    设置子项界面不可显示
         * @param[i] enable     是否显示
         * @param[i] isFirst    首项是否跳过
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-22
         */
        void setSubWidgetEnable(bool enable, bool isFirst = true);
    private:
        /**
         * @brief  ui对象
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-08-22
         */
        Ui::CompVBoxWidget* _ui = nullptr;

        QList<QWidget*> _subWidgets = {};
    };
}

#endif // !_CompVBoxWidget_H
