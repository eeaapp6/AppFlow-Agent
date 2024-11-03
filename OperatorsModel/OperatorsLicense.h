#ifndef __OPER_LICENSE_H___
#define __OPER_LICENSE_H___

#include "FITK_Kernel/FITKCore/FITKActionOperator.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"

namespace ModelOper
{
    class OperatorsLicense : public  Core::FITKActionOperator
    {
        Q_OBJECT;
    public:
        explicit OperatorsLicense() = default;
        virtual ~OperatorsLicense() = default;
    protected:
        bool execGUI() override;
    private:
    };
    Register2FITKOperatorRepo(actionLicense, OperatorsLicense);
}


#endif
