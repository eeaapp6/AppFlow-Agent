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
    Register2FITKOPeratorRepo(actionCreateCylinder, OperatorsCylinderManager);
    Register2FITKOPeratorRepo(actionEditCylinder, OperatorsCylinderManager);
    Register2FITKOPeratorRepo(actionDeleteCylinder, OperatorsCylinderManager);
    Register2FITKOPeratorRepo(actionRenameCylinder, OperatorsCylinderManager);
}

#endif