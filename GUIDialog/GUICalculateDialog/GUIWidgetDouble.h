/**
 * 
 * @file GUIWidgetDouble.h
 * @brief 双精度型控件
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _GUIWidgetDouble_H
#define _GUIWidgetDouble_H

#include "GUICalculateDialogAPI.h"
#include "GUICalculateWidgetBase.h"
#include <QDoubleSpinBox>

namespace Interface {
    class FITKFlowDataBase;
    class FITKFlowDataDouble;
}

namespace GUI
{
    /**
     * @brief 双精度型控件
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUICalculateDialogAPI GUIWidgetDouble :public QDoubleSpinBox, public GUICalculateSubWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new GUIWidgetDouble object
         * @param[i]  data           数据对象
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        GUIWidgetDouble(Interface::FITKFlowDataBase* data, QWidget* parent = nullptr);
        /**
         * @brief Destroy the GUIWidgetDouble object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~GUIWidgetDouble();
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
        Interface::FITKFlowDataDouble* _value = nullptr;
    };
}

#endif
