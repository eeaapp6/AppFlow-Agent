#ifndef _GUIWidgetBool_H
#define _GUIWidgetBool_H

#include "GUICalculateDialogAPI.h"
#include <QCheckBox>

namespace Interface {
    class FITKFlowDataBase;
    class FITKFlowDataBool;
}

namespace GUI
{
    class GUICalculateDialogAPI GUIWidgetBool :public QCheckBox
    {
        Q_OBJECT;
    public:
        GUIWidgetBool(Interface::FITKFlowDataBase* data, QWidget* parent = nullptr);
        ~GUIWidgetBool();

        void init();
    private slots:
        ;
        void slotDataChange();
    protected:
        Interface::FITKFlowDataBool* _value = nullptr;
    };
}

#endif
