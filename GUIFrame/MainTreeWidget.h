#ifndef _MainTreeWidget_H
#define _MainTreeWidget_H

#include "GUIFrameAPI.h"
#include "PanelWidgetBase.h"
#include "GUIFramEnum.h"

class QTreeWidget;

namespace GUI
{
    class MainWindow;

    enum class MainTreeEnum {
        MainTree_None = 0,
        MainTree_Geomety,
        MainTree_GeometyItem,
        MainTree_Mesh,
        MainTree_MeshItem,
    };

    class GUIFRAMEAPI MainTreeWidget : public PanelWidgetBase
    {
        Q_OBJECT;
    public:
        MainTreeWidget(MainWindow* parent);
        ~MainTreeWidget();

        void init();

        QTreeWidget* getTreeWidget();

    private:
        QTreeWidget* _treeWidget = nullptr;
    };
}

#endif