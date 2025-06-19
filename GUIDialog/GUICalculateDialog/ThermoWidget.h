/**********************************************************************
 * @file   ThermoWidget.h
 * @brief  热属性界面
 * @author liuzhonghua (liuzhonghuaszch@163.com)
 * @date   2025-06-18
 *********************************************************************/
#ifndef _ThermoWidget_H
#define _ThermoWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUICalculateWidgetBase.h"

class QAbstractButton;

namespace Ui {
    class ThermoWidget;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace Interface {
    class FITKOFThermo;
}

namespace GUI
{
    /**
     * @brief  求解器运行界面
     * @author liuzhonghua (liuzhonghuaszch@163.com)
     * @date   2024-09-02
     */
    class GUICalculateDialogAPI ThermoWidget : public GUICalculateWidgetBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief    构造函数
         * @param[i] oper      操作器对象
         * @param[i] parent    父对象
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2024-09-02
         */
        ThermoWidget(EventOper::ParaWidgetInterfaceOperator* oper, QWidget* parent = nullptr);
        /**
         * @brief    析构函数
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2024-09-02
         */
        ~ThermoWidget();
        /**
         * @brief    初始化
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2024-09-02
         */
        void init();
        /**
         * @brief    显示事件
         * @param[i] event   事件对象
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2024-09-02
         */
        void showEvent(QShowEvent * event);
        /**
         * @brief    隐藏事件
         * @param[i] event   事件对象
         * @author   liuzhonghua (liuzhonghuaszch@163.com)
         * @date     2024-09-02
         */
        void resizeEvent(QResizeEvent * event);
    private:
        /**
         * @brief  UI对象
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date   2024-09-02
         */
        Ui::ThermoWidget* _ui = nullptr;
        /**
         * @brief  求解器运行参数对象
         * @author liuzhonghua (liuzhonghuaszch@163.com)
         * @date   2024-09-02
         */
        Interface::FITKOFThermo* _thermoObj = nullptr;
    };
}

#endif
