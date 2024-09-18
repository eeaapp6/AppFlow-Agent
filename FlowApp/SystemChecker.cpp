#include "SystemChecker.h"
#include <QThread>

QStringList SystemChecker::check()
{
    QStringList s;
    const int nThread = QThread::idealThreadCount();
    if (nThread < 4)
        s << QString("CPU thread count is %1, at least 4 !").arg(nThread);
    return s;
}

