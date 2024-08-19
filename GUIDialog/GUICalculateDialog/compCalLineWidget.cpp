#include "compCalLineWidget.h"
#include "ui_compCalLineWidget.h"

#include "GUIWidgetBool.h"
#include "GUIWidgetComBox.h"
#include "GUIWidgetDouble.h"
#include "GUIWidgetInt.h"
#include "GUIWidgetString.h"

#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataBase.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataCombox.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataInt.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataString.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataBool.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataDouble.h"

#include <QWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QLabel>

namespace GUI
{
    compCalLineWidget::compCalLineWidget(Interface::FITKFlowDataBase * data, QWidget * parent) :
        GUIWidgetBase(parent), _data(data)
    {
        _ui = new Ui::compCalLineWidget();
        _ui->setupUi(this);

        init();
    }

    compCalLineWidget::~compCalLineWidget()
    {
        if (_ui)delete _ui;
    }

    void compCalLineWidget::init()
    {
        if (_data == nullptr)return;
        QLabel* label = new QLabel(this);
        label->setText(_data->getDataObjectName());
        _ui->horizontalLayout->addWidget(label);

        QWidget* widget = DataSwitchToWidget(_data, this);
        if (widget == nullptr);
        _ui->horizontalLayout->addWidget(widget);
    }

    QWidget* compCalLineWidget::DataSwitchToWidget(Interface::FITKFlowDataBase * data, QWidget * parent)
    {
        QWidget* widget = nullptr;
        if (data == nullptr)return widget;
        Interface::FlowDataType type = data->getDataType();
        switch (type) {
        case Interface::FlowDataType::FLowDataInt:widget = new GUIWidgetInt(data, parent); break;
        case Interface::FlowDataType::FLowDataDouble:widget = new GUIWidgetDouble(data, parent); break;
        case Interface::FlowDataType::FLowDataString:widget = new GUIWidgetString(data, parent); break;
        case Interface::FlowDataType::FLowDataComBox:widget = new GUIWidgetComBox(data, parent); break;
        case Interface::FlowDataType::FLowDataBool:widget = new GUIWidgetBool(data, parent); break;
        }
        return widget;
    }
}

