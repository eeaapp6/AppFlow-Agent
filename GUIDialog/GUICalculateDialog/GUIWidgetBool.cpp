#include "GUIWidgetBool.h"

#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataBase.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataBool.h"

namespace GUI
{
    GUIWidgetBool::GUIWidgetBool(Interface::FITKFlowDataBase * data, QWidget * parent) :
        QCheckBox(parent), GUICalculateSubWidgetBase(parent)
    {
        _value = dynamic_cast<Interface::FITKFlowDataBool*>(data);

        init();

        connect(this, SIGNAL(stateChanged(int)), this, SLOT(slotDataChange()));
    }

    GUIWidgetBool::~GUIWidgetBool()
    {

    }

    void GUIWidgetBool::init()
    {
        if (_value == nullptr)return;
        setChecked(_value->getValue());
    }

    void GUIWidgetBool::slotDataChange()
    {
        if (_value == nullptr)return;
        _value->setValue(isChecked());
    }
}

