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
#include "FITK_Component/FITKWidget/FITKSciNotationLineEdit.h"
#include <QDoubleSpinBox>

namespace Interface {
    class FITKFlowDataBase;
    class FITKFlowDataDouble;
}

namespace Comp {
    class FITKSciNotationLineEdit;
}

namespace GUI
{
    /**
     * @brief 双精度型控件
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUICalculateDialogAPI GUIWidgetDouble :public Comp::FITKSciNotationLineEdit, public GUICalculateSubWidgetBase
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
    protected:
        //鼠标滚轮事件
        void wheelEvent(QWheelEvent *event) override;
    protected slots:
        ;
        /**
         * @brief LineEdit中文本发生更改槽函数
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-06-11
         */
        virtual void textChanged() override;
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
