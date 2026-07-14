#include "LocalAgentServiceManager.h"

#include "AppSettings.h"

#include <QCoreApplication>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDir>
#include <QEventLoop>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStringList>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QVariant>

namespace {

const int HealthTimeoutMs = 500;
const int ProcessStopTimeoutMs = 1500;
const int ProcessKillTimeoutMs = 1000;
const int MaxDiagnosticBytes = 2048;

QString foreignServiceMessage(quint16 port)
{
    return QString::fromUtf8("\xE7\xAB\xAF\xE5\x8F\xA3\x20\x25\x31\x20\xE5\xB7\xB2\xE8\xA2\xAB\xE5\x85\xB6\xE4\xBB\x96\xE6\x9C\x8D\xE5\x8A\xA1\xE5\x8D\xA0\xE7\x94\xA8\xEF\xBC\x8C\xE6\x9C\xAA\xE6\xA3\x80\xE6\xB5\x8B\xE5\x88\xB0\x20\x73\x69\x6D\x61\x67\x65\x6E\x74\x5F\x63\x6F\x72\x65\xE3\x80\x82")
        .arg(port);
}

QString externalProviderMismatchMessage(const QString &runningProvider, const QString &configuredProvider)
{
    return QString::fromUtf8("\xE6\xA3\x80\xE6\xB5\x8B\xE5\x88\xB0\x20\x73\x69\x6D\x61\x67\x65\x6E\x74\x5F\x63\x6F\x72\x65\xEF\xBC\x8C\xE4\xBD\x86\xE8\xBF\x90\xE8\xA1\x8C\x20\x70\x72\x6F\x76\x69\x64\x65\x72\x20\xE4\xB8\xBA\x20\x25\x31\xEF\xBC\x8C\xE5\xBD\x93\xE5\x89\x8D\xE8\xAE\xBE\xE7\xBD\xAE\x20\x70\x72\x6F\x76\x69\x64\x65\x72\x20\xE4\xB8\xBA\x20\x25\x32\xE3\x80\x82\xE8\xAF\xB7\xE5\x81\x9C\xE6\xAD\xA2\xE5\xA4\x96\xE9\x83\xA8\x20\x41\x67\x65\x6E\x74\x20\xE6\x9C\x8D\xE5\x8A\xA1\xEF\xBC\x8C\xE6\x88\x96\xE5\xB0\x86\xE8\xAE\xBE\xE7\xBD\xAE\xE5\x88\x87\xE6\x8D\xA2\xE5\x88\xB0\xE5\xAF\xB9\xE5\xBA\x94\x20\x70\x72\x6F\x76\x69\x64\x65\x72\xE3\x80\x82")
        .arg(runningProvider, configuredProvider);
}

QString withDiagnostic(const QString &message, const QString &diagnostic)
{
    return diagnostic.isEmpty() ? message : QStringLiteral("%1\n%2").arg(message, diagnostic);
}

}

LocalAgentServiceManager::LocalAgentServiceManager(QObject *parent)
    : QObject(parent)
{
}

LocalAgentServiceManager::~LocalAgentServiceManager()
{
    stopOwnedProcess();
}

bool LocalAgentServiceManager::ensureServiceRunning(QString *errorMessage)
{
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    if (m_process != nullptr && m_process->state() == QProcess::NotRunning) {
        stopOwnedProcess();
    }

    const ServiceConfiguration configuration = currentServiceConfiguration();
    const HealthCheckResult health = checkServiceHealth(configuration);
    if (health.status == HealthStatus::Compatible) {
        if (!hasRunningOwnedProcess()) {
            return true;
        }
        if (!m_ownedConfigurationFingerprint.isEmpty()
            && m_ownedConfigurationFingerprint == configuration.fingerprint) {
            return true;
        }

        stopOwnedProcess();
        return startService(configuration, errorMessage)
            && waitForCompatibleService(configuration, errorMessage);
    }
    if (health.status == HealthStatus::ForeignService) {
        if (errorMessage != nullptr) {
            *errorMessage = foreignServiceMessage(configuration.agentServicePort);
        }
        return false;
    }
    if (health.status == HealthStatus::ProviderMismatch && !hasRunningOwnedProcess()) {
        if (errorMessage != nullptr) {
            const QString configuredProvider = AppSettings().activeProvider().trimmed().toLower();
            *errorMessage = externalProviderMismatchMessage(health.runningProvider, configuredProvider);
        }
        return false;
    }

    stopOwnedProcess();
    return startService(configuration, errorMessage)
        && waitForCompatibleService(configuration, errorMessage);
}

bool LocalAgentServiceManager::isServiceAvailable() const
{
    const ServiceConfiguration configuration = currentServiceConfiguration();
    return checkServiceHealth(configuration).status == HealthStatus::Compatible;
}

LocalAgentServiceManager::ServiceConfiguration LocalAgentServiceManager::currentServiceConfiguration() const
{
    AppSettings settings;
    ServiceConfiguration configuration;
    configuration.provider = settings.activeProvider().trimmed().toLower();
    configuration.apiKey = settings.providerApiKey(configuration.provider);
    configuration.model = settings.providerModel(configuration.provider).trimmed();
    configuration.baseUrl = settings.providerBaseUrl(configuration.provider).trimmed();
    configuration.agentServicePort = settings.agentServicePort();
    configuration.agentServiceBaseUrl = settings.agentServiceBaseUrl();
    configuration.pythonProgram = pythonProgram().trimmed();
    configuration.scriptPath = QDir::cleanPath(serviceScriptPath());
    configuration.fingerprint = configurationFingerprint(configuration);
    return configuration;
}

QByteArray LocalAgentServiceManager::configurationFingerprint(
    const ServiceConfiguration &configuration) const
{
    QCryptographicHash hash(QCryptographicHash::Sha256);
    const auto addField = [&hash](const QString &value) {
        const QByteArray bytes = value.toUtf8();
        hash.addData(QByteArray::number(bytes.size()));
        hash.addData(":", 1);
        hash.addData(bytes);
    };

    addField(configuration.provider);
    addField(configuration.apiKey);
    addField(configuration.model);
    addField(configuration.baseUrl);
    addField(QString::number(configuration.agentServicePort));
    addField(configuration.agentServiceBaseUrl);
    return hash.result();
}

LocalAgentServiceManager::HealthCheckResult LocalAgentServiceManager::checkServiceHealth(
    const ServiceConfiguration &configuration) const
{
    QNetworkAccessManager network;
    QNetworkRequest request(QUrl(configuration.agentServiceBaseUrl + QStringLiteral("/health")));
    QNetworkReply *reply = network.get(request);

    QTimer timer;
    timer.setSingleShot(true);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start(HealthTimeoutMs);
    loop.exec();

    if (!reply->isFinished()) {
        reply->abort();
        reply->deleteLater();
        return {};
    }

    const QVariant httpStatusValue = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (!httpStatusValue.isValid()) {
        reply->deleteLater();
        return {};
    }

    const int httpStatus = httpStatusValue.toInt();
    const QByteArray responseBody = reply->readAll();
    const QNetworkReply::NetworkError networkError = reply->error();
    reply->deleteLater();
    if (httpStatus != 200) {
        return {HealthStatus::ForeignService, {}};
    }
    if (networkError != QNetworkReply::NoError) {
        return {};
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(responseBody, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        return {HealthStatus::ForeignService, {}};
    }

    const QJsonObject root = document.object();
    const QJsonValue statusValue = root.value(QStringLiteral("status"));
    const QJsonValue coreValue = root.value(QStringLiteral("core"));
    const QJsonValue providerValue = root.value(QStringLiteral("provider"));
    if (!statusValue.isString() || statusValue.toString() != QStringLiteral("ok")
        || !coreValue.isString() || coreValue.toString() != QStringLiteral("simagent_core")
        || !providerValue.isString()) {
        return {HealthStatus::ForeignService, {}};
    }

    const QString runningProvider = providerValue.toString().trimmed().toLower();
    if (runningProvider.isEmpty()) {
        return {HealthStatus::ForeignService, {}};
    }

    const QString configuredProvider = configuration.provider;
    if (runningProvider != configuredProvider) {
        return {HealthStatus::ProviderMismatch, runningProvider};
    }
    return {HealthStatus::Compatible, runningProvider};
}

bool LocalAgentServiceManager::waitForCompatibleService(
    const ServiceConfiguration &configuration,
    QString *errorMessage)
{
    for (int i = 0; i < 20; ++i) {
        if (m_process == nullptr || m_process->state() == QProcess::NotRunning) {
            const QString diagnostic = takeOwnedProcessDiagnostic(configuration.apiKey);
            stopOwnedProcess();
            if (errorMessage != nullptr) {
                *errorMessage = withDiagnostic(
                    QString::fromUtf8("\xE6\x9C\xAC\xE5\x9C\xB0\x20\x41\x67\x65\x6E\x74\x20\xE6\x9C\x8D\xE5\x8A\xA1\xE8\xBF\x9B\xE7\xA8\x8B\xE6\x8F\x90\xE5\x89\x8D\xE9\x80\x80\xE5\x87\xBA\xE3\x80\x82"),
                    diagnostic);
            }
            return false;
        }

        const HealthCheckResult health = checkServiceHealth(configuration);
        if (health.status == HealthStatus::Compatible) {
            return true;
        }
        if (health.status == HealthStatus::ForeignService) {
            const QString diagnostic = takeOwnedProcessDiagnostic(configuration.apiKey);
            stopOwnedProcess();
            if (errorMessage != nullptr) {
                *errorMessage = withDiagnostic(
                    foreignServiceMessage(configuration.agentServicePort), diagnostic);
            }
            return false;
        }
        if (health.status == HealthStatus::ProviderMismatch) {
            const QString diagnostic = takeOwnedProcessDiagnostic(configuration.apiKey);
            stopOwnedProcess();
            if (errorMessage != nullptr) {
                *errorMessage = withDiagnostic(
                    QStringLiteral("Local simagent_core provider mismatch: running %1, configured %2.")
                        .arg(health.runningProvider, configuration.provider),
                    diagnostic);
            }
            return false;
        }

        QCoreApplication::processEvents();
        QThread::msleep(100);
    }

    const QString diagnostic = takeOwnedProcessDiagnostic(configuration.apiKey);
    stopOwnedProcess();
    if (errorMessage != nullptr) {
        *errorMessage = withDiagnostic(
            QString::fromUtf8("\xE6\x9C\xAC\xE5\x9C\xB0\x20\x41\x67\x65\x6E\x74\x20\xE6\x9C\x8D\xE5\x8A\xA1\xE5\x90\xAF\xE5\x8A\xA8\xE5\x90\x8E\xE6\x9C\xAA\xE9\x80\x9A\xE8\xBF\x87\xE4\xB8\xA5\xE6\xA0\xBC\xE5\x81\xA5\xE5\xBA\xB7\xE6\xA3\x80\xE6\x9F\xA5\xE3\x80\x82"),
            diagnostic);
    }
    return false;
}

bool LocalAgentServiceManager::hasRunningOwnedProcess() const
{
    return m_process != nullptr && m_process->state() != QProcess::NotRunning;
}

void LocalAgentServiceManager::stopOwnedProcess()
{
    m_ownedConfigurationFingerprint.clear();
    if (m_process == nullptr) {
        return;
    }

    if (m_process->state() != QProcess::NotRunning) {
        m_process->terminate();
        if (!m_process->waitForFinished(ProcessStopTimeoutMs)) {
            m_process->kill();
            m_process->waitForFinished(ProcessKillTimeoutMs);
        }
    }

    delete m_process;
    m_process = nullptr;
}

QString LocalAgentServiceManager::takeOwnedProcessDiagnostic(const QString &apiKey)
{
    if (m_process == nullptr) {
        return {};
    }

    QByteArray standardError = m_process->readAllStandardError();
    if (standardError.size() > MaxDiagnosticBytes) {
        standardError = standardError.right(MaxDiagnosticBytes);
    }
    QString diagnostic = QString::fromLocal8Bit(standardError).trimmed();

    if (!apiKey.isEmpty()) {
        diagnostic.replace(apiKey, QStringLiteral("[redacted]"), Qt::CaseSensitive);
    }
    return diagnostic;
}

QString LocalAgentServiceManager::projectRootPath() const
{
    QDir dir(QCoreApplication::applicationDirPath());
    while (!dir.exists(QStringLiteral("scripts/agent_service.py"))
           && !dir.exists(QStringLiteral("AgentBridge/chat_ui_backend/scripts/agent_service.py"))
           && dir.cdUp()) {
    }

    if (dir.exists(QStringLiteral("AgentBridge/chat_ui_backend/scripts/agent_service.py"))) {
        return dir.filePath(QStringLiteral("AgentBridge/chat_ui_backend"));
    }

    return dir.absolutePath();
}

QString LocalAgentServiceManager::serviceScriptPath() const
{
    return QDir(projectRootPath()).filePath(QStringLiteral("scripts/agent_service.py"));
}

QString LocalAgentServiceManager::pythonProgram() const
{
    return QStringLiteral("python");
}

bool LocalAgentServiceManager::startService(
    const ServiceConfiguration &configuration,
    QString *errorMessage)
{
    stopOwnedProcess();

    if (configuration.apiKey.isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = QString::fromUtf8("\xE5\xBD\x93\xE5\x89\x8D\xE6\x9C\x8D\xE5\x8A\xA1\xE5\x95\x86\x20\x41\x50\x49\x20\xE5\xAF\x86\xE9\x92\xA5\xE4\xB8\xBA\xE7\xA9\xBA\xEF\xBC\x8C\xE8\xAF\xB7\xE5\x85\x88\xE5\x9C\xA8\xE8\xAE\xBE\xE7\xBD\xAE\xE4\xB8\xAD\xE5\xA1\xAB\xE5\x86\x99\xE3\x80\x82");
        }
        return false;
    }

    if (!QFileInfo::exists(configuration.scriptPath)) {
        if (errorMessage != nullptr) {
            *errorMessage = QString::fromUtf8("\xE6\x9C\xAA\xE6\x89\xBE\xE5\x88\xB0\xE6\x9C\xAC\xE5\x9C\xB0\xE6\x9C\x8D\xE5\x8A\xA1\xE8\x84\x9A\xE6\x9C\xAC\xEF\xBC\x9A\x25\x31").arg(configuration.scriptPath);
        }
        return false;
    }

    m_process = new QProcess(this);
    m_process->setWorkingDirectory(projectRootPath());

    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("ACTIVE_PROVIDER"), configuration.provider);
    environment.insert(QStringLiteral("PROVIDER_API_KEY"), configuration.apiKey);
    environment.insert(QStringLiteral("PROVIDER_MODEL"), configuration.model);
    environment.insert(QStringLiteral("PROVIDER_BASE_URL"), configuration.baseUrl);
    environment.insert(QStringLiteral("SIMAGENT_HOST"), QStringLiteral("127.0.0.1"));
    environment.insert(
        QStringLiteral("SIMAGENT_PORT"), QString::number(configuration.agentServicePort));
    m_process->setProcessEnvironment(environment);

    QStringList arguments;
    arguments << configuration.scriptPath;
    m_process->start(configuration.pythonProgram, arguments);
    if (!m_process->waitForStarted(3000)) {
        const QString processError = m_process->errorString();
        const QString diagnostic = takeOwnedProcessDiagnostic(configuration.apiKey);
        stopOwnedProcess();
        if (errorMessage != nullptr) {
            *errorMessage = withDiagnostic(
                QString::fromUtf8("\x50\x79\x74\x68\x6F\x6E\x20\xE6\x9C\x8D\xE5\x8A\xA1\xE5\x90\xAF\xE5\x8A\xA8\xE5\xA4\xB1\xE8\xB4\xA5\xEF\xBC\x9A\x25\x31").arg(processError),
                diagnostic);
        }
        return false;
    }

    m_ownedConfigurationFingerprint = configuration.fingerprint;
    return true;
}
