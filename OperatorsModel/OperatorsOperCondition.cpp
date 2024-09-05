#include "OperatorsOperCondition.h"
#include "GUIFrame/PropertyWidget.h"

#include "GUIDialog/GUICalculateDialog/OperatingConditionWidget.h"

namespace ModelOper
{
    OperatorsOperCondition::OperatorsOperCondition()
    {

    }

    OperatorsOperCondition::~OperatorsOperCondition()
    {

    }

    bool OperatorsOperCondition::execGUI()
    {
        if (_mainWindow == nullptr)return false;
        GUI::PropertyWidget* propertyWidget = _mainWindow->getPropertyWidget();
        if (propertyWidget == nullptr)return false;

        GUI::OperatingConditionWidget* widget = new GUI::OperatingConditionWidget(this, propertyWidget);
        propertyWidget->setWidget(widget);
        return true;
    }

    bool OperatorsOperCondition::execProfession()
    {
        return true;
    }
}
