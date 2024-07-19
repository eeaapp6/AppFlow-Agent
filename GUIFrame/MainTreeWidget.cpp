#include "MainTreeWidget.h"

#include "GUIWidget/TreeWidget.h"
#include <QTreeWidgetItem>

namespace GUI
{
    MainTreeWidget::MainTreeWidget(MainWindow* parent) :
        PanelWidgetBase(parent)
    {
        this->setTitle(tr("Tree"));

        _treeWidget = new TreeWidget(this);
        this->setWidget(_treeWidget);

        init();
    }

    MainTreeWidget::~MainTreeWidget()
    {
        if (_treeWidget)delete _treeWidget;
    }

    void MainTreeWidget::init()
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(QStringList{ "geometry" });
        
        _treeWidget->addTopLevelItem(item);
    }

    TreeWidget * MainTreeWidget::getTreeWidget()
    {
        return _treeWidget;
    }
}
