#include "GUIWidgetComBox.h"
#include "GUICalculateWidgetBase.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataBase.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataCombox.h"

#include <QTableWidget>

namespace GUI
{
    GUIWidgetComBox::GUIWidgetComBox(Interface::FITKFlowDataBase * data, QWidget * parent):
        QComboBox(parent), GUICalculateSubWidgetBase(parent)
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

    void GUIWidgetComBox::wheelEvent(QWheelEvent * event)
    {
        Q_UNUSED(event);
    }

    void GUIWidgetComBox::slotDataChange()
    {
        if (_value == nullptr)return;
        _value->setIndex(currentIndex());
    }
}
