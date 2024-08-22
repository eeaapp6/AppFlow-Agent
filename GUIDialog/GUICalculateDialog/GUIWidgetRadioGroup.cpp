#include "GUIWidgetRadioGroup.h"
#include "ui_GUIWidgetRadioGroup.h"
#include "compCalLineWidget.h"

#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataRadioGroup.h"

#include <QButtonGroup>
#include <QRadioButton>
#include <QPair>

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
        QButtonGroup* group = new QButtonGroup(this);

        QList<Interface::FITKRadioGroupValue> radioValues = _data->getRadioValues();
        for (auto radioValue : radioValues) {
            QHBoxLayout* layout = new QHBoxLayout(this);

            //radio单选按钮添加
            QRadioButton*  radioButton = new QRadioButton(this);
            radioButton->setText(radioValue._name);
            layout->addWidget(radioButton);

            //单选选项数据添加
            Interface::FITKAbstractParameter* values = radioValue._value;
            if (values == nullptr)continue;
            for (auto v : values->getParameter()) {
                if (v == nullptr)continue;
                QWidget* widget = compCalLineWidget::DataSwitchToWidget(v, this);
                if (widget == nullptr)continue;
                layout->addWidget(widget);
            }
            _ui->verticalLayout_Value->addLayout(layout);
        }
        connect(group, SIGNAL(buttonClicked(int)), this, SLOT(slotRadioClicked(int)));
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
            widget->setObjectName(subData->getDataObjectName());
            _ui->verticalLayout_SubValue->addWidget(widget);
            _subWidget.append(widget);
        }
    }

    void GUIWidgetRadioGroup::slotRadioClicked(int index)
    {
        if (_data == nullptr)return;
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

