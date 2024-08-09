#ifndef _CompTreeItem_H
#define _CompTreeItem_H

#include "FITK_Kernel/FITKCore/FITKAbstractGUI.h"
#include "GUIWidgetAPI.h"

class QTreeWidgetItem;

namespace Ui {
    class CompTreeItem;
}

namespace GUI
{
    class GUIWIDGETAPI CompTreeItem : public Core::FITKWidget
    {
        Q_OBJECT;
    public:
        CompTreeItem(QTreeWidgetItem* itme, QWidget* parent);
        ~CompTreeItem();

        void init();

        void setButtonIcon(QIcon icon);

        void setText(QString text);

        QTreeWidgetItem* getTreeItem();
    signals:
        void sigIconButtonClicked();
    private:
        QTreeWidgetItem* _item = nullptr;
        Ui::CompTreeItem* _ui = nullptr;
    };
}

#endif
