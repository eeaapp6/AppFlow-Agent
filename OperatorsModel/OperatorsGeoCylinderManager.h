#ifndef _OperatorsGeoCylinderManager_H
#define _OperatorsGeoCylinderManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsGeoCylinderManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsGeoCylinderManager();
        ~OperatorsGeoCylinderManager();

        virtual bool execGUI();

        virtual bool execProfession();

        void moveToStep(int index, QVariant value) override;

    private slots:
        ;
        void slotReselectOriginPoint();

        void slotSelectFaceGroup();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionGeoCylinderCreate, OperatorsGeoCylinderManager);
    Register2FITKOPeratorRepo(actionGeoCylinderEdit, OperatorsGeoCylinderManager);
    Register2FITKOPeratorRepo(actionGeoCylinderDelete, OperatorsGeoCylinderManager);
    Register2FITKOPeratorRepo(actionGeoCylinderRename, OperatorsGeoCylinderManager);
}

#endif