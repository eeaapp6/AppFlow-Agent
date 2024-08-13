#ifndef _GUIWidgetComBox_H
#define _GUIWidgetComBox_H

#include "GUICalculateDialogAPI.h"
#include <QComboBox>

namespace Interface {
    class FITKFlowDataBase;
    class FITKFlowDataCombox;
}

namespace GUI
{
    class GUICalculateDialogAPI GUIWidgetComBox :public QComboBox
    {
        Q_OBJECT;
    public:
        GUIWidgetComBox(Interface::FITKFlowDataBase* data, QWidget* parent = nullptr);
        ~GUIWidgetComBox();

        void init();
    private slots:
        ;
        void slotDataChange();
    protected:
        Interface::FITKFlowDataCombox* _value = nullptr;
    };
}

#endif
