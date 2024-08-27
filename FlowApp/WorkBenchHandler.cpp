#include "WorkBenchHandler.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Kernel/FITKCore/FITKActionOperator.h"

void FlowAppWorkBenchHandler::execHandler()
{
    //文件数量
    const int n = this->getInputFilesCount();
    for (int i = 0; i<n; ++i)
    {
        //遍历文件
        AppFrame::IOFileInfo finfo = this->getInputFileInfo(i);
        if (finfo._suffix == "brep" || finfo._suffix == "step")
        {
            //导入文件
            this->importGeoFile(QString("%1/%2").arg(finfo._path).arg(finfo._name));
        }
    }

}

void FlowAppWorkBenchHandler::execOutput()
{

}

void FlowAppWorkBenchHandler::importGeoFile(const QString & fileName)
{
    //获取操作器
    Core::FITKActionOperator* oper = FITKOPERREPO->getOperatorT<Core::FITKActionOperator>("actionImportGeometry");
    if (oper == nullptr) return;
    oper->setArgs("FileName", fileName);
    oper->setArgs("SenderName", "actionImportGeometry");
    //执行操作
    oper->execProfession();
}

