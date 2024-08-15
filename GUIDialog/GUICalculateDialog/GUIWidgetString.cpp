#include "GUIWidgetString.h"

#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataBase.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataString.h"

namespace GUI
{
    GUIWidgetString::GUIWidgetString(Interface::FITKFlowDataBase * data, QWidget * parent) :
        QLineEdit(parent), GUICalculateSubWidgetBase(parent)
    {
        _value = dynamic_cast<Interface::FITKFlowDataString*>(data);
        init();
        connect(this, SIGNAL(textChanged(QString)), this, SLOT(slotDataChange()));
    }

    GUIWidgetString::~GUIWidgetString()
    {

    }

    void GUIWidgetString::init()
    {
        if (_value == nullptr)return;
        QString value = _value->getValue();

        this->setText(value);
    }

    void GUIWidgetString::slotDataChange()
    {
        if (_value == nullptr)return;
        _value->setValue(text());
    }
}

