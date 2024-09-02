#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "OperatorsLicense.h"

namespace ModelOper 
{
    bool OperatorsLicense::execGUI()
    {
        FITKAPP->showCopyrightDlg();
        return false;
    }
}