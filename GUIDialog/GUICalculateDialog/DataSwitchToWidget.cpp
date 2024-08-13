#include "DataSwitchToWidget.h"

#include "GUIWidgetBool.h"
#include "GUIWidgetComBox.h"
#include "GUIWidgetDouble.h"
#include "GUIWidgetInt.h"
#include "GUIWidgetString.h"

#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowDataBase.h"

#include <QWidget>
#include <QTableWidget>
#include <QHeaderView>

namespace GUI
{
    QWidget* DataSwitchToWidget(Interface::FITKFlowDataBase * data, QWidget * parent)
    {
        QWidget* widget = nullptr;
        if (data == nullptr)return widget;
        Interface::FlowDataType type = data->getDataType();
        switch (type){
        case Interface::FlowDataType::FLowDataInt:widget = new GUIWidgetInt(data, parent); break;
        case Interface::FlowDataType::FLowDataDouble:widget = new GUIWidgetDouble(data, parent); break;
        case Interface::FlowDataType::FLowDataString:widget = new GUIWidgetString(data, parent); break;
        case Interface::FlowDataType::FLowDataComBox:widget = new GUIWidgetComBox(data, parent); break;
        case Interface::FlowDataType::FLowDataBool:widget = new GUIWidgetBool(data, parent); break;
        }
        return widget;
    }

    QTableWidget* DataGroupSwitchToWidget(QWidget* parent) 
    {
        QTableWidget* widget = new QTableWidget(parent);
        //自适应布局（更具表头数量平均分配大小）
        widget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        //设置tableWidget的item不可编辑
        widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        //隐藏行表头
        widget->verticalHeader()->setVisible(false);
        //隐藏列表头
        widget->horizontalHeader()->setVisible(false);
        //隐藏网格线
        widget->setShowGrid(false);

        int rowNum = 0;
        int colNum = 0;

        return widget;
    }
}


