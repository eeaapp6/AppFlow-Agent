#include "CompCalLineWidget.h"
#include "ui_CompCalLineWidget.h"

#include "GUIWidgetBool.h"
#include "GUIWidgetComBox.h"
#include "GUIWidgetDouble.h"
#include "GUIWidgetInt.h"
#include "GUIWidgetString.h"
#include "GUIWidgetRadioGroup.h"
#include "GUIWidgetBoolGroup.h"
#include "CompHBoxWidget.h"

#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataBase.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataCombox.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataInt.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataString.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataBool.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataDouble.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataRadioGroup.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataBoolGroup.h"

#include <QWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>
#include <QToolBox>

namespace GUI
{
    CompCalLineWidget::CompCalLineWidget(Interface::FITKFlowDataBase * data, QWidget * parent) :
        GUIWidgetBase(parent), _data(data)
    {
        _ui = new Ui::CompCalLineWidget();
        _ui->setupUi(this);

        init();
    }

    CompCalLineWidget::~CompCalLineWidget()
    {
        if (_ui)delete _ui;
    }

    void CompCalLineWidget::init()
    {
        if (_data == nullptr)return;
        QLabel* label = new QLabel(this);
        label->setText(_data->getDataObjectName());
        int width = this->width();
        label->setMinimumWidth(width*0.4);
        label->setMaximumWidth(width*0.4);
        _ui->horizontalLayout->addWidget(label);

        QWidget* widget = DataSwitchToWidget(_data, this);
        if (widget == nullptr)return;
        _ui->horizontalLayout->addWidget(widget);
    }

    QWidget* CompCalLineWidget::DataSwitchToWidget(Interface::FITKFlowDataBase * data, QWidget * parent, QString name)
    {
        QWidget* widget = nullptr;
        if (data == nullptr)return widget;
        Interface::FlowDataType type = data->getDataType();
        switch (type) {
        case Interface::FlowDataType::FLowDataInt: {
            auto w = new GUIWidgetInt(data, parent);
            widget = w;
            break;
        }
        case Interface::FlowDataType::FLowDataDouble: {
            auto w = new GUIWidgetDouble(data, parent);
            widget = w;
            break;
        }
        case Interface::FlowDataType::FLowDataString: {
            auto w = new GUIWidgetString(data, parent);
            widget = w;
            break;
        }
        case Interface::FlowDataType::FLowDataComBox: {
            auto w = new GUIWidgetComBox(data, parent);
            widget = w;
            break;
        }
        case Interface::FlowDataType::FLowDataBool: {
            auto w = new GUIWidgetBool(data, parent);
            w->setText(name);
            widget = w;
            break;
        }
        case Interface::FlowDataType::FLowDataRadioGroup: {
            auto w = new GUIWidgetRadioGroup(data, parent);
            widget = w;
            break;
        }case Interface::FlowDataType::FLowDataDoubleList: {
            auto doubleListData = dynamic_cast<Interface::FITKFlowDataDoubleList*>(data);
            if (doubleListData == nullptr)break;
            QList<QWidget*> widgets = {};
            for (auto d : doubleListData->getDoubleDatas()) {
                if (!d)continue;
                widgets.append(new GUIWidgetDouble(d, parent));
            }
            CompHBoxWidget* w = new CompHBoxWidget(widgets, parent);
            widget = w;
            break;
        }case Interface::FlowDataType::FLowDataBoolGroup: {
            auto boolGroupData = dynamic_cast<Interface::FITKFlowDataBoolGroup*>(data);
            if (boolGroupData == nullptr)break;
            GUIWidgetBoolGroup* w = new GUIWidgetBoolGroup(boolGroupData, parent);
            widget = w;
            break;
        }
        }
        return widget;
    }

    QToolBox * CompCalLineWidget::CreateToolBox(QWidget * parent)
    {
        QToolBox* toolBox = new QToolBox(parent);
        toolBox->setStyleSheet(
            "QToolBox::tab {"
            "    background-color: #d3d3d3;" /* 淡浅灰色背景 */
            "}"
            "QToolBox::tab:selected {"
            "    background-color: #a9a9a9;" /* 选中时的背景色，稍深的灰色 */
            "}"
        );

        return toolBox;
    }
}

