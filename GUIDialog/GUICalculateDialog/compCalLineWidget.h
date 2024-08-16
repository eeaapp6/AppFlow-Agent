#ifndef _compCalLineWidget_H
#define _compCalLineWidget_H

#include "GUICalculateDialogAPI.h"
#include "GUIWidget/GUIWidgetBase.h"

namespace Ui {
    class compCalLineWidget;
}

namespace Interface {
    class FITKFlowDataBase;
}

namespace GUI
{
    class GUICalculateDialogAPI compCalLineWidget : public GUIWidgetBase
    {
        Q_OBJECT;
    public:
        compCalLineWidget(Interface::FITKFlowDataBase* data, QWidget* parent = nullptr);
        ~compCalLineWidget();
        void init();
        /**
         * @brief 数据类型转换为控件
         * @param[i]  data           数据对象
         * @param[i]  parent         父对象
         * @return GUICalculateDialogAPI*
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        static QWidget* DataSwitchToWidget(Interface::FITKFlowDataBase* data, QWidget* parent = nullptr);
    private:
        Ui::compCalLineWidget* _ui = nullptr;
        Interface::FITKFlowDataBase* _data = nullptr;
    };
}

#endif
