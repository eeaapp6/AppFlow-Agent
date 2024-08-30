#ifndef _CalculateThread_H
#define _CalculateThread_H

#include "FITK_Kernel/FITKCore/FITKThreadTask.h"

namespace GUI
{
    struct CalculateThreadInfo{
        QString _workDir = "";
        QString _cmd = "";
    };

    class CalculateThread : public Core::FITKThreadTask
    {
        Q_OBJECT;
    public:
        CalculateThread();
        ~CalculateThread();

        CalculateThreadInfo* getInfo();

        void run() override;
    private:
        CalculateThreadInfo* _info = nullptr;
    };
}

#endif
