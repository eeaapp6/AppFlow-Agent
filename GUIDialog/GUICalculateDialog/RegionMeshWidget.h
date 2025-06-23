/**********************************************************************
 * @file   RegionMeshWidget.h
 * @brief  区域网格信息界面声明
 * @author liuzhonghua (liuzhonghuaszch@163.com)
 * @date   2025-06-23
 *********************************************************************/
#ifndef _RegionMeshWidget_H
#define _RegionMeshWidget_H

#include "GUICalculateDialogAPI.h"
#include "FITK_Kernel/FITKCore/FITKAbstractGUI.h"

class QTableWidgetItem;

namespace Ui{
    class RegionMeshWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    /**
     * @brief cude create or edit widget
     * @author liuzhonghua (liuzhonghuaszch@163.com)
     * @date 2024-08-14
     */
    class GUICalculateDialogAPI RegionMeshWidget :public Core::FITKWidget
    {
        Q_OBJECT;
    public:
        /**
         * @brief create constructor
         * @param[i]  oper           operators
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date 2024-08-14
         */
        RegionMeshWidget(EventOper::ParaWidgetInterfaceOperator* oper);
        /**
         * @brief Destroy the Mesh Info Widget object
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date 2024-08-14
         */
        ~RegionMeshWidget();
        /**
         * @brief init
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date 2024-08-14
         */
        void init();
    protected:
        /**
         * @brief close event override
         * @param[i]  event          event object
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date 2024-08-14
         */
        void closeEvent(QCloseEvent *event) override;

    private slots:
        void setDataFormWidgetSlot(int index);

    private:
        /**
         * @brief ui object
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date 2024-08-14
         */
        Ui::RegionMeshWidget* _ui = nullptr;
        /**
         * @brief 操作器对象
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date 2024-08-14
         */
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
    };
}

#endif
