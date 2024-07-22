/**
 * @file   TreeWidget.h
 * @brief  定义模型树点击事件处理器
 */
#ifndef _TreeWidget_H__
#define _TreeWidget_H__

#include <QTreeWidget>
#include "GUIWidgetAPI.h"

class QTreeWidget;
class QTreeWidgetItem;

namespace GUI {

    class ModelCaseTree;
    
    /**
     * @brief 模型树点击事件处理器
     */
    class GUIWIDGETAPI TreeWidget : public QTreeWidget
    {
        Q_OBJECT;
    public:
        TreeWidget(QWidget* parent);
        ~TreeWidget();
        /**
         * @brief  更新整棵树
         */
        void updateTree();
    public slots:
        ;
        /**
         * @brief 鼠标右键事件
         * @param  item 当前操作节点
         */
        void onModelCustomContextMenu(QPoint point);
    private slots:
        ;
        /**
         * @brief 鼠标单击事件
         * @param  item 当前操作节点
         */
        void onItemClicked(QTreeWidgetItem *item, int column);
        /**
         * @brief 鼠标双击事件
         * @param  item 当前操作节点
         */
        void onDoubleClicked(QTreeWidgetItem *item, int column);


        void acitonClicked();
    private:
        void updateGeometryItems();
        void updateMeshItems();
        //添加actions
        void addMenuActions(QMenu& menu, QString actions, QString objectName);
    };
}  

#endif
