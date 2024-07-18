#ifndef _OperatorsCylinderManager_H
#define _OperatorsCylinderManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsCylinderManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsCylinderManager();
        ~OperatorsCylinderManager();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionCreateCylinder, OperatorsCylinderManager);
    Register2FITKOPeratorRepo(actionEditCylinder, OperatorsCylinderManager);
}

#endif