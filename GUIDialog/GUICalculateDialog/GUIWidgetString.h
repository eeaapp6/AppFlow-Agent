/**
 * 
 * @file GUIWidgetString.h
 * @brief 字符串型控件
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _GUIWidgetString_H
#define _GUIWidgetString_H

#include "GUICalculateDialogAPI.h"
#include <QLineEdit>

namespace Interface {
    class FITKFlowDataBase;
    class FITKFlowDataString;
}

namespace GUI
{
    class GUICalculateDialogAPI GUIWidgetString :public QLineEdit
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new GUIWidgetString object
         * @param[i]  data           数据对象
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        GUIWidgetString(Interface::FITKFlowDataBase* data, QWidget* parent = nullptr);
        /**
         * @brief Destroy the GUIWidgetString object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~GUIWidgetString();
        /**
         * @brief 初始化参数
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
    private slots:
        ;
        /**
         * @brief 数据更改槽函数
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void slotDataChange();
    protected:
        /**
         * @brief 数据对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Interface::FITKFlowDataString* _value = nullptr;
    };
}

#endif
