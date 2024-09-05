/**********************************************************************
 * @file   CalculateDriver.h
 * @brief  求解器第三方程序驱动类
 * @author BaGuijun (baguijun@163.com)
 * @date   2024-09-05
 *********************************************************************/
#include "FITK_Kernel/FITKAppFramework/FITKExecProgramDriver.h"
#include "FITK_Kernel/FITKAppFramework/FITKProgramDriverFactory.h"

namespace GUI
{
    class CalculateDriver : public AppFrame::FITKExecProgramDriver
    {
        Q_OBJECT;
    public:
        CalculateDriver();
        ~CalculateDriver();
        virtual int getProgramType() override;
        virtual QString getProgramName() override;
    };

    Register2FITKProgramDriverFactory(1, CalculateDriver, CalculateDriver)
} // namespace GUI

    