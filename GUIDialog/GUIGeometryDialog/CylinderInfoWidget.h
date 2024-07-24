#ifndef _CylinderInfoWidget_H
#define _CylinderInfoWidget_H

#include "GUIGeometryDialogAPI.h"
#include "FITK_Kernel/FITKCore/FITKAbstractGUI.h"

class QTableWidgetItem;

namespace Ui {
    class CylinderInfoWidget;
}

namespace Interface {
    class FITKAbsGeoModelCylinder;
    class FITKAbstractGeoModel;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class GUIGeometryDialogAPI CylinderInfoWidget :public Core::FITKWidget
    {
        Q_OBJECT;
    public:
        //创建构造函数
        CylinderInfoWidget(EventOper::ParaWidgetInterfaceOperator* oper);
        //编辑构造函数
        CylinderInfoWidget(Interface::FITKAbsGeoModelCylinder* obj, EventOper::ParaWidgetInterfaceOperator* oper);
        ~CylinderInfoWidget();

        void init();

        void setOriginPoint(double* point);

        void setFaceGroupValue(int rowIndex, int faceId);
    private slots:
        ;
        void on_pushButton_OriginPoint_clicked();

        void on_pushButton_Cancel_clicked();

        void on_pushButton_CreateOrEdit_clicked();

        void on_pushButton_Clear_clicked();

        void on_pushButton_Add_clicked();
        /**
         * @brief 表格点击事件
         * @param[i]  item           表格对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void itemTableClickedSlot(QTableWidgetItem* item);
        /**
         * @brief 表格双击事件
         * @param[i]  item           表格对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void itemTableDoubleClickedSlot(QTableWidgetItem* item);
    private:
        bool checkValue();
        void setDataToWidget();
        void getDataFormWidget();
        void initTableWidget();
    private:
        bool _isCreate = false;
        Interface::FITKAbsGeoModelCylinder* _obj = nullptr;
        Interface::FITKAbstractGeoModel* _geoModel = nullptr;
        Ui::CylinderInfoWidget* _ui = nullptr;
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
    };
}

#endif
