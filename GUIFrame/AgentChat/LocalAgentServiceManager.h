#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QtGlobal>

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
    enum class HealthStatus {
        Unavailable,
        Compatible,
        ProviderMismatch,
        ForeignService,
    };

    struct HealthCheckResult {
        HealthStatus status = HealthStatus::Unavailable;
        QString runningProvider;
    };

    struct ServiceConfiguration {
        QString provider;
        QString apiKey;
        QString model;
        QString baseUrl;
        quint16 agentServicePort = 0;
        QString agentServiceBaseUrl;
        QString pythonProgram;
        QString scriptPath;
        QByteArray fingerprint;
    };

    QString projectRootPath() const;
    QString serviceScriptPath() const;
    QString pythonProgram() const;
    ServiceConfiguration currentServiceConfiguration() const;
    QByteArray configurationFingerprint(const ServiceConfiguration &configuration) const;
    HealthCheckResult checkServiceHealth(const ServiceConfiguration &configuration) const;
    bool waitForCompatibleService(const ServiceConfiguration &configuration, QString *errorMessage);
    bool hasRunningOwnedProcess() const;
    void stopOwnedProcess();
    QString takeOwnedProcessDiagnostic(const QString &apiKey);
    bool startService(const ServiceConfiguration &configuration, QString *errorMessage);

    QProcess *m_process = nullptr;
    QByteArray m_ownedConfigurationFingerprint;
};
