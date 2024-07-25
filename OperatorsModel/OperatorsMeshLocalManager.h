#ifndef _OperatorsMeshLocalManager_H
#define _OperatorsMeshLocalManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsMeshLocalManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsMeshLocalManager();
        ~OperatorsMeshLocalManager();

        virtual bool execGUI();

        virtual bool execProfession();

        void moveToStep(int index, QVariant value = QVariant()) override;
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionMeshLocalSelectGroup, OperatorsMeshLocalManager);
    Register2FITKOPeratorRepo(actionMeshLocalEdit, OperatorsMeshLocalManager);
    Register2FITKOPeratorRepo(actionMeshLocalDelete, OperatorsMeshLocalManager);
}
#endif

