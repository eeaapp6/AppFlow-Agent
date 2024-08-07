#ifndef _BoolInfoWidget_H
#define _BoolInfoWidget_H

#include "GUIGeometryDialogAPI.h"
#include "GeometryWidgetBase.h"

class QTableWidgetItem;

namespace Ui {
    class BoolInfoWidget;
}

namespace Interface {
    class FITKAbsGeoOperBool;
    class FITKAbstractGeoModel;
    class FITKAbsGeoCommand;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class CompFaceGroupWidget;

    class GUIGeometryDialogAPI BoolInfoWidget :public GeometryWidgetBase
    {
        Q_OBJECT;
    public:
        //编辑构造函数
        BoolInfoWidget(Interface::FITKAbsGeoOperBool* obj, EventOper::ParaWidgetInterfaceOperator* oper);
        ~BoolInfoWidget();

        void init();
        //设置面组
        void setFaceGroupValue(int rowIndex, QList<int> facesId);
        //获取当前数据对象
        Interface::FITKAbsGeoCommand* getCurrentGeoCommand();
    protected:
        /**
         * @brief 关闭事件重写
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-07-16
         */
        void closeEvent(QCloseEvent *event) override;
    private:
        bool checkValue();
        void setDataToWidget();
    private:
        bool _isCreate = false;
        Interface::FITKAbsGeoOperBool* _obj = nullptr;
        Ui::BoolInfoWidget* _ui = nullptr;
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        CompFaceGroupWidget* _faceGroupWidget = nullptr;
    };
}

#endif
