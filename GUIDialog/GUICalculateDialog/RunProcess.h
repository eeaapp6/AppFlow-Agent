#ifndef _RunProcess_H
#define _RunProcess_H

#include <QObject>
#include <QProcess>

namespace GUI
{
    class RunProcess :public QObject
    {
        Q_OBJECT;
    public:
        RunProcess();
        ~RunProcess();

        void start(QString sh);

        void kill();
    signals:
        ;
        void sigFinish();
    private slots:
        ;
        void slotProcessOutput();
        void slotProcessOutputError();
        void slotProcessFinish(int exitCode, QProcess::ExitStatus exitStatus);
        void slotMainwindowClose();
    private:
        QProcess* _process = nullptr;
    };
}
#endif
