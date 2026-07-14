#pragma once

#include <QByteArray>
#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>

class QNetworkAccessManager;
class QNetworkReply;

class AgentController : public QObject
{
    Q_OBJECT

public:
    explicit AgentController(QObject *parent = nullptr);

    bool isRunning() const;
    void setOutputDirectory(const QString &path);
    void setHostOutputDirectory(const QString &path);
    void submitUserMessage(const QString &text);
    void generateCaseForTask(const QString &taskDir, const QJsonObject &repairAction = {});
    void validateCaseForTask(const QString &taskDir);
    void runCaseForTask(const QString &taskDir);
    void replanTaskForMessage(const QString &taskDir, const QString &message);
    void recordRepairActionForTask(const QString &taskDir, const QJsonObject &repairAction);

signals:
    void agentMessageReceived(const QString &text);
    void workflowMessageReceived(const QString &text);
    void errorMessageReceived(const QString &text);
    void taskRunningChanged(bool running);
    void taskContextReceived(const QString &taskDir);
    void caseGenerated(const QString &taskDir);
    void caseValidated(const QString &taskDir);
    void manifestReadinessReceived(const QString &manifestPath, bool importReady, const QString &blockerMessage);
    void workflowActionsReceived(const QStringList &actions);
    void nextActionReceived(const QString &id, const QString &label, const QString &endpoint, const QString &text);
    void gateReviewsReceived(const QJsonArray &reviews);
    void repairHistoryReceived(const QJsonArray &history);
    void repairActionReceived(const QJsonObject &action);
    void workflowActionStarted(const QString &action);
    void workflowStatusReceived(const QString &status);
    void workflowActionUncertain(const QString &action);

private:
    bool rejectIfRequestBusy();
    void invalidateManifestReadiness();
    void invalidateWorkflowActions();
    void synchronizeWorkflowActions(const QByteArray &body);
    void postJson(const QString &path, const QJsonObject &payload);
    void handleReplyFinished(QNetworkReply *reply);
    void handleRepairActionReplyFinished(QNetworkReply *reply);
    QString hostPathForBackendPath(const QString &backendPath) const;

    QNetworkAccessManager *m_network = nullptr;
    QNetworkReply *m_currentReply = nullptr;
    QString m_currentRequestPath;
    QString m_outputDirectory;
    QString m_hostOutputDirectory;
};
