#ifndef _GUIWidgetDouble_H
#define _GUIWidgetDouble_H

#include "GUICalculateDialogAPI.h"
#include <QDoubleSpinBox>

namespace Interface {
    class FITKFlowDataBase;
    class FITKFlowDataDouble;
}

namespace GUI
{
    class GUICalculateDialogAPI GUIWidgetDouble :public QDoubleSpinBox
    {
        Q_OBJECT;
    public:
        GUIWidgetDouble(Interface::FITKFlowDataBase* data, QWidget* parent = nullptr);
        ~GUIWidgetDouble();

        void init();
    private slots:
        ;
        void slotDataChange();
    protected:
        Interface::FITKFlowDataDouble* _value = nullptr;
    };
}

#endif
