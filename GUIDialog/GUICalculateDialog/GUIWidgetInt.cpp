#include "GUIWidgetInt.h"

#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataBase.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataInt.h"

namespace GUI
{
    GUIWidgetInt::GUIWidgetInt(Interface::FITKFlowDataBase * data, QWidget * parent):
        QSpinBox(parent), GUICalculateSubWidgetBase(parent)
    {
        _value = dynamic_cast<Interface::FITKFlowDataInt*>(data);
        init();
        connect(this, SIGNAL(valueChanged(int)), this, SLOT(slotDataChange()));
    }

    GUIWidgetInt::~GUIWidgetInt()
    {

    }

    void GUIWidgetInt::init()
    {
        if (_value == nullptr)return;
        int value = _value->getValue();
        int range[2] = { 0,0 };
        _value->getRange(range);

        this->setRange(range[0], range[1]);
        this->setValue(value);
    }

    void GUIWidgetInt::wheelEvent(QWheelEvent * event)
    {
        Q_UNUSED(event);
    }

    void GUIWidgetInt::slotDataChange()
    {
        if (_value == nullptr)return;
        _value->setValue(value());
    }
}
