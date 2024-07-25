#ifndef _OperatorsGeoCubeManager_H
#define _OperatorsGeoCubeManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsGeoCubeManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsGeoCubeManager();
        ~OperatorsGeoCubeManager();

        virtual bool execGUI();

        virtual bool execProfession();

        void moveToStep(int index, QVariant value = QVariant()) override;
    private slots:
        ;
        void slotReselectBasePoint();

        void slotSelectFaceGroup();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionGeoCubeCreate, OperatorsGeoCubeManager);
    Register2FITKOPeratorRepo(actionGeoCubeEdit, OperatorsGeoCubeManager);
    Register2FITKOPeratorRepo(actionGeoCubeDelete, OperatorsGeoCubeManager);
    Register2FITKOPeratorRepo(actionGeoCubeRename, OperatorsGeoCubeManager);
}

#endif