#include "GUIWidgetDouble.h"

#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataBase.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataDouble.h"

namespace GUI
{
    GUIWidgetDouble::GUIWidgetDouble(Interface::FITKFlowDataBase * data, QWidget * parent):
        Comp::FITKSciNotationLineEdit(parent), GUICalculateSubWidgetBase(parent)
    {
        _value = dynamic_cast<Interface::FITKFlowDataDouble*>(data);
        init();
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
        this->setRange(range);
        this->setCurrentValidValue(value);

        connect(this, SIGNAL(dataChanged()), this, SLOT(slotDataChangeFinished()));
    }

    void GUIWidgetDouble::wheelEvent(QWheelEvent * event)
    {
        Q_UNUSED(event);
    }

    void GUIWidgetDouble::slotDataChangeFinished()
    {
        if (_value == nullptr)return;

        double value = 0.0;
        getCurrentValidValue(value);
        _value->setValue(value);
    }
}
