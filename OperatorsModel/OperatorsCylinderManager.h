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

        void moveToStep(int index, QVariant value) override;

    private slots:
        ;
        void slotReselectOriginPoint();

        void slotSelectFaceGroup();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionGeoCylinderCreate, OperatorsCylinderManager);
    Register2FITKOPeratorRepo(actionGeoCylinderEdit, OperatorsCylinderManager);
    Register2FITKOPeratorRepo(actionGeoCylinderDelete, OperatorsCylinderManager);
    Register2FITKOPeratorRepo(actionGeoCylinderRename, OperatorsCylinderManager);
}

#endif