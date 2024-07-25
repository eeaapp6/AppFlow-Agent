#ifndef LocalGroupInfoWidget_H
#define LocalGroupInfoWidget_H

#include "GUIWidget/GUIWidgetBase.h"
#include "GUIMeshDialogAPI.h"

namespace Ui {
    class LocalGroupInfoWidget;
}

namespace Interface {
    class FITKGeometryMeshSize;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class GUIMeshDialogAPI LocalGroupInfoWidget : public GUIWidgetBase
    {
        Q_OBJECT;
    public:
        LocalGroupInfoWidget(Interface::FITKGeometryMeshSize* obj, EventOper::ParaWidgetInterfaceOperator* oper);
        ~LocalGroupInfoWidget();

        void init();

        Interface::FITKGeometryMeshSize* getCurrentGeoMeshObj();
    private slots:
        ;
        void on_pushButton_Cancel_clicked();

        void on_pushButton_OK_clicked();
    private:
        bool checkValue();
        void setDataToWidget();
        void getDataFromWidget();
    private:
        Ui::LocalGroupInfoWidget* _ui = nullptr;
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        Interface::FITKGeometryMeshSize* _obj = nullptr;
    };
}

#endif
