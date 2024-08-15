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
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractParameter.h"

#include <QObject>

class QWidget;
class QTableWidget;

namespace Interface {
    class FITKFlowDataBase;
    class FITKFlowDataGroup;
    //class FITKAbstractParameter;
}

namespace GUI
{
    class GUICalculateDialogAPI DataGroupSwitchToWidget :public QObject
    {
    public:
        DataGroupSwitchToWidget();
        ~DataGroupSwitchToWidget();

        QTableWidget* dataToWidget(QTableWidget* widget, Interface::FITKAbstractParameter* dataBase, QWidget* parent = nullptr);
    private:
        /**
         * @brief 数据类型转换为控件
         * @param[i]  data           数据对象
         * @param[i]  parent         父对象
         * @return GUICalculateDialogAPI*
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        QWidget* DataSwitchToWidget(Interface::FITKFlowDataBase* data, QWidget* parent = nullptr);

        void dataGroupToWidget(QTableWidget* tableWidget, int& rowNum, int tierNum, Interface::FITKFlowDataGroup* dataGroup);

        void dataToWidget(QTableWidget* tableWidget, int& rowNum, int tierNum, Interface::FITKFlowDataBase* data);
    };
}

#endif
