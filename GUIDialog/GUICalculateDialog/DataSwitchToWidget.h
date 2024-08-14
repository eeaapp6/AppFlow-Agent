/**
 * 
 * @file DataSwitchToWidget.h
 * @brief 求解器参数转换为界面 
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _DataSwitchToWidget_H
#define _DataSwitchToWidget_H

#include "GUICalculateDialogAPI.h"

class QWidget;
class QTableWidget;

namespace Interface {
    class FITKFlowDataBase;
    class FITKFlowDataGroup;
}

namespace GUI
{
    /**
     * @brief 数据类型转换为控件
     * @param[i]  data           数据对象
     * @param[i]  parent         父对象
     * @return GUICalculateDialogAPI* 
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    GUICalculateDialogAPI QWidget* DataSwitchToWidget(Interface::FITKFlowDataBase* data, QWidget* parent = nullptr);
    GUICalculateDialogAPI QTableWidget* DataGroupSwitchToWidget(QWidget* parent = nullptr);
    void DataGroupSwitchToWidget(QTableWidget* tableWidget, int& rowNum, int tierNum, Interface::FITKFlowDataGroup* dataGroup);
}

#endif
