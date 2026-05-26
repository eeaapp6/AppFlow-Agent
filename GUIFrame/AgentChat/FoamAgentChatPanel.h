#pragma once

#include <QJsonObject>
#include <QString>
#include <QWidget>

class AgentController;
class AgentTypingRenderer;
class ChatHeader;
class Composer;
class GateStatusBar;
class LocalAgentServiceManager;
class MessageList;
class RepairHistoryBar;
class WorkflowActionBar;
class WorkflowStatusBar;
class WorkflowWorkspaceBar;

class FoamAgentChatPanel : public QWidget
{
    Q_OBJECT

public:
    explicit FoamAgentChatPanel(QWidget *parent = nullptr);

private:
    QString hostTaskDirectoryForBackendTask(const QString &backendTaskDir) const;

    void handleMessageSubmitted(const QString &text);
    void handleTaskContextReceived(const QString &taskDir);
    void handleRepairActionReceived(const QJsonObject &action);
    void handleRepairActionRequested();
    void handleCaseGenerated(const QString &taskDir);
    void handleCaseValidated(const QString &taskDir);
    void handleOpenFolderRequested();
    void handleGenerateCaseRequested();
    void requestGenerateCase(const QJsonObject &repairAction = {});
    void handleValidateCaseRequested();
    void handleRunCaseRequested();
    void handleManifestPathReceived(const QString &manifestPath);
    void handleImportToAppFlowRequested();
    void handleNextActionReceived(const QString &id, const QString &label, const QString &endpoint, const QString &text);
    void handleAgentMessageReceived(const QString &text);
    void requestParameterReplan(const QString &message);
    void executeCurrentNextAction();
    void clearCurrentNextAction();
    void handleOutputDirectoryChanged(const QString &path);
    void handleBackendOutputDirectoryChanged(const QString &path);
    void handleBrowseOutputDirectoryRequested();
    void handleSettingsRequested();
    void handleStopRequested();
    void ensureLocalAgentService();

private:
    AgentController *m_agentController = nullptr;
    ChatHeader *m_header = nullptr;
    WorkflowWorkspaceBar *m_workspaceBar = nullptr;
    WorkflowStatusBar *m_statusBar = nullptr;
    WorkflowActionBar *m_actionBar = nullptr;
    GateStatusBar *m_gateStatusBar = nullptr;
    RepairHistoryBar *m_repairHistoryBar = nullptr;
    MessageList *m_messageList = nullptr;
    Composer *m_composer = nullptr;
    LocalAgentServiceManager *m_localServiceManager = nullptr;
    AgentTypingRenderer *m_typingRenderer = nullptr;
    QString m_currentBackendTaskDir;
    QString m_currentHostTaskDir;
    QString m_latestManifestPath;
    QString m_currentNextActionId;
    QString m_currentNextActionLabel;
    QString m_currentNextActionEndpoint;
    QString m_currentRepairActionId;
    QJsonObject m_currentRepairAction;
};
