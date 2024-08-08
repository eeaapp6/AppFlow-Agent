#ifndef _OperatorsImportManager_H
#define _OperatorsImportManager_H

#include "OperManagerBase.h"
#include "FITK_Kernel/FITKCore/FITKThreadTask.h"

namespace ModelOper
{
    enum class ImportType {
        ImportNone,
        ImportGeo,
        ImportMesh,
    };
    class OperatorsImportManager :public OperManagerBase
    {
        Q_OBJECT;
    public:
        OperatorsImportManager();
        ~OperatorsImportManager();

        virtual bool execGUI();

        virtual bool execProfession();

    private slots:
        ;
        void slotGeoImportFinish(bool result, int objID);
    };

    class ImportReadThread :public Core::FITKThreadTask
    {
        Q_OBJECT;
    public:
        ImportReadThread() = default;
        ~ImportReadThread() = default;

        void run();
    signals:
        ;
        void sigImportFinish(bool, int);

    public:
        ImportType _type = ImportType::ImportNone;
        QString _fileName = "";
    };

    // 按钮注册相关操作
    Register2FITKOPeratorRepo(actionImportGeometry, OperatorsImportManager);
    Register2FITKOPeratorRepo(actionImportMesh, OperatorsImportManager);
}

#endif