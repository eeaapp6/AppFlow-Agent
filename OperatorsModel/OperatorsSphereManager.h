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

        void moveToStep(int index, QVariant value) override;

    private slots:
        ;
        void slotReselectCurrentPoint();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionCreateSphere, OperatorsSphereManager);
    Register2FITKOPeratorRepo(actionEditSphere, OperatorsSphereManager);
    Register2FITKOPeratorRepo(actionDeleteSphere, OperatorsSphereManager);
    Register2FITKOPeratorRepo(actionRenameSphere, OperatorsSphereManager);
}

#endif