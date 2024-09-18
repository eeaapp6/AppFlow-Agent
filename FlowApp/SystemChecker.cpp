#include "SystemChecker.h"
#include <QThread>

QStringList SystemChecker::check()
{
    QStringList s;
    //线程数检查，最小是4
    const int nThread = QThread::idealThreadCount();
    if (nThread < 4)
        s << QString("CPU thread count is %1, at least 4 !").arg(nThread);
    return s;
}

