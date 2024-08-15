/**
 * 
 * @file GUIWidgetComBox.h
 * @brief combox类型控件
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _GUIWidgetComBox_H
#define _GUIWidgetComBox_H

#include "GUICalculateDialogAPI.h"
#include "GUICalculateWidgetBase.h"
#include <QComboBox>

namespace Interface {
    class FITKFlowDataBase;
    class FITKFlowDataCombox;
}

namespace GUI
{
    /**
     * @brief combox类型控件
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUICalculateDialogAPI GUIWidgetComBox :public QComboBox, public GUICalculateSubWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new GUIWidgetComBox object
         * @param[i]  data           数据对象
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        GUIWidgetComBox(Interface::FITKFlowDataBase* data, QWidget* parent = nullptr);
        /**
         * @brief Destroy the GUIWidgetComBox object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~GUIWidgetComBox();
        /**
         * @brief 初始化
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
         * @brief combox数据对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Interface::FITKFlowDataCombox* _value = nullptr;
    };
}

#endif
