#ifndef _GUIWidgetRadioGroup_H
#define _GUIWidgetRadioGroup_H

#include "GUICalculateDialogAPI.h"
#include <QWidget>

class QButtonGroup;
class QHBoxLayout;

namespace Ui {
    class GUIWidgetRadioGroup;
}

namespace Interface {
    class FITKFlowDataBase;
    class FITKFlowDataRadioGroup;
}

namespace GUI
{
    class CompHBoxWidget;

    class GUICalculateDialogAPI GUIWidgetRadioGroup : public QWidget
    {
        Q_OBJECT;
    public:
        GUIWidgetRadioGroup(Interface::FITKFlowDataBase* dataBase, QWidget* parent = nullptr);
        ~GUIWidgetRadioGroup();

        void init();
    private:
        void initRadioData();
        void initSubData();
        /**
         * @brief    禁用Radio界面
         * @return   void
         * @author   BaGuijun (baguijun@163.com)
         * @date     2024-08-22
         */
        void disableAllRadioWidget();
    private slots:
        ;
        void slotRadioClicked(int index);
    private:
        Ui::GUIWidgetRadioGroup* _ui = nullptr;
        Interface::FITKFlowDataRadioGroup* _data = nullptr;

        QList<QWidget*> _subWidget = {};

        QButtonGroup* _group = nullptr;

        QHash<int, CompHBoxWidget*> _radioWidgets = {};
    };
}

#endif
