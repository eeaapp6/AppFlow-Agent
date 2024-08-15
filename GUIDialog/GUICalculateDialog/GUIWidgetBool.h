/**
 * 
 * @file GUIWidgetBool.h
 * @brief 布尔类型控件
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _GUIWidgetBool_H
#define _GUIWidgetBool_H

#include "GUICalculateDialogAPI.h"
#include "GUICalculateWidgetBase.h"
#include <QCheckBox>

namespace Interface {
    class FITKFlowDataBase;
    class FITKFlowDataBool;
}

namespace GUI
{
    /**
     * @brief 布尔类型控件
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUICalculateDialogAPI GUIWidgetBool :public QCheckBox, public GUICalculateSubWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new GUIWidgetBool object
         * @param[i]  data           数据对象
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        GUIWidgetBool(Interface::FITKFlowDataBase* data, QWidget* parent = nullptr);
        /**
         * @brief Destroy the GUIWidgetBool object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~GUIWidgetBool();
        /**
         * @brief 初始化
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
    private slots:
        ;
        /**
         * @brief 数据更改
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
        Interface::FITKFlowDataBool* _value = nullptr;
    };
}

#endif
