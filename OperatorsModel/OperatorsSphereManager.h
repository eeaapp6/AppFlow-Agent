#ifndef _OperatorsSphereManager_H
#define _OperatorsSphereManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsSphereManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsSphereManager();
        ~OperatorsSphereManager();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionCreateSphere, OperatorsSphereManager);
    Register2FITKOPeratorRepo(actionEditSphere, OperatorsSphereManager);
}

#endif