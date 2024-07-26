#ifndef _LocalGroupDeleteDialog_H
#define _LocalGroupDeleteDialog_H

#include "GUIWidget/GUIDialogBase.h"
#include "GUIMeshDialogAPI.h"

namespace Ui {
    class MeshDeleteDialog;
}

namespace Interface {
    class FITKGeometryMeshSize;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    class GUIMeshDialogAPI LocalGroupDeleteDialog : public GUIDialogBase
    {
        Q_OBJECT;
    public:
        LocalGroupDeleteDialog(Interface::FITKGeometryMeshSize* obj, EventOper::ParaWidgetInterfaceOperator* oper);
        ~LocalGroupDeleteDialog();
    private slots:
        ;
        void on_pushButton_Cancel_clicked();

        void on_pushButton_OK_clicked();
    private:
        Ui::MeshDeleteDialog* _ui = nullptr;
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        Interface::FITKGeometryMeshSize* _obj = nullptr;
    };
}
#endif
