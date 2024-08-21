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
        //if (_data == nullptr)return;
        //QButtonGroup* group = new QButtonGroup(this);

        //QList<QPair<QString, QList<Interface::FITKFlowDataBase*>>> radioValues = _data->getRadioValues();
        //for (auto radioValue : radioValues) {
        //    QHBoxLayout* layout = new QHBoxLayout(this);

        //    QRadioButton*  radioButton = new QRadioButton(this);
        //    radioButton->setText(radioValue.first);
        //    layout->addWidget(radioButton);

        //    QList<Interface::FITKFlowDataBase*> values = radioValue.second;
        //    for (auto v : values) {
        //        if (v == nullptr)continue;
        //        QWidget* widget = compCalLineWidget::DataSwitchToWidget(v, this);
        //        if (widget == nullptr)continue;
        //        layout->addWidget(widget);
        //    }
        //    _ui->verticalLayout_Value->addLayout(layout);
        //}
        //connect(group, SIGNAL(buttonClicked(int)), this, SLOT(slotRadioClicked(int)));
    }

    void GUIWidgetRadioGroup::initSubData()
    {
        //if (_data == nullptr)return;

        //auto subDataList = _data->getSubValues();
        //if (subDataList.size() == 0)return;
        //_ui->line->show();

        //for (auto subData : subDataList) {
        //  QWidget* widget = compCalLineWidget::DataSwitchToWidget(subData, this);
        //  if (widget == nullptr)continue;
        //  widget->setObjectName(subData->getDataObjectName());
        //  _ui->verticalLayout_SubValue->addWidget(widget);
        //}
    }

    void GUIWidgetRadioGroup::slotRadioClicked(int index)
    {
        if (_data == nullptr);
    }
}

