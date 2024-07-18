/**
 * @file   OperModelTreeMouseEvent.h
 * @brief  定义模型树点击事件处理器
 */
#ifndef _OPERMODOELTREEEVENTMOUDE_H__
#define _OPERMODOELTREEEVENTMOUDE_H__

#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include <QAction>
#include "OperatorsInterface/TreeEventOperator.h"
#include "OperatorsGUIAPI.h"

class QTreeWidget;
class QTreeWidgetItem;

namespace GUI {
    class ModelCaseTree;
}
namespace GUIOper
{
    /**
     * @brief 模型树点击事件处理器
     */
    class OperatorsGUIAPI OperModelTreeMouseEvent : public EventOper::TreeEventOperator
    {
        Q_OBJECT;
    public:
        OperModelTreeMouseEvent();
        ~OperModelTreeMouseEvent();
        /**
         * @brief  更新整棵树
         */
        void updateTree() override;
        /**
         * @brief 鼠标单击事件
         * @param  item 当前操作节点
         */
    private slots:
        ;
        void onItemClicked(QTreeWidgetItem *item, int column);
        /**
         * @brief 鼠标双击事件
         * @param  item 当前操作节点
         */
        void onDoubleClicked(QTreeWidgetItem *item, int column);

    private:
        QTreeWidget* _treeWidget = nullptr;
    };

    Register2FITKOPeratorRepo(ModelTreeEvent, OperModelTreeMouseEvent);

}  // namespace GUIOper

#endif
