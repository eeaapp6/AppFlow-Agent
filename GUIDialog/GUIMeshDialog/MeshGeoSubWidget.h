/**
 * 
 * @file MeshGeoSubWidget.h
 * @brief 几何相关的网格区域尺寸子界面
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _MeshGeoSubWidget_H
#define _MeshGeoSubWidget_H

#include "GUIWidget/GUIWidgetBase.h"

namespace Ui {
    class MeshGeoSubWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace Interface {
    class FITKRegionMeshSizeGeom;
}

namespace GUI
{
    /**
     * @brief 几何相关的网格区域尺寸子界面
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class MeshGeoSubWidget :public GUIWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Mesh Geo Sub Widget object
         * @param[i]  geoID          几何id
         * @param[i]  oper           操作器对象
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        MeshGeoSubWidget(int geoID, EventOper::ParaWidgetInterfaceOperator * oper, QWidget* parent = nullptr);
        /**
         * @brief Destroy the Mesh Geo Sub Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~MeshGeoSubWidget();
        /**
         * @brief 设置名称
         * @param[i]  name           名称
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void setName(const QString& name);
        /**
         * @brief 获取当前的几何id
         * @return int id
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        int getObjID();
        /**
         * @brief 初始化
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void init();
    private slots:
        /**
         * @brief 最小细化参数编辑事件
         * @param[i]  arg1           数据
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_spinBox_Min_valueChanged(int arg1);
        /**
         * @brief 最大细化参数编辑事件
         * @param[i]  arg1           数据
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_spinBox_Max_valueChanged(int arg1);
    private:
        /**
         * @brief Set the Data To Widget object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void setDataToWidget();
    private:
        /**
         * @brief 几何相关网格局部区域尺寸对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Interface::FITKRegionMeshSizeGeom* _geoMeshSize = nullptr;
        /**
         * @brief 操作器对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        /**
         * @brief ui对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Ui::MeshGeoSubWidget* _ui = nullptr;
        /**
         * @brief 几何对象id
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        int _objID = -1;
    };
}

#endif
