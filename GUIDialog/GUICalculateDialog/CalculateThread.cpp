#include "CalculateThread.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKSignalTransfer.h"
#include <QProcess>

namespace GUI
{
    CalculateThread::CalculateThread()
    {
        _info = new CalculateThreadInfo();
    }

    CalculateThread::~CalculateThread()
    {
        if (_info) {
            delete _info;
            _info = nullptr;
        }
    }

    CalculateThreadInfo* CalculateThread::getInfo()
    {
        return _info;
    }

    void CalculateThread::run()
    {
        if (_info == nullptr)return;
        QProcess pro;
        if (!_info->_workDir.isEmpty()) {
            pro.setWorkingDirectory(_info->_workDir);
        }
        int result = pro.execute(_info->_cmd);

        if (result == -2) {
            QString message = tr("Started failed!");
            emit FITKAPP->getSignalTransfer()->outputMessageSig(3, message);
        }
    }
}
