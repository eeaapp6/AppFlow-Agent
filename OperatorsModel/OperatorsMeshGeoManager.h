#ifndef _OperatorsMeshGeoManager_H
#define _OperatorsMeshGeoManager_H

#include "OperManagerBase.h"

namespace ModelOper
{
    class OperatorsMeshGeoManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsMeshGeoManager();
        ~OperatorsMeshGeoManager();

        virtual bool execGUI();

        virtual bool execProfession();
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionMeshGeoEdit, OperatorsMeshGeoManager);
}

#endif