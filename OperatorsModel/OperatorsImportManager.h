#ifndef _OperatorsImportManager_H
#define _OperatorsImportManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsImportManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsImportManager();
        ~OperatorsImportManager();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionImportGeometry, OperatorsImportManager);
    Register2FITKOPeratorRepo(actionImportMesh, OperatorsImportManager);
}

#endif