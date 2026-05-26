#include "LocalAgentServiceManager.h"

#include "AppSettings.h"

#include <QCoreApplication>
#include <QDir>
#include <QEventLoop>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStringList>
#include <QThread>
#include <QTimer>
#include <QUrl>

namespace {

const char *HealthUrl = "http://127.0.0.1:8765/health";

}

LocalAgentServiceManager::LocalAgentServiceManager(QObject *parent)
    : QObject(parent)
{
}

LocalAgentServiceManager::~LocalAgentServiceManager()
{
    if (m_process == nullptr) {
        return;
    }

    m_process->terminate();
    if (!m_process->waitForFinished(1500)) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
}

bool LocalAgentServiceManager::ensureServiceRunning(QString *errorMessage)
{
    if (isServiceAvailable()) {
        return true;
    }

    if (!startService(errorMessage)) {
        return false;
    }

    for (int i = 0; i < 20; ++i) {
        if (isServiceAvailable()) {
            return true;
        }
        QCoreApplication::processEvents();
        QThread::msleep(100);
    }

    if (errorMessage != nullptr) {
        *errorMessage = QString::fromUtf8("\xE6\x9C\xAC\xE5\x9C\xB0\x20\x41\x67\x65\x6E\x74\x20\xE6\x9C\x8D\xE5\x8A\xA1\xE5\x90\xAF\xE5\x8A\xA8\xE5\x90\x8E\xE6\x9C\xAA\xE9\x80\x9A\xE8\xBF\x87\xE5\x81\xA5\xE5\xBA\xB7\xE6\xA3\x80\xE6\x9F\xA5\xE3\x80\x82");
    }
    return false;
}

bool LocalAgentServiceManager::isServiceAvailable() const
{
    QNetworkAccessManager network;
    QNetworkRequest request(QUrl(QString::fromUtf8(HealthUrl)));
    QNetworkReply *reply = network.get(request);

    QTimer timer;
    timer.setSingleShot(true);

    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start(500);
    loop.exec();

    const bool available = reply->isFinished() && reply->error() == QNetworkReply::NoError;
    reply->deleteLater();
    return available;
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

bool LocalAgentServiceManager::startService(QString *errorMessage)
{
    AppSettings settings;
    const QString provider = settings.activeProvider();
    const QString apiKey = settings.providerApiKey(provider);
    if (apiKey.isEmpty()) {
        if (errorMessage != nullptr) {
            *errorMessage = QString::fromUtf8("\xE5\xBD\x93\xE5\x89\x8D\xE6\x9C\x8D\xE5\x8A\xA1\xE5\x95\x86\x20\x41\x50\x49\x20\xE5\xAF\x86\xE9\x92\xA5\xE4\xB8\xBA\xE7\xA9\xBA\xEF\xBC\x8C\xE8\xAF\xB7\xE5\x85\x88\xE5\x9C\xA8\xE8\xAE\xBE\xE7\xBD\xAE\xE4\xB8\xAD\xE5\xA1\xAB\xE5\x86\x99\xE3\x80\x82");
        }
        return false;
    }

    const QString scriptPath = serviceScriptPath();
    if (!QFileInfo::exists(scriptPath)) {
        if (errorMessage != nullptr) {
            *errorMessage = QString::fromUtf8("\xE6\x9C\xAA\xE6\x89\xBE\xE5\x88\xB0\xE6\x9C\xAC\xE5\x9C\xB0\xE6\x9C\x8D\xE5\x8A\xA1\xE8\x84\x9A\xE6\x9C\xAC\xEF\xBC\x9A\x25\x31").arg(scriptPath);
        }
        return false;
    }

    if (m_process != nullptr) {
        m_process->deleteLater();
    }

    m_process = new QProcess(this);
    m_process->setWorkingDirectory(projectRootPath());

    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("ACTIVE_PROVIDER"), provider);
    environment.insert(QStringLiteral("PROVIDER_API_KEY"), apiKey);
    environment.insert(QStringLiteral("PROVIDER_MODEL"), settings.providerModel(provider));
    environment.insert(QStringLiteral("PROVIDER_BASE_URL"), settings.providerBaseUrl(provider));
    m_process->setProcessEnvironment(environment);

    QStringList arguments;
    arguments << scriptPath;
    m_process->start(pythonProgram(), arguments);
    if (!m_process->waitForStarted(3000)) {
        if (errorMessage != nullptr) {
            *errorMessage = QString::fromUtf8("\x50\x79\x74\x68\x6F\x6E\x20\xE6\x9C\x8D\xE5\x8A\xA1\xE5\x90\xAF\xE5\x8A\xA8\xE5\xA4\xB1\xE8\xB4\xA5\xEF\xBC\x9A\x25\x31").arg(m_process->errorString());
        }
        return false;
    }

    return true;
}
