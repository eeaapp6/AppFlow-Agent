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
        connect(_process, SIGNAL(readyReadStandardOutput()), this, SLOT(slotProcessOutput()));
        connect(_process, SIGNAL(readyReadStandardError()), this, SLOT(slotProcessOutputError()));
        connect(_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotProcessFinish(int, QProcess::ExitStatus)));
    }

    RunProcess::~RunProcess()
    {
        if (_process) {
            _process->kill();
            delete _process;
            _process = nullptr;
        }
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

        //判断进程是否启动成功
        if (!_process->waitForStarted()) {
            QString message = tr("Solution calculation failed to start!");
            emit FITKAPP->getSignalTransfer()->outputMessageSig(3, message);
            emit sigFinish();
        }
    }

    void RunProcess::kill()
    {
        if (_process == nullptr)return;
        _process->kill();
    }

    void RunProcess::slotProcessOutput()
    {
        QString message = _process->readAllStandardOutput();
        emit FITKAPP->getSignalTransfer()->outputMessageSig(4, message);
    }

    void RunProcess::slotProcessOutputError()
    {

    }

    void RunProcess::slotProcessFinish(int exitCode, QProcess::ExitStatus exitStatus)
    {
        switch (exitStatus)
        {
        case QProcess::NormalExit:
            break;
        case QProcess::CrashExit:
            break;
        }
        emit sigFinish();
    }
}

