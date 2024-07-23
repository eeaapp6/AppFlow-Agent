#ifndef _CudeInfoWidget_H
#define _CudeInfoWidget_H

#include "GUIGeometryDialogAPI.h"
#include "FITK_Kernel/FITKCore/FITKAbstractGUI.h"

namespace Ui{
    class CudeInfoWidget;
}

namespace Interface {
    class FITKAbsGeoModelBox;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class GUIGeometryDialogAPI CudeInfoWidget :public Core::FITKWidget
    {
        Q_OBJECT;
    public:
        //创建构造函数
        CudeInfoWidget(EventOper::ParaWidgetInterfaceOperator* oper);
        //编辑构造函数
        CudeInfoWidget(Interface::FITKAbsGeoModelBox* obj, EventOper::ParaWidgetInterfaceOperator* oper);
        ~CudeInfoWidget();

        void init();
        //重新设置基点
        void setBasicPoint(double* point);
    private slots:
        ;
        void on_pushButton_BasicPoint_clicked();

        void on_pushButton_Cancel_clicked();

        void on_pushButton_CreateOrEdit_clicked();

        void on_pushButton_Clear_clicked();

        void on_pushButton_Add_clicked();
    private:
        bool checkValue();
        void setDataToWidget();
        void getDataFormWidget();
    private:
        bool _isCreate = false;
        Interface::FITKAbsGeoModelBox* _obj = nullptr;
        Ui::CudeInfoWidget* _ui = nullptr;
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
    };
}

#endif
