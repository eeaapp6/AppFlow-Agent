#ifndef _OperatorsCubeManager_H
#define _OperatorsCubeManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsCubeManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsCubeManager();
        ~OperatorsCubeManager();

        virtual bool execGUI();

        virtual bool execProfession();

        void moveToStep(int index, QVariant value = QVariant()) override;
    private slots:
        ;
        void slotReselectBasePoint();

        void slotSelectFaceGroup();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionGeoCubeCreate, OperatorsCubeManager);
    Register2FITKOPeratorRepo(actionGeoCubeEdit, OperatorsCubeManager);
    Register2FITKOPeratorRepo(actionGeoCubeDelete, OperatorsCubeManager);
    Register2FITKOPeratorRepo(actionGeoCubeRename, OperatorsCubeManager);
}

#endif