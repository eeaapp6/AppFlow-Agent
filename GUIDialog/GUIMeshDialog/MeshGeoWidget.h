/**
 * 
 * @file MeshGeoWidget.h
 * @brief 几何相关的网格区域尺寸界面
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _MeshGeoWidget_H
#define _MeshGeoWidget_H

#include "GUIMeshDialogAPI.h"
#include "GUIWidget/GUIWidgetBase.h"

class QVBoxLayout;

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    /**
     * @brief 几何相关的网格区域尺寸界面
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUIMeshDialogAPI MeshGeoWidget :public GUIWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Mesh Geo Widget object
         * @param[i]  oper           操作器对象
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        MeshGeoWidget(EventOper::ParaWidgetInterfaceOperator * oper, QWidget* parent = nullptr);
        /**
         * @brief Destroy the Mesh Geo Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~MeshGeoWidget();
        /**
         * @brief 初始化界面
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
        /**
         * @brief 更新子界面
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void updateSubWidget();
    private:
        /**
         * @brief 操作器对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        /**
         * @brief 界垂直面布局对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        QVBoxLayout* _subWidgetLayout = nullptr;
    };
}

#endif
