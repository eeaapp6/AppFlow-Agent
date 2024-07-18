#ifndef _MainTreeWidget_H
#define _MainTreeWidget_H

#include "GUIFrameAPI.h"
#include "PanelWidgetBase.h"
#include "GUIFramEnum.h"

class QTreeWidget;

namespace GUI
{
    class MainWindow;

    class GUIFRAMEAPI MainTreeWidget : public PanelWidgetBase
    {
        Q_OBJECT;
    public:
        MainTreeWidget(MainWindow* parent);
        ~MainTreeWidget();

        void init();

        void updateWidget();
    private:
        QTreeWidget* _treeWidget = nullptr;
    };
}

#endif