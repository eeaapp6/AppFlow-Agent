#ifndef _MeshBaseWidget_H
#define _MeshBaseWidget_H

#include "GUIMeshDialogAPI.h"
#include "FITK_Kernel/FITKCore/FITKAbstractGUI.h"

namespace Ui {
    class MeshBaseWidget;
}

namespace Interface{
    class FITKRegionMeshSizeManager;
    class FITKAbstractRegionMeshSize;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class MeshBaseTypeWidgetBase;

    class GUIMeshDialogAPI MeshBaseWidget :public Core::FITKWidget
    {
        Q_OBJECT;
    public:
        //创建构造函数
        MeshBaseWidget(EventOper::ParaWidgetInterfaceOperator* oper);
        //编辑构造函数
        //MeshBaseWidget(EventOper::ParaWidgetInterfaceOperator* oper);
        ~MeshBaseWidget();

        void init();
    private slots:
        ;

        void on_comboBox_Type_activated(int index);

        void on_pushButton_Cancel_clicked();

        void on_pushButton_OK_clicked();

    private:
        void updateWidget(MeshBaseTypeWidgetBase* newWidget);
    private:
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        Ui::MeshBaseWidget* _ui = nullptr;
        Interface::FITKAbstractRegionMeshSize* _currentObj = nullptr;
        Interface::FITKRegionMeshSizeManager* _meshSizeManager = nullptr;
        MeshBaseTypeWidgetBase* _subWidget = nullptr;
    };
}

#endif
