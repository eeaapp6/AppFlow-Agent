#include "RunProcess.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"
#include "FITK_Kernel/FITKAppFramework/FITKSignalTransfer.h"
#include "FITK_Kernel/FITKCore/FITKDirFileTools.h"

#include <QFile>
#include <QTextStream>

namespace GUI
{
    RunProcess::RunProcess()
    {
        _process = new QProcess(this);
        connect(_process, SIGNAL(readyReadStandardOutput(QPrivateSignal)), this, SLOT(slotProcessOutput()));
        connect(_process, SIGNAL(finished(int, QProcess::ExitStatus);), this, SLOT(slotProcessFinish(int, QProcess::ExitStatus)));
    }

    RunProcess::~RunProcess()
    {

    }

    void RunProcess::start(QString sh)
    {
        //工作路径获取
        QString workDir = "";
        if (FITKAPP->getAppSettings()) {
            workDir = FITKAPP->getAppSettings()->getWorkingDir();
        }
        if (workDir.isEmpty()) workDir = QApplication::applicationDirPath() + "/../WorkDir";
        Core::CreateDir(workDir);
        QString shFilePath = workDir + "/startOpenFoam.sh";
        QFile file(shFilePath);
        // 打开文件进行写操作
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
        // 创建一个 QTextStream 对象来写入文本
        QTextStream out(&file);
        // 写入字符串到文件
        out << sh;
        // 关闭文件
        file.close();

        _process->start("/bin/bash", QStringList() << shFilePath);
    }

    void RunProcess::slotProcessOutput()
    {
        QString message = _process->readAllStandardOutput();
        emit FITKAPP->getSignalTransfer()->outputMessageSig(4, message);
    }

    void RunProcess::slotProcessFinish(int exitCode, QProcess::ExitStatus exitStatus)
    {
        if (exitStatus == QProcess::NormalExit) {}
        if (exitStatus == QProcess::CrashExit) {}
    }

    void RunProcess::outputMessage(QString message)
    {

    }
}

