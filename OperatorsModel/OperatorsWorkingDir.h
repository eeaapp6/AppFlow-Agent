/**
 *
 * @file OperatorsWorkingDir.h
 * @brief 设置工作路径
 * @author BaGuijun (baguijun@163.com)
 * @date 2024-04-06
 *
 */
#ifndef __OperatorsWorkingDir_H__
#define __OperatorsWorkingDir_H__

#include "FITK_Kernel/FITKCore/FITKActionOperator.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"

namespace ModelOper
{
    /**
     * @brief 设置工作路径操作器
     * @author BaGuijun (baguijun@163.com)
     * @date 2024-04-06
     */
    class OperatorsWorkingDir : public Core::FITKActionOperator
    {
    public:
        /**
         * @brief Construct a new Oper G U I Working Dir object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-04-06
         */
        explicit OperatorsWorkingDir() = default;
        /**
         * @brief Destroy the Oper G U I Working Dir object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-04-06
         */
        ~OperatorsWorkingDir() = default;

    private:
        /**
         * @brief 执行
         * @return true 成功
         * @return false 失败
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-14
         */
        bool execGUI() override;
    };

    // 注册
    Register2FITKOperatorRepo(actionWorkingDir, OperatorsWorkingDir);
} // namespace GUIOper

#endif