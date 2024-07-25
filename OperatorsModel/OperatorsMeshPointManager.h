#ifndef _OperatorsMeshPointManager_H
#define _OperatorsMeshPointManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsMeshPointManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsMeshPointManager();
        ~OperatorsMeshPointManager();

        virtual bool execGUI();

        virtual bool execProfession();

        void moveToStep(int index, QVariant value = QVariant()) override;
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionMeshPointEdit, OperatorsMeshPointManager);
}
#endif

