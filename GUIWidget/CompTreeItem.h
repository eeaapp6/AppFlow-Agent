/**
 * 
 * @file CompTreeItem.h
 * @brief 树item组件
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-16
 * 
 */
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
    /**
     * @brief 树item组件
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-16
     */
    class GUIWIDGETAPI CompTreeItem : public Core::FITKWidget
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Comp Tree Item object
         * @param[i]  itme           树item对象
         * @param[i]  parent         父对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-16
         */
        CompTreeItem(QTreeWidgetItem* itme, QWidget* parent);
        /**
         * @brief Destroy the Comp Tree Item object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-16
         */
        ~CompTreeItem();
        /**
         * @brief 初始化
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-16
         */
        void init();
        /**
         * @brief 设置图标
         * @param[i]  icon           图标对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-16
         */
        void setButtonIcon(QIcon icon);
        /**
         * @brief 设置文本
         * @param[i]  text           文本
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-16
         */
        void setText(QString text);
        /**
         * @brief 回去当前组件对应的树item对象
         * @return QTreeWidgetItem* 树item对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-16
         */
        QTreeWidgetItem* getTreeItem();
    signals:
        ;
        /**
         * @brief 图标点击事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-16
         */
        void sigIconButtonClicked();
    private:
        /**
         * @brief 树item对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-16
         */
        QTreeWidgetItem* _item = nullptr;
        /**
         * @brief ui对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-16
         */
        Ui::CompTreeItem* _ui = nullptr;
    };
}

#endif
