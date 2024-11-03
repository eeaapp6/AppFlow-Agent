/**
 * @file   OperModelTreeMouseEvent.h
 * @brief  定义模型树点击事件处理器
 */
#ifndef _OPERMODOELTREEEVENTMOUDE_H__
#define _OPERMODOELTREEEVENTMOUDE_H__

#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "OperatorsInterface/TreeEventOperator.h"
#include "OperatorsGUIAPI.h"

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

        virtual void moveProcessToStep(int step, void *addInfo = nullptr) override;
    };

    Register2FITKOperatorRepo(ModelTreeEvent, OperModelTreeMouseEvent);

}  // namespace GUIOper

#endif
