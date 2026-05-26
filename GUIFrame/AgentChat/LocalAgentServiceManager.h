#pragma once

#include <QObject>
#include <QString>

class QProcess;

class LocalAgentServiceManager : public QObject
{
    Q_OBJECT

public:
    explicit LocalAgentServiceManager(QObject *parent = nullptr);
    ~LocalAgentServiceManager() override;

    bool ensureServiceRunning(QString *errorMessage = nullptr);
    bool isServiceAvailable() const;

private:
    QString projectRootPath() const;
    QString serviceScriptPath() const;
    QString pythonProgram() const;
    bool startService(QString *errorMessage);

    QProcess *m_process = nullptr;
};
