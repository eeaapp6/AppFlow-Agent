#include "GUIWidgetComBox.h"

#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataBase.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataCombox.h"

namespace GUI
{
    GUIWidgetComBox::GUIWidgetComBox(Interface::FITKFlowDataBase * data, QWidget * parent):
        QComboBox(parent)
    {
        _value = dynamic_cast<Interface::FITKFlowDataCombox*>(data);
        init();
        connect(this, SIGNAL(activated(int)), this, SLOT(slotDataChange()));
    }

    GUIWidgetComBox::~GUIWidgetComBox()
    {

    }

    void GUIWidgetComBox::init()
    {
        if (_value == nullptr)return;
        QStringList value = _value->getValue();
        int index = _value->getIndex();
        addItems(value);
        setCurrentIndex(index);
    }

    void GUIWidgetComBox::slotDataChange()
    {
        if (_value == nullptr)return;
        _value->setIndex(currentIndex());
    }
}
