#include "GUIWidgetRadioGroup.h"
#include "ui_GUIWidgetRadioGroup.h"
#include "compCalLineWidget.h"

#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataRadioGroup.h"

namespace GUI
{
    GUIWidgetRadioGroup::GUIWidgetRadioGroup(Interface::FITKFlowDataBase * dataBase, QWidget * parent) :
        QWidget(parent)
    {
        _data = dynamic_cast<Interface::FITKFlowDataRadioGroup*>(dataBase);
        if (_data == nullptr)return;
        _ui = new Ui::GUIWidgetRadioGroup();
        _ui->setupUi(this);

        _ui->line->hide();
        init();
    }

    GUIWidgetRadioGroup::~GUIWidgetRadioGroup()
    {

    }

    void GUIWidgetRadioGroup::init()
    {
        if (_data == nullptr)return;
        initRadioData();
        initSubData();
    }

    void GUIWidgetRadioGroup::initRadioData()
    {
        if (_data == nullptr)return;
        QList<QPair<QString, QList<Interface::FITKFlowDataBase*>>> radioValues = _data->getRadioValues();
        for (auto radioValue : radioValues) {

        }
    }

    void GUIWidgetRadioGroup::initSubData()
    {
        if (_data == nullptr)return;

        auto subDataList = _data->getSubValues();
        if (subDataList.size() == 0)return;
        _ui->line->show();

        for (auto subData : subDataList) {
          QWidget* widget = compCalLineWidget::DataSwitchToWidget(subData, this);
          if (widget == nullptr)continue;
          _ui->verticalLayout_SubValue->addWidget(widget);
        }
    }
}

