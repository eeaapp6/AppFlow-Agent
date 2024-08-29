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
    private slots:
        ;
        void slotProcessOutput();
        void slotProcessFinish(int exitCode, QProcess::ExitStatus exitStatus);
    private:
        void outputMessage(QString message);
    private:
        QProcess* _process = nullptr;
    };
}
#endif
