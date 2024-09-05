#include "GUIWidgetRadioGroup.h"
#include "ui_GUIWidgetRadioGroup.h"
#include "CompCalLineWidget.h"
#include "CompHBoxWidget.h"

#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataRadioGroup.h"

#include <QButtonGroup>
#include <QRadioButton>
#include <QPair>
#include <QFrame>
#include <QPushButton>

namespace GUI
{
    GUIWidgetRadioGroup::GUIWidgetRadioGroup(Interface::FITKFlowDataBase * dataBase, QWidget * parent) :
        QWidget(parent)
    {
        _data = dynamic_cast<Interface::FITKFlowDataRadioGroup*>(dataBase);
        if (_data == nullptr)return;
        _ui = new Ui::GUIWidgetRadioGroup();
        _ui->setupUi(this);
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

        //设置默认选项
        int currentindex = _data->getCurrentValueIndex();
        auto radioButton = _group->button(currentindex);
        if (radioButton == nullptr)return;
        radioButton->setChecked(true);
        slotRadioClicked(currentindex);
    }

    void GUIWidgetRadioGroup::initRadioData()
    {
        if (_data == nullptr)return;
        _group = new QButtonGroup(this);

        QList<Interface::FITKRadioGroupValue> radioValues = _data->getRadioValues();
        if (radioValues.size() == 0)return;

        for (int i = 0; i < radioValues.size(); i++) {            
            auto radioValue = radioValues[i];
            //单选选项数据添加
            Interface::FITKAbstractParameter* values = radioValue._value;
            QList<QWidget*> widgetList = {};

            //radio单选按钮添加
            QRadioButton* radioButton = new QRadioButton(this);
            radioButton->setText(radioValue._name);
            widgetList.append(radioButton);
            _group->addButton(radioButton, i);

            //子数据添加
            if (values) {
                if (values->getParameterCount() > 0) {
                    int width = this->width();
                    radioButton->setMinimumWidth(width*0.4);
                    radioButton->setMaximumWidth(width*0.4);
                }
                for (auto v : values->getParameter()) {
                    if (v == nullptr)continue;
                    QWidget* widget = CompCalLineWidget::DataSwitchToWidget(v, this);
                    if (widget == nullptr)continue;
                    widgetList.append(widget);
                }
            }
            CompHBoxWidget* subWidget = new CompHBoxWidget(widgetList, this);
            _radioWidgets.insert(i, subWidget);
            _ui->verticalLayout->addWidget(subWidget);
        }

        //禁用
        disableAllRadioWidget();
        connect(_group, SIGNAL(buttonClicked(int)), this, SLOT(slotRadioClicked(int)));
    }

    void GUIWidgetRadioGroup::initSubData()
    {
        if (_data == nullptr)return;

        auto subDataList = _data->getSubValues();
        if (subDataList.size() == 0)return;

        QFrame* line = new QFrame(this);
        line->setFrameShape(QFrame::HLine);
        _ui->verticalLayout->addWidget(line);

        for (auto subData : subDataList) {
            if (subData == nullptr)continue;
            auto subDataType = subData->getDataType();
            QWidget* widget = nullptr;
            switch (subDataType){
            case Interface::FlowDataType::FLowDataInt:
            case Interface::FlowDataType::FLowDataDouble:
            case Interface::FlowDataType::FLowDataString:
            case Interface::FlowDataType::FLowDataComBox:
            case Interface::FlowDataType::FLowDataDoubleList: {
               widget = new CompCalLineWidget(subData, this);
                break;
            }
            case Interface::FlowDataType::FLowDataBool: {
                widget = CompCalLineWidget::DataSwitchToWidget(subData, this, subData->getDataObjectName());
                break;
            }
            }
            if (widget == nullptr)continue;
            _ui->verticalLayout->addWidget(widget);
            _subWidget.append(widget);
        }
    }

    void GUIWidgetRadioGroup::disableAllRadioWidget()
    {
        for (auto w : _radioWidgets.values()) {
            if (w == nullptr)continue;
            w->setSubWidgetEnable(false);
        }
    }

    void GUIWidgetRadioGroup::slotRadioClicked(int index)
    {
        if (index == -1)return;
        if (_data == nullptr)return;

        //Radio禁用
        disableAllRadioWidget();
        auto w = _radioWidgets.value(index);
        if (w) {
            w->setSubWidgetEnable(true);
        }

        //sub禁用
        _data->setCurrentIndex(index);
        QList<bool> subEnable = _data->getCurrentValueSubState();
        if (subEnable.size() != _subWidget.size())return;

        for (int i = 0; i < subEnable.size(); i++) {
            if (_subWidget[i] == nullptr)continue;
            if (subEnable[i] == true) {
                _subWidget[i]->setEnabled(true);
            }
            else {
                _subWidget[i]->setEnabled(false);
            }
        }
    }
}

