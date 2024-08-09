#ifndef _SphereInfoWidget_H
#define _SphereInfoWidget_H

#include "GUIGeometryDialogAPI.h"
#include "GeometryWidgetBase.h"

namespace Ui {
    class SphereInfoWidget;
}

namespace Interface {
    class FITKAbsGeoModelSphere;
    class FITKAbsGeoCommand;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class CompFaceGroupWidget;

    class GUIGeometryDialogAPI SphereInfoWidget :public GeometryWidgetBase
    {
        Q_OBJECT;
    public:
        //创建构造函数
        SphereInfoWidget(EventOper::ParaWidgetInterfaceOperator* oper);
        //编辑构造函数
        SphereInfoWidget(Interface::FITKAbsGeoModelSphere* obj, EventOper::ParaWidgetInterfaceOperator* oper);
        ~SphereInfoWidget();

        void init();
        //重新设置基点
        void setCenterPoint(double* point);

        void setFaceGroupValue(int rowIndex, QList<int> facesId);

        Interface::FITKAbsGeoCommand* getCurrentGeoCommand();
    protected:
        /**
         * @brief 关闭事件重写
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void closeEvent(QCloseEvent *event) override;
    private slots:
        ;
        void on_pushButton_CenterPoint_clicked();

        void on_pushButton_Cancel_clicked();

        void on_pushButton_CreateOrEdit_clicked();
    private:
        bool checkValue();
        void setDataToWidget();
        void getDataFormWidget();
        /**
         * @brief 是否切换为创建模式
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void switchCreateModel(bool isCreate);
    private:
        bool _isCreate = false;
        Interface::FITKAbsGeoModelSphere* _obj = nullptr;
        Ui::SphereInfoWidget* _ui = nullptr;
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        CompFaceGroupWidget* _faceGroupWidget = nullptr;
    };
}

#endif
