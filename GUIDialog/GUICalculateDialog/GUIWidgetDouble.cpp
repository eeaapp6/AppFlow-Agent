#include "GUIWidgetDouble.h"

#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataBase.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataDouble.h"

namespace GUI
{
    GUIWidgetDouble::GUIWidgetDouble(Interface::FITKFlowDataBase * data, QWidget * parent):
        QDoubleSpinBox(parent), GUICalculateSubWidgetBase(parent)
    {
        _value = dynamic_cast<Interface::FITKFlowDataDouble*>(data);
        init();
        connect(this, SIGNAL(valueChanged(double)), this, SLOT(slotDataChange()));
    }

    GUIWidgetDouble::~GUIWidgetDouble()
    {

    }

    void GUIWidgetDouble::init()
    {
        if (_value == nullptr)return;
        double value = _value->getValue();
        double range[2] = { 0,0 };
        _value->getRange(range);

        this->setRange(range[0], range[1]);
        this->setValue(value);
    }

    void GUIWidgetDouble::wheelEvent(QWheelEvent * event)
    {
        Q_UNUSED(event);
    }

    void GUIWidgetDouble::slotDataChange()
    {
        if (_value == nullptr)return;
        _value->setValue(value());
    }
}
