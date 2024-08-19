/**
 * 
 * @file GUIWidgetInt.h
 * @brief 整型类型控件
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _GUIWidgetInt_H
#define _GUIWidgetInt_H

#include "GUICalculateDialogAPI.h"
#include "GUICalculateWidgetBase.h"
#include <QSpinBox>

namespace Interface {
    class FITKFlowDataBase;
    class FITKFlowDataInt;
}

namespace GUI
{
    /**
     * @brief 整型类型控件
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUICalculateDialogAPI GUIWidgetInt :public QSpinBox, public GUICalculateSubWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new GUIWidgetInt object
         * @param[i]  data           数据对象
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        GUIWidgetInt(Interface::FITKFlowDataBase* data, QWidget* parent = nullptr);
        /**
         * @brief Destroy the GUIWidgetInt object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~GUIWidgetInt();
        /**
         * @brief 初始化
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
    protected:
        //鼠标滚轮事件
        void wheelEvent(QWheelEvent *event) override;
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
        Interface::FITKFlowDataInt* _value = nullptr;
    };
}

#endif
