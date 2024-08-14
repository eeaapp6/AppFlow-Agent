/**
 * 
 * @file LocalGroupDeleteDialog.h
 * @brief 网格尺寸对象删除对话框
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-08-14
 * 
 */
#ifndef _LocalGroupDeleteDialog_H
#define _LocalGroupDeleteDialog_H

#include "GUIWidget/GUIDialogBase.h"
#include "GUIMeshDialogAPI.h"

namespace Ui {
    class MeshDeleteDialog;
}

namespace Interface {
    class FITKGeometryMeshSize;
}

namespace EventOper {
    class ParaWidgetInterfaceOperator;
}

namespace GUI
{
    /**
     * @brief 网格尺寸对象删除对话框
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-08-14
     */
    class GUIMeshDialogAPI LocalGroupDeleteDialog : public GUIDialogBase
    {
        Q_OBJECT;
    public:
        /**
         * @brief Construct a new Local Group Delete Dialog object
         * @param[i]  obj            网格尺寸对象
         * @param[i]  oper           操作器
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        LocalGroupDeleteDialog(Interface::FITKGeometryMeshSize* obj, EventOper::ParaWidgetInterfaceOperator* oper);
        /**
         * @brief Destroy the Local Group Delete Dialog object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        ~LocalGroupDeleteDialog();
    private slots:
        ;
        /**
         * @brief 取消按钮点击事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_pushButton_Cancel_clicked();
        /**
         * @brief ok按钮点击事件
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        void on_pushButton_OK_clicked();
    private:
        /**
         * @brief ui对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Ui::MeshDeleteDialog* _ui = nullptr;
        /**
         * @brief 操作器对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        EventOper::ParaWidgetInterfaceOperator* _oper = nullptr;
        /**
         * @brief 网格尺寸对象
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        Interface::FITKGeometryMeshSize* _obj = nullptr;
    };
}
#endif
