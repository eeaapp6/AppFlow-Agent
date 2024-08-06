/**
 * @file OperManagerBase.h
 * @brief 操作器基类
 */
#ifndef __OPERMANAGERBASE_H__
#define __OPERMANAGERBASE_H__

#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "OperatorsInterface/ParaWidgetInterfaceOperator.h"
#include "GUIFrame/MainWindow.h"
#include "OperatorsModelAPI.h"
#include <QStringList>

namespace ModelOper
{
    /**
     * @brief 操作器基类
     */
    class OperatorsModelAPI OperManagerBase : public EventOper::ParaWidgetInterfaceOperator
    {
        Q_OBJECT
    public:
        OperManagerBase();
        virtual ~OperManagerBase();
    protected:
        /**
         * @brief 参数预处理
         */
        void preArgs() override;
        /**
         * @brief 操作类型
         */
        enum OperType
        {
            None,       ///< 未定义
            Create,     ///< 创建操作
            Edit,       ///< 编辑操作
            Copy,       ///< 拷贝操作
            Delete,     ///< 删除操作
            Rename,     ///< 重命名操作
            Select,     ///< 选择操作
        } _operType{ None };
        /**
         * @brief 主界面
         */
        GUI::MainWindow* _mainWindow = nullptr;

        QString _senderName = "";
    };
} // namespace ModelOper
#endif //!__OPERMANAGERBASE_H__
