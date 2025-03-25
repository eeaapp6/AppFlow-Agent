#include "RunProcess.h"

#include "GUIFrame/MainWindow.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"
#include "FITK_Kernel/FITKAppFramework/FITKSignalTransfer.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKCore/FITKDirFileTools.h"

#include <QFile>
#include <QTextStream>

namespace GUI
{
    RunProcess::RunProcess()
    {
        _process = new QProcess(this);
        MainWindow* mianWindow = dynamic_cast<MainWindow*>(FITKAPP->getGlobalData()->getMainWindow());
        connect(_process, SIGNAL(readyReadStandardOutput()), this, SLOT(slotProcessOutput()));
        connect(_process, SIGNAL(readyReadStandardError()), this, SLOT(slotProcessOutputError()));
        connect(_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(slotProcessFinish(int, QProcess::ExitStatus)));
        connect(mianWindow, SIGNAL(sigMainWindowClose()), this, SLOT(slotMainwindowClose()));
    }

    RunProcess::~RunProcess()
    {
        if (_process) {
            _process->kill();
            _process->waitForFinished();
            delete _process;
            _process = nullptr;
        }
    }

    void RunProcess::start(QString sh)
    {
        _process->start(sh);

        //判断进程是否启动成功
        if (!_process->waitForStarted()) {
            QString message = tr("Started failed!");
            emit FITKAPP->getSignalTransfer()->outputMessageSig(3, message);
            emit sigFinish();
        }
        else {
            QString message = tr("Started successfully!");
            emit FITKAPP->getSignalTransfer()->outputMessageSig(1, message);
        }
    }

    void RunProcess::kill()
    {
        if (_process == nullptr)return;
        _process->kill();
        _process->waitForFinished();
    }

    void RunProcess::slotProcessOutput()
    {
        QString message = _process->readAllStandardOutput();
        emit FITKAPP->getSignalTransfer()->outputMessageSig(4, message);
    }

    void RunProcess::slotProcessOutputError()
    {
        QString message = _process->readAllStandardOutput();
        emit FITKAPP->getSignalTransfer()->outputMessageSig(3, message);
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

    void RunProcess::slotMainwindowClose()
    {
        this->kill();
    }
}

