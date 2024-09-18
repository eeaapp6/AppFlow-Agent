#ifndef _SYSTEM_CHECKER____H___
#define _SYSTEM_CHECKER____H___
 

#include "FITK_Kernel/FITKAppFramework/FITKAbstractSysChecker.h"

class SystemChecker : public AppFrame::FITKAbstractSysChecker
{
public: 
    explicit SystemChecker() = default;
    virtual ~SystemChecker() = default;

    virtual QStringList check() override;

};


#endif
