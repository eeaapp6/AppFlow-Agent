#include "AppSettings.h"

#include <QSettings>
#include <QStandardPaths>

namespace {

const char *ActiveProviderSetting = "model/activeProvider";
const char *OutputRootSetting = "workflow/outputRootPath";
const char *BackendOutputRootSetting = "workflow/backendOutputRootPath";
const char *DefaultProviderId = "deepseek";
constexpr quint16 DefaultAgentServicePort = 8765;

QSettings &createSettings()
{
    static QSettings settings(QStringLiteral("FoamAgent"), QStringLiteral("ChatUI"));
    return settings;
}

QString providerPrefix(const QString &providerId)
{
    return QStringLiteral("providers/%1").arg(providerId.trimmed().toLower());
}

QString settingKey(const QString &providerId, const QString &name)
{
    return QStringLiteral("%1/%2").arg(providerPrefix(providerId), name);
}

QString defaultModelForProvider(const QString &providerId)
{
    const QString normalizedProvider = providerId.trimmed().toLower();
    if (normalizedProvider == QStringLiteral("openai")) {
        return QStringLiteral("gpt-4.1-mini");
    }
    if (normalizedProvider == QStringLiteral("gemini")) {
        return QStringLiteral("gemini-2.5-flash");
    }

    return QStringLiteral("deepseek-v4-flash");
}

QString defaultBaseUrlForProvider(const QString &providerId)
{
    const QString normalizedProvider = providerId.trimmed().toLower();
    if (normalizedProvider == QStringLiteral("openai")) {
        return QStringLiteral("https://api.openai.com");
    }
    if (normalizedProvider == QStringLiteral("gemini")) {
        return QStringLiteral("https://generativelanguage.googleapis.com");
    }

    return QStringLiteral("https://api.deepseek.com");
}

QString defaultOutputRootPath()
{
    const QString documents = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (!documents.isEmpty()) {
        return documents + QStringLiteral("/FoamAgentOutputs");
    }

    return QStringLiteral("FoamAgentOutputs");
}

}

QString AppSettings::activeProvider() const
{
    return createSettings()
        .value(QString::fromUtf8(ActiveProviderSetting), QString::fromUtf8(DefaultProviderId))
        .toString()
        .trimmed()
        .toLower();
}

void AppSettings::setActiveProvider(const QString &providerId)
{
    const QString normalizedProvider = providerId.trimmed().toLower();
    createSettings().setValue(
        QString::fromUtf8(ActiveProviderSetting),
        normalizedProvider.isEmpty() ? QString::fromUtf8(DefaultProviderId) : normalizedProvider);
}

QStringList AppSettings::supportedProviders() const
{
    return {
        QStringLiteral("deepseek"),
        QStringLiteral("openai"),
        QStringLiteral("gemini"),
    };
}

QString AppSettings::displayNameForProvider(const QString &providerId) const
{
    const QString normalizedProvider = providerId.trimmed().toLower();
    if (normalizedProvider == QStringLiteral("openai")) {
        return QStringLiteral("OpenAI");
    }
    if (normalizedProvider == QStringLiteral("gemini")) {
        return QStringLiteral("Gemini");
    }

    return QStringLiteral("DeepSeek");
}

QString AppSettings::providerApiKey(const QString &providerId) const
{
    return createSettings().value(settingKey(providerId, QStringLiteral("apiKey"))).toString().trimmed();
}

void AppSettings::setProviderApiKey(const QString &providerId, const QString &apiKey)
{
    createSettings().setValue(settingKey(providerId, QStringLiteral("apiKey")), apiKey.trimmed());
}

QString AppSettings::providerModel(const QString &providerId) const
{
    return createSettings()
        .value(settingKey(providerId, QStringLiteral("model")), defaultModelForProvider(providerId))
        .toString()
        .trimmed();
}

void AppSettings::setProviderModel(const QString &providerId, const QString &model)
{
    const QString normalizedModel = model.trimmed();
    createSettings().setValue(
        settingKey(providerId, QStringLiteral("model")),
        normalizedModel.isEmpty() ? defaultModelForProvider(providerId) : normalizedModel);
}

QString AppSettings::providerBaseUrl(const QString &providerId) const
{
    return createSettings()
        .value(settingKey(providerId, QStringLiteral("baseUrl")), defaultBaseUrlForProvider(providerId))
        .toString()
        .trimmed();
}

void AppSettings::setProviderBaseUrl(const QString &providerId, const QString &baseUrl)
{
    const QString normalizedBaseUrl = baseUrl.trimmed();
    createSettings().setValue(
        settingKey(providerId, QStringLiteral("baseUrl")),
        normalizedBaseUrl.isEmpty() ? defaultBaseUrlForProvider(providerId) : normalizedBaseUrl);
}

quint16 AppSettings::agentServicePort() const
{
    const QString value = qEnvironmentVariable("SIMAGENT_PORT").trimmed();
    if (value.isEmpty()) {
        return DefaultAgentServicePort;
    }
    for (const QChar character : value) {
        if (character < QChar('0') || character > QChar('9')) {
            return DefaultAgentServicePort;
        }
    }

    bool valid = false;
    const uint configuredPort = value.toUInt(&valid, 10);
    if (!valid || configuredPort == 0 || configuredPort > 65535) {
        return DefaultAgentServicePort;
    }
    return static_cast<quint16>(configuredPort);
}

QString AppSettings::agentServiceBaseUrl() const
{
    return QStringLiteral("http://127.0.0.1:%1").arg(agentServicePort());
}

bool AppSettings::hasProviderApiKey(const QString &providerId) const
{
    return !providerApiKey(providerId).isEmpty();
}

bool AppSettings::hasActiveProviderApiKey() const
{
    return hasProviderApiKey(activeProvider());
}

QString AppSettings::outputRootPath() const
{
    return createSettings()
        .value(QString::fromUtf8(OutputRootSetting), defaultOutputRootPath())
        .toString()
        .trimmed();
}

void AppSettings::setOutputRootPath(const QString &path)
{
    const QString normalizedPath = path.trimmed();
    createSettings().setValue(
        QString::fromUtf8(OutputRootSetting),
        normalizedPath.isEmpty() ? defaultOutputRootPath() : normalizedPath);
}

QString AppSettings::backendOutputRootPath() const
{
    return createSettings()
        .value(QString::fromUtf8(BackendOutputRootSetting), outputRootPath())
        .toString()
        .trimmed();
}

void AppSettings::setBackendOutputRootPath(const QString &path)
{
    const QString normalizedPath = path.trimmed();
    createSettings().setValue(
        QString::fromUtf8(BackendOutputRootSetting),
        normalizedPath.isEmpty() ? outputRootPath() : normalizedPath);
}

QString AppSettings::deepSeekApiKey() const
{
    return providerApiKey(QStringLiteral("deepseek"));
}

void AppSettings::setDeepSeekApiKey(const QString &apiKey)
{
    setProviderApiKey(QStringLiteral("deepseek"), apiKey);
}

QString AppSettings::deepSeekModel() const
{
    return providerModel(QStringLiteral("deepseek"));
}

void AppSettings::setDeepSeekModel(const QString &model)
{
    setProviderModel(QStringLiteral("deepseek"), model);
}

bool AppSettings::hasDeepSeekApiKey() const
{
    return hasProviderApiKey(QStringLiteral("deepseek"));
}
