#ifndef __OperatorsCFDPost_H__
#define __OperatorsCFDPost_H__

#include "FITK_Kernel/FITKCore/FITKActionOperator.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"

namespace ModelOper
{
    class OperatorsCFDPost : public Core::FITKActionOperator
    {
    public:
        /**
         * @brief Construct a new Oper G U I Working Dir object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-04-06
         */
        explicit OperatorsCFDPost() = default;
        /**
         * @brief Destroy the Oper G U I Working Dir object
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-04-06
         */
        ~OperatorsCFDPost() = default;

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
    Register2FITKOperatorRepo(actionCFDPostPath, OperatorsCFDPost);
} // namespace GUIOper

#endif