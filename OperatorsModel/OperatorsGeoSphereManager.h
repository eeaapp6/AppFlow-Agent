#ifndef _OperatorsGeoSphereManager_H
#define _OperatorsGeoSphereManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsGeoSphereManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsGeoSphereManager();
        ~OperatorsGeoSphereManager();

        virtual bool execGUI();

        virtual bool execProfession();

        void moveToStep(int index, QVariant value) override;

    private slots:
        ;
        void slotReselectCurrentPoint();

        void slotSelectFaceGroup();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionGeoSphereCreate, OperatorsGeoSphereManager);
    Register2FITKOPeratorRepo(actionGeoSphereEdit, OperatorsGeoSphereManager);
    Register2FITKOPeratorRepo(actionGeoSphereDelete, OperatorsGeoSphereManager);
    Register2FITKOPeratorRepo(actionGeoSphereRename, OperatorsGeoSphereManager);
}

#endif