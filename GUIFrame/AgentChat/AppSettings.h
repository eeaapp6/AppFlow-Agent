#pragma once

#include <QString>
#include <QStringList>

class AppSettings
{
public:
    QString activeProvider() const;
    void setActiveProvider(const QString &providerId);

    QStringList supportedProviders() const;
    QString displayNameForProvider(const QString &providerId) const;

    QString providerApiKey(const QString &providerId) const;
    void setProviderApiKey(const QString &providerId, const QString &apiKey);

    QString providerModel(const QString &providerId) const;
    void setProviderModel(const QString &providerId, const QString &model);

    QString providerBaseUrl(const QString &providerId) const;
    void setProviderBaseUrl(const QString &providerId, const QString &baseUrl);

    bool hasProviderApiKey(const QString &providerId) const;
    bool hasActiveProviderApiKey() const;

    QString outputRootPath() const;
    void setOutputRootPath(const QString &path);
    QString backendOutputRootPath() const;
    void setBackendOutputRootPath(const QString &path);

    QString deepSeekApiKey() const;
    void setDeepSeekApiKey(const QString &apiKey);
    QString deepSeekModel() const;
    void setDeepSeekModel(const QString &model);
    bool hasDeepSeekApiKey() const;
};
