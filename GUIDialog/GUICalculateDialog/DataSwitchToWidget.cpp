#include "DataSwitchToWidget.h"

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
    DataGroupSwitchToWidget::DataGroupSwitchToWidget()
    {

    }

    DataGroupSwitchToWidget::~DataGroupSwitchToWidget()
    {

    }

    QTableWidget * DataGroupSwitchToWidget::dataToWidget(QTableWidget * widget, Interface::FITKAbstractParameter * dataBase, QWidget * parent)
    {
        int rowNum = 0;
        int colNum = 0;

        for (int i = 0; i < dataBase->getParameterCount(); i++) {
            Interface::FITKFlowDataBase* data = dataBase->getParameterAt(i);
            if (data == nullptr)continue;
            dataToWidget(widget, rowNum, colNum, data);
        }

        //for (int i = 0; i < dataBase->getGroupCount(); i++) {
        //    Interface::FITKFlowDataGroup* data = dataBase->getGroupAt(i);
        //    if (data == nullptr)continue;
        //    dataGroupToWidget(widget, rowNum, colNum, data);
        //}

        return widget;
    }

    QWidget* DataGroupSwitchToWidget::DataSwitchToWidget(Interface::FITKFlowDataBase * data, QWidget * parent)
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

    void DataGroupSwitchToWidget::dataGroupToWidget(QTableWidget* tableWidget, int& rowNum, int tierNum, Interface::FITKFlowDataGroup * dataGroup)
    {
        if (tableWidget == nullptr)return;
        if (dataGroup == nullptr)return;
        tierNum++;
        tableWidget->setColumnCount(tierNum + 2);

        for (int i = 0; i < dataGroup->getDataCount(); i++) {
            Interface::FITKFlowDataBase* data = dataGroup->getDataByIndex(i);
            if (data == nullptr)continue;
            QWidget* widget = DataSwitchToWidget(data, tableWidget);
            if (widget == nullptr)continue;
            QLabel* dataLabel = new QLabel(tableWidget);
            dataLabel->setText(data->getDataObjectName());
            if (widget == nullptr)return;

            tableWidget->setRowCount(rowNum + 1);
            tableWidget->setCellWidget(rowNum, tierNum, dataLabel);
            tableWidget->setCellWidget(rowNum, tierNum + 1, widget);
            rowNum++;

            switch (data->getDataType()) {
            case Interface::FlowDataType::FLowDataInt:break;
            case Interface::FlowDataType::FLowDataDouble:break;
            case Interface::FlowDataType::FLowDataString:break;
            case Interface::FlowDataType::FLowDataComBox: {
                Interface::FITKFlowDataCombox* comboxWidget = dynamic_cast<Interface::FITKFlowDataCombox*>(data);
                if (comboxWidget == nullptr)break;
                int index = comboxWidget->getIndex();
                QStringList valueList = comboxWidget->getValue();
                QString type = valueList[index];
                if (0 > index || index >= valueList.size())break;
                Interface::FITKFlowDataGroup* subDataGroup = comboxWidget->getValueGroup(type);
                dataGroupToWidget(tableWidget, rowNum, tierNum, subDataGroup);
                break;
            }
            case Interface::FlowDataType::FLowDataBool:break;
            }
        }
    }

    void DataGroupSwitchToWidget::dataToWidget(QTableWidget * tableWidget, int & rowNum, int tierNum, Interface::FITKFlowDataBase * data)
    {
        if (data == nullptr)return;

        tableWidget->setRowCount(rowNum + 1);
        tableWidget->setColumnCount(tierNum + 2);

        QWidget* widget = DataSwitchToWidget(data, tableWidget);
        QLabel* dataLabel = new QLabel(tableWidget);
        dataLabel->setText(data->getDataObjectName());
        if (widget == nullptr)return;
        tableWidget->setCellWidget(rowNum, tierNum, dataLabel);
        tableWidget->setCellWidget(rowNum, tierNum + 1, widget);
        rowNum++;

        switch (data->getDataType()) {
        case Interface::FlowDataType::FLowDataInt:break;
        case Interface::FlowDataType::FLowDataDouble:break;
        case Interface::FlowDataType::FLowDataString:break;
        case Interface::FlowDataType::FLowDataComBox: {
            Interface::FITKFlowDataCombox* comboxWidget = dynamic_cast<Interface::FITKFlowDataCombox*>(data);
            if (comboxWidget == nullptr)break;
            int index = comboxWidget->getIndex();
            QStringList valueList = comboxWidget->getValue();
            if (0 > index || index >= valueList.size())break;
            QString type = valueList[index];
            Interface::FITKFlowDataGroup* subDataGroup = comboxWidget->getValueGroup(type);
            dataGroupToWidget(tableWidget, rowNum, tierNum, subDataGroup);
            break;
        }
        case Interface::FlowDataType::FLowDataBool:break;
        }
    }
}