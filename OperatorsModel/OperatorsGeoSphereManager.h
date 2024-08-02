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
        /**
         * @brief 其他事件执行
         * @param[i]  index          事件索引（自定义）
         * @param[i]  value          其他数据
         * @author BaGuijun (baguijun@163.com)
         * @date 2024-08-01
         */
        void eventProcess(int index, QVariant value) override;

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