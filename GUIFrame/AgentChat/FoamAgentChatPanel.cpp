#include "FoamAgentChatPanel.h"

#include "AgentChatIntent.h"
#include "AgentController.h"
#include "AgentTypingRenderer.h"
#include "AppSettings.h"
#include "ApiSettingsDialog.h"
#include "ChatHeader.h"
#include "Composer.h"
#include "GateStatusBar.h"
#include "LocalAgentServiceManager.h"
#include "MessageList.h"
#include "RepairHistoryBar.h"
#include "WorkflowActionBar.h"
#include "WorkflowStatusBar.h"
#include "WorkflowWorkspaceBar.h"

#include <QDir>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QTimer>
#include <QUrl>
#include <QVBoxLayout>

#include "FITK_Kernel/FITKCore/FITKActionOperator.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"

namespace {

enum class RepairUiAction
{
    None,
    GenerateCase,
    OpenTaskFolder,
    ReviseRequest,
};

RepairUiAction repairUiActionForId(const QString &actionId)
{
    if (actionId == QStringLiteral("regenerate_case")) {
        return RepairUiAction::GenerateCase;
    }

    if (actionId == QStringLiteral("repair_case_dictionaries")
        || actionId == QStringLiteral("configure_openfoam_runtime")
        || actionId == QStringLiteral("inspect_fatal_log")
        || actionId == QStringLiteral("review_solver_logs")
        || actionId == QStringLiteral("rewrite_manifest")
        || actionId == QStringLiteral("fix_run_pipeline")
        || actionId == QStringLiteral("inspect_solver_numerics")
        || actionId == QStringLiteral("review_mesh_quality")) {
        return RepairUiAction::OpenTaskFolder;
    }

    if (actionId == QStringLiteral("revise_simulation_spec")
        || actionId == QStringLiteral("revise_request_scope")) {
        return RepairUiAction::ReviseRequest;
    }

    return RepairUiAction::None;
}

}

FoamAgentChatPanel::FoamAgentChatPanel(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("foamAgentChatPanel");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_agentController = new AgentController(this);
    m_header = new ChatHeader(this);
    m_workspaceBar = new WorkflowWorkspaceBar(this);
    m_statusBar = new WorkflowStatusBar(this);
    m_actionBar = new WorkflowActionBar(this);
    m_gateStatusBar = new GateStatusBar(this);
    m_repairHistoryBar = new RepairHistoryBar(this);
    m_messageList = new MessageList(this);
    m_composer = new Composer(this);
    m_localServiceManager = new LocalAgentServiceManager(this);
    m_typingRenderer = new AgentTypingRenderer(m_messageList, this);

    layout->addWidget(m_header);
    layout->addWidget(m_workspaceBar);
    layout->addWidget(m_statusBar);
    layout->addWidget(m_actionBar);
    layout->addWidget(m_gateStatusBar);
    layout->addWidget(m_repairHistoryBar);
    layout->addWidget(m_messageList, 1);
    layout->addWidget(m_composer);

    AppSettings settings;
    m_workspaceBar->setOutputDirectory(settings.outputRootPath());
    m_workspaceBar->setBackendOutputDirectory(settings.backendOutputRootPath());
    m_agentController->setHostOutputDirectory(settings.outputRootPath());
    m_agentController->setOutputDirectory(settings.backendOutputRootPath());

    connect(m_composer, &Composer::messageSubmitted,
            this, &FoamAgentChatPanel::handleMessageSubmitted);
    connect(m_workspaceBar, &WorkflowWorkspaceBar::outputDirectoryChanged,
            this, &FoamAgentChatPanel::handleOutputDirectoryChanged);
    connect(m_workspaceBar, &WorkflowWorkspaceBar::backendOutputDirectoryChanged,
            this, &FoamAgentChatPanel::handleBackendOutputDirectoryChanged);
    connect(m_workspaceBar, &WorkflowWorkspaceBar::browseOutputDirectoryRequested,
            this, &FoamAgentChatPanel::handleBrowseOutputDirectoryRequested);
    connect(m_actionBar, &WorkflowActionBar::openFolderRequested,
            this, &FoamAgentChatPanel::handleOpenFolderRequested);
    connect(m_actionBar, &WorkflowActionBar::generateCaseRequested,
            this, &FoamAgentChatPanel::handleGenerateCaseRequested);
    connect(m_actionBar, &WorkflowActionBar::validateCaseRequested,
            this, &FoamAgentChatPanel::handleValidateCaseRequested);
    connect(m_actionBar, &WorkflowActionBar::runCaseRequested,
            this, &FoamAgentChatPanel::handleRunCaseRequested);
    connect(m_actionBar, &WorkflowActionBar::importToAppFlowRequested,
            this, &FoamAgentChatPanel::handleImportToAppFlowRequested);
    connect(m_header, &ChatHeader::settingsRequested,
            this, &FoamAgentChatPanel::handleSettingsRequested);
    connect(m_header, &ChatHeader::stopRequested,
            this, &FoamAgentChatPanel::handleStopRequested);
    connect(m_agentController, &AgentController::agentMessageReceived,
            this, &FoamAgentChatPanel::handleAgentMessageReceived);
    connect(m_agentController, &AgentController::workflowMessageReceived,
            m_messageList, &MessageList::addToolMessage);
    connect(m_agentController, &AgentController::errorMessageReceived,
            m_messageList, &MessageList::addErrorMessage);
    connect(m_agentController, &AgentController::taskRunningChanged,
            m_header, &ChatHeader::setTaskRunning);
    connect(m_agentController, &AgentController::taskRunningChanged,
            m_workspaceBar, &WorkflowWorkspaceBar::setTaskRunning);
    connect(m_agentController, &AgentController::taskRunningChanged,
            m_actionBar, &WorkflowActionBar::setTaskRunning);
    connect(m_agentController, &AgentController::taskContextReceived,
            this, &FoamAgentChatPanel::handleTaskContextReceived);
    connect(m_agentController, &AgentController::caseGenerated,
            this, &FoamAgentChatPanel::handleCaseGenerated);
    connect(m_agentController, &AgentController::caseValidated,
            this, &FoamAgentChatPanel::handleCaseValidated);
    connect(m_agentController, &AgentController::manifestPathReceived,
            this, &FoamAgentChatPanel::handleManifestPathReceived);
    connect(m_agentController, &AgentController::nextActionReceived,
            this, &FoamAgentChatPanel::handleNextActionReceived);
    connect(m_agentController, &AgentController::gateReviewsReceived,
            m_gateStatusBar, &GateStatusBar::setGateReviews);
    connect(m_agentController, &AgentController::repairHistoryReceived,
            m_repairHistoryBar, &RepairHistoryBar::setRepairHistory);
    connect(m_agentController, &AgentController::repairActionReceived,
            this, &FoamAgentChatPanel::handleRepairActionReceived);
    connect(m_messageList, &MessageList::repairActionRequested,
            this, &FoamAgentChatPanel::handleRepairActionRequested);
    setStyleSheet(QStringLiteral(R"(
        QWidget#foamAgentChatPanel {
            background: #f4f7fb;
            color: #172033;
            font-family: "Microsoft YaHei", "Segoe UI", sans-serif;
            font-size: 13px;
        }

        QWidget#chatHeader {
            background: #ffffff;
            border-bottom: 1px solid #dbe3ef;
        }

        QWidget#workflowWorkspaceBar {
            background: #ffffff;
            border-bottom: 1px solid #e4eaf2;
        }

        QWidget#workflowActionBar {
            background: #ffffff;
            border-bottom: 1px solid #e4eaf2;
        }

        QWidget#workflowStatusBar {
            background: #f8fafc;
            border-bottom: 1px solid #e4eaf2;
        }

        QWidget#gateStatusBar {
            background: #ffffff;
            border-bottom: 1px solid #edf1f6;
        }

        QWidget#repairHistoryBar {
            background: #f8fafc;
            border-bottom: 1px solid #edf1f6;
        }

        QLabel#repairHistoryLabel {
            color: #475569;
            font-size: 12px;
            font-weight: 500;
            padding: 4px 10px;
            border: 1px solid #dbe3ef;
            border-radius: 6px;
            background: #ffffff;
        }

        QLabel#gateStatusSummary {
            color: #475569;
            font-size: 12px;
            font-weight: 600;
            min-width: 108px;
        }

        QLabel#gateStatusChip {
            color: #64748b;
            font-size: 12px;
            min-height: 24px;
            padding: 0 8px;
            border: 1px solid #dbe3ef;
            border-radius: 6px;
            background: #ffffff;
        }

        QLabel#gateStatusChip[status="passed"] {
            color: #14532d;
            border-color: #86efac;
            background: #f0fdf4;
        }

        QLabel#gateStatusChip[status="warning"] {
            color: #713f12;
            border-color: #facc15;
            background: #fefce8;
        }

        QLabel#gateStatusChip[status="failed"],
        QLabel#gateStatusChip[status="unsupported"] {
            color: #7f1d1d;
            border-color: #fca5a5;
            background: #fef2f2;
        }

        QLabel#workflowStatusItem {
            color: #334155;
            font-size: 12px;
            padding: 4px 10px;
            border: 1px solid #cbd5e1;
            border-radius: 8px;
            background: #ffffff;
        }

        QLabel#workflowWorkspaceLabel {
            color: #374151;
            font-size: 12px;
            font-weight: 600;
        }

        QLabel#workflowWorkspaceStatus {
            color: #334155;
            font-size: 12px;
            padding: 3px 10px;
            border: 1px solid #cbd5e1;
            border-radius: 8px;
            background: #f8fafc;
        }

        QLabel#chatHeaderTitle {
            color: #0f172a;
            font-size: 17px;
            font-weight: 700;
        }

        QLabel#chatHeaderStatus {
            color: #334155;
            font-size: 12px;
            padding: 3px 10px;
            border: 1px solid #cbd5e1;
            border-radius: 8px;
            background: #f8fafc;
        }

        QPushButton#chatHeaderSettingsButton,
        QPushButton#chatHeaderStopButton,
        QPushButton#composerSendButton,
        QPushButton#workflowWorkspaceBrowseButton,
        QPushButton#workflowActionOpenFolderButton,
        QPushButton#workflowActionGenerateButton,
        QPushButton#workflowActionValidateButton,
        QPushButton#workflowActionRunButton,
        QPushButton#workflowActionImportToAppFlowButton,
        QPushButton#repairActionButton {
            min-height: 32px;
            padding: 0 16px;
            border: 1px solid #cbd5e1;
            border-radius: 6px;
            background: #ffffff;
            color: #172033;
        }

        QPushButton#chatHeaderSettingsButton:hover,
        QPushButton#chatHeaderStopButton:hover,
        QPushButton#composerSendButton:hover,
        QPushButton#workflowWorkspaceBrowseButton:hover,
        QPushButton#workflowActionOpenFolderButton:hover,
        QPushButton#workflowActionGenerateButton:hover,
        QPushButton#workflowActionValidateButton:hover,
        QPushButton#workflowActionRunButton:hover,
        QPushButton#workflowActionImportToAppFlowButton:hover,
        QPushButton#repairActionButton:hover {
            background: #f8fafc;
            border-color: #94a3b8;
        }

        QPushButton#workflowActionGenerateButton {
            background: #16a34a;
            border-color: #16a34a;
            color: #ffffff;
            font-weight: 500;
        }

        QPushButton#workflowActionGenerateButton:hover {
            background: #15803d;
            border-color: #15803d;
        }

        QPushButton#workflowActionValidateButton {
            background: #2563eb;
            border-color: #2563eb;
            color: #ffffff;
            font-weight: 500;
        }

        QPushButton#workflowActionValidateButton:hover {
            background: #1d4ed8;
            border-color: #1d4ed8;
        }

        QPushButton#workflowActionRunButton {
            background: #0f172a;
            border-color: #0f172a;
            color: #ffffff;
            font-weight: 500;
        }

        QPushButton#workflowActionRunButton:hover {
            background: #1e293b;
            border-color: #1e293b;
        }

        QPushButton#workflowActionImportToAppFlowButton {
            background: #7c3aed;
            border-color: #7c3aed;
            color: #ffffff;
            font-weight: 500;
        }

        QPushButton#workflowActionImportToAppFlowButton:hover {
            background: #6d28d9;
            border-color: #6d28d9;
        }

        QPushButton#workflowActionGenerateButton:disabled,
        QPushButton#workflowActionValidateButton:disabled,
        QPushButton#workflowActionRunButton:disabled,
        QPushButton#workflowActionImportToAppFlowButton:disabled,
        QPushButton#workflowActionOpenFolderButton:disabled,
        QPushButton#repairActionButton:disabled {
            color: #9ca3af;
            background: #f9fafb;
            border-color: #d1d5db;
        }

        QPushButton#chatHeaderStopButton:disabled {
            color: #9ca3af;
            background: #f9fafb;
        }

        QPushButton#composerSendButton {
            background: #2563eb;
            border-color: #2563eb;
            color: #ffffff;
            font-weight: 500;
        }

        QPushButton#composerSendButton:hover {
            background: #1d4ed8;
            border-color: #1d4ed8;
        }

        QPushButton#repairActionButton {
            background: #16a34a;
            border-color: #16a34a;
            color: #ffffff;
            font-weight: 500;
        }

        QPushButton#repairActionButton:hover {
            background: #15803d;
            border-color: #15803d;
        }

        QPushButton#repairActionButton:disabled {
            color: #9ca3af;
            background: #f9fafb;
            border-color: #d1d5db;
        }

        QWidget#messageList,
        QScrollArea#messageScrollArea {
            background: #f4f7fb;
            border: none;
        }

        QLabel#messageSender {
            color: #64748b;
            font-size: 11px;
            font-weight: 600;
            padding-left: 2px;
            padding-right: 2px;
        }

        QLabel#messageSender[role="user"] {
            color: #2563eb;
        }

        QLabel#messageBubble {
            background: #ffffff;
            border: 1px solid #dbe3ef;
            border-radius: 8px;
            padding: 11px 13px;
            color: #172033;
            line-height: 1.4;
        }

        QLabel#messageBubble[role="user"] {
            background: #2563eb;
            border-color: #2563eb;
            color: #ffffff;
        }

        QLabel#messageBubble[role="agent"] {
            background: #ffffff;
            border-color: #dbe3ef;
        }

        QLabel#messageBubble[role="tool"] {
            background: #f0fdf4;
            border-color: #bbf7d0;
            color: #14532d;
        }

        QLabel#messageBubble[role="error"] {
            background: #fef2f2;
            border-color: #fecaca;
        }

        QWidget#composer {
            background: #ffffff;
            border-top: 1px solid #dbe3ef;
        }

        QLineEdit#composerInput,
        QLineEdit#workflowWorkspacePath {
            min-height: 36px;
            padding: 0 11px;
            border: 1px solid #cbd5e1;
            border-radius: 6px;
            background: #ffffff;
            selection-background-color: #bfdbfe;
        }

        QLineEdit#composerInput:focus,
        QLineEdit#workflowWorkspacePath:focus {
            border-color: #2563eb;
            background: #ffffff;
        }
    )"));

    QTimer::singleShot(0, this, [this]() {
        ensureLocalAgentService();
    });
}

void FoamAgentChatPanel::ensureLocalAgentService()
{
    m_messageList->addToolMessage(QString::fromUtf8("\xE6\xAD\xA3\xE5\x9C\xA8\xE5\x90\xAF\xE5\x8A\xA8\x20\x41\x67\x65\x6E\x74\x20\xE5\x90\x8E\xE7\xAB\xAF\xE6\x9C\x8D\xE5\x8A\xA1\x2E\x2E\x2E"));

    QString errorMessage;
    if (m_localServiceManager->ensureServiceRunning(&errorMessage)) {
        m_messageList->addToolMessage(QString::fromUtf8("\x41\x67\x65\x6E\x74\x20\xE5\x90\x8E\xE7\xAB\xAF\xE6\x9C\x8D\xE5\x8A\xA1\xE5\xB7\xB2\xE5\xB0\xB1\xE7\xBB\xAA\xE3\x80\x82"));
        return;
    }

    m_messageList->addErrorMessage(QString::fromUtf8("\x41\x67\x65\x6E\x74\x20\xE5\x90\x8E\xE7\xAB\xAF\xE6\x9C\x8D\xE5\x8A\xA1\xE5\x90\xAF\xE5\x8A\xA8\xE5\xA4\xB1\xE8\xB4\xA5\xEF\xBC\x9A\x25\x31").arg(errorMessage));
}

void FoamAgentChatPanel::handleMessageSubmitted(const QString &text)
{
    if (!m_currentNextActionId.isEmpty() && AgentChatIntent::isNextActionConfirmation(text)) {
        m_messageList->addUserMessage(text);
        executeCurrentNextAction();
        return;
    }

    if (!m_currentBackendTaskDir.isEmpty() && AgentChatIntent::isParameterChangeRequest(text)) {
        m_messageList->addUserMessage(text);
        requestParameterReplan(text);
        return;
    }

    const QString outputDirectory = m_workspaceBar->outputDirectory();
    QDir().mkpath(outputDirectory);
    const QString backendOutputDirectory = m_workspaceBar->backendOutputDirectory();

    m_messageList->addUserMessage(text);
    m_messageList->addToolMessage(QString::fromUtf8("\xE6\xAD\xA3\xE5\x9C\xA8\xE8\xA7\x84\xE5\x88\x92\xE7\xAE\x97\xE4\xBE\x8B\x2E\x2E\x2E"));
    m_currentBackendTaskDir.clear();
    m_currentHostTaskDir.clear();
    m_latestManifestPath.clear();
    m_currentRepairActionId.clear();
    m_currentRepairAction = {};
    clearCurrentNextAction();
    m_gateStatusBar->clear();
    m_repairHistoryBar->clear();
    m_messageList->setRepairMessage(QString());
    m_statusBar->reset();
    m_statusBar->setStageText(QString::fromUtf8("\xE8\xA7\x84\xE5\x88\x92\xE4\xB8\xAD"));
    m_actionBar->setGenerateCaseEnabled(false);
    m_actionBar->setValidateCaseEnabled(false);
    m_actionBar->setRunCaseEnabled(false);
    m_actionBar->setImportToAppFlowEnabled(false);
    m_agentController->setHostOutputDirectory(outputDirectory);
    m_agentController->setOutputDirectory(backendOutputDirectory);
    m_agentController->submitUserMessage(text);
}

void FoamAgentChatPanel::handleAgentMessageReceived(const QString &text)
{
    m_typingRenderer->queueMessage(text);
}

QString FoamAgentChatPanel::hostTaskDirectoryForBackendTask(const QString &backendTaskDir) const
{
    const QString normalizedBackendTaskDir = backendTaskDir.trimmed().replace('\\', '/');
    const QString taskName = normalizedBackendTaskDir.section('/', -1, -1);
    if (taskName.isEmpty()) {
        return QString();
    }

    return QDir(m_workspaceBar->outputDirectory()).filePath(taskName);
}

void FoamAgentChatPanel::handleTaskContextReceived(const QString &taskDir)
{
    const bool newTask = m_currentBackendTaskDir != taskDir;
    m_currentBackendTaskDir = taskDir;
    m_currentHostTaskDir = hostTaskDirectoryForBackendTask(taskDir);
    m_statusBar->setTaskId(AgentChatIntent::taskIdFromDirectory(taskDir));
    if (newTask) {
        m_statusBar->setStageText(QString::fromUtf8("\xE5\xB7\xB2\xE8\xA7\x84\xE5\x88\x92"));
    }
    m_actionBar->setGenerateCaseEnabled(!m_currentBackendTaskDir.isEmpty());
}

void FoamAgentChatPanel::handleRepairActionReceived(const QJsonObject &action)
{
    const QString actionId = action.value(QStringLiteral("id")).toString().trimmed();
    const QString label = action.value(QStringLiteral("label")).toString().trimmed();
    const QString description = action.value(QStringLiteral("description")).toString().trimmed();
    if (label.isEmpty() && description.isEmpty()) {
        m_currentRepairActionId.clear();
        m_currentRepairAction = {};
        m_messageList->setRepairMessage(QString());
        return;
    }

    m_currentRepairActionId = actionId;
    m_currentRepairAction = action;

    QStringList lines;
    if (!label.isEmpty()) {
        lines << QString::fromUtf8("\xE4\xB8\x8B\xE4\xB8\x80\xE4\xB8\xAA\xE4\xBF\xAE\xE5\xA4\x8D\xE5\x8A\xA8\xE4\xBD\x9C\xEF\xBC\x9A\x25\x31").arg(label);
    }
    if (!description.isEmpty()) {
        lines << description;
    }
    m_messageList->setRepairMessage(lines.join(QStringLiteral("\n")), label);
}

void FoamAgentChatPanel::handleRepairActionRequested()
{
    const RepairUiAction action = repairUiActionForId(m_currentRepairActionId);
    if (action == RepairUiAction::GenerateCase) {
        requestGenerateCase(m_currentRepairAction);
        return;
    }

    if (!m_currentBackendTaskDir.isEmpty() && !m_currentRepairAction.isEmpty()) {
        m_agentController->recordRepairActionForTask(m_currentBackendTaskDir, m_currentRepairAction);
    }

    if (action == RepairUiAction::OpenTaskFolder) {
        handleOpenFolderRequested();
        return;
    }

    if (action == RepairUiAction::ReviseRequest) {
        m_messageList->addToolMessage(QString::fromUtf8("\xE8\xAF\xB7\xE5\x9C\xA8\xE8\xBE\x93\xE5\x85\xA5\xE6\xA1\x86\xE4\xB8\xAD\xE4\xBF\xAE\xE6\x94\xB9\xE9\x9C\x80\xE6\xB1\x82\xEF\xBC\x8C\xE7\x84\xB6\xE5\x90\x8E\xE9\x87\x8D\xE6\x96\xB0\xE7\x94\x9F\xE6\x88\x90\xE7\xAE\x97\xE4\xBE\x8B\xE3\x80\x82"));
        return;
    }

    m_messageList->addToolMessage(QString::fromUtf8("\xE5\xBD\x93\xE5\x89\x8D\xE4\xBF\xAE\xE5\xA4\x8D\xE5\xBB\xBA\xE8\xAE\xAE\xE6\xB2\xA1\xE6\x9C\x89\xE7\x9B\xB4\xE6\x8E\xA5\x20\x55\x49\x20\xE5\x8A\xA8\xE4\xBD\x9C\xE3\x80\x82"));
}

void FoamAgentChatPanel::handleCaseGenerated(const QString &taskDir)
{
    if (taskDir == m_currentBackendTaskDir) {
        m_statusBar->setStageText(QString::fromUtf8("\xE5\xB7\xB2\xE7\x94\x9F\xE6\x88\x90"));
        m_actionBar->setValidateCaseEnabled(true);
    }
}

void FoamAgentChatPanel::handleCaseValidated(const QString &taskDir)
{
    if (taskDir == m_currentBackendTaskDir) {
        m_statusBar->setStageText(QString::fromUtf8("\xE5\xB7\xB2\xE6\xA0\xA1\xE9\xAA\x8C"));
        m_actionBar->setRunCaseEnabled(true);
    }
}

void FoamAgentChatPanel::handleOpenFolderRequested()
{
    const QString folderPath = m_currentHostTaskDir.isEmpty()
        ? m_workspaceBar->outputDirectory()
        : m_currentHostTaskDir;

    QDir().mkpath(folderPath);
    const QUrl folderUrl = QUrl::fromLocalFile(folderPath);
    if (!QDesktopServices::openUrl(folderUrl)) {
        m_messageList->addErrorMessage(QString::fromUtf8("\xE6\x89\x93\xE5\xBC\x80\xE7\x9B\xAE\xE5\xBD\x95\xE5\xA4\xB1\xE8\xB4\xA5\x3A\x0A\x25\x31").arg(folderPath));
    }
}

void FoamAgentChatPanel::handleGenerateCaseRequested()
{
    requestGenerateCase();
}

void FoamAgentChatPanel::requestGenerateCase(const QJsonObject &repairAction)
{
    if (m_currentBackendTaskDir.isEmpty()) {
        m_messageList->addErrorMessage(QString::fromUtf8("\xE5\xBD\x93\xE5\x89\x8D\xE6\xB2\xA1\xE6\x9C\x89\xE5\xB7\xB2\xE8\xA7\x84\xE5\x88\x92\xE4\xBB\xBB\xE5\x8A\xA1\xEF\xBC\x8C\xE8\xAF\xB7\xE5\x85\x88\xE5\x8F\x91\xE9\x80\x81\xE9\x9C\x80\xE6\xB1\x82\xE8\xBF\x9B\xE8\xA1\x8C\xE8\xA7\x84\xE5\x88\x92\xE3\x80\x82"));
        return;
    }

    m_messageList->addToolMessage(QString::fromUtf8("\xE6\xAD\xA3\xE5\x9C\xA8\xE7\x94\x9F\xE6\x88\x90\xE7\xAE\x97\xE4\xBE\x8B\xE6\x96\x87\xE4\xBB\xB6\x2E\x2E\x2E"));
    m_statusBar->setStageText(QString::fromUtf8("\xE7\x94\x9F\xE6\x88\x90\xE4\xB8\xAD"));
    m_currentRepairActionId.clear();
    m_currentRepairAction = {};
    m_messageList->setRepairMessage(QString());
    m_agentController->generateCaseForTask(m_currentBackendTaskDir, repairAction);
}

void FoamAgentChatPanel::handleValidateCaseRequested()
{
    if (m_currentBackendTaskDir.isEmpty()) {
        m_messageList->addErrorMessage(QString::fromUtf8("\xE5\xBD\x93\xE5\x89\x8D\xE6\xB2\xA1\xE6\x9C\x89\xE5\xB7\xB2\xE7\x94\x9F\xE6\x88\x90\xE4\xBB\xBB\xE5\x8A\xA1\xEF\xBC\x8C\xE8\xAF\xB7\xE5\x85\x88\xE7\x94\x9F\xE6\x88\x90\xE7\xAE\x97\xE4\xBE\x8B\xE3\x80\x82"));
        return;
    }

    m_messageList->addToolMessage(QString::fromUtf8("\xE6\xAD\xA3\xE5\x9C\xA8\xE6\xA0\xA1\xE9\xAA\x8C\xE7\xAE\x97\xE4\xBE\x8B\x2E\x2E\x2E"));
    m_statusBar->setStageText(QString::fromUtf8("\xE6\xA0\xA1\xE9\xAA\x8C\xE4\xB8\xAD"));
    m_agentController->validateCaseForTask(m_currentBackendTaskDir);
}

void FoamAgentChatPanel::handleRunCaseRequested()
{
    if (m_currentBackendTaskDir.isEmpty()) {
        m_messageList->addErrorMessage(QString::fromUtf8("\xE5\xBD\x93\xE5\x89\x8D\xE6\xB2\xA1\xE6\x9C\x89\xE5\xB7\xB2\xE6\xA0\xA1\xE9\xAA\x8C\xE4\xBB\xBB\xE5\x8A\xA1\xEF\xBC\x8C\xE8\xAF\xB7\xE5\x85\x88\xE6\xA0\xA1\xE9\xAA\x8C\xE7\xAE\x97\xE4\xBE\x8B\xE3\x80\x82"));
        return;
    }

    m_messageList->addToolMessage(QString::fromUtf8("\xE6\xAD\xA3\xE5\x9C\xA8\xE8\xBF\x90\xE8\xA1\x8C\x20\x4F\x70\x65\x6E\x46\x4F\x41\x4D\x2E\x2E\x2E"));
    m_statusBar->setStageText(QString::fromUtf8("\xE8\xBF\x90\xE8\xA1\x8C\xE4\xB8\xAD"));
    m_agentController->runCaseForTask(m_currentBackendTaskDir);
}

void FoamAgentChatPanel::handleManifestPathReceived(const QString &manifestPath)
{
    m_latestManifestPath = QDir::toNativeSeparators(manifestPath);
    m_statusBar->setManifestReady(!m_latestManifestPath.isEmpty());
    m_actionBar->setImportToAppFlowEnabled(!m_latestManifestPath.isEmpty());
    m_messageList->addToolMessage(QString::fromUtf8("\xE5\xB7\xB2\xE6\x94\xB6\xE5\x88\xB0\x20\x41\x67\x65\x6E\x74\x20\xE6\xB8\x85\xE5\x8D\x95\xEF\xBC\x9A\x25\x31").arg(m_latestManifestPath));
}

void FoamAgentChatPanel::handleImportToAppFlowRequested()
{
    if (m_latestManifestPath.isEmpty()) {
        m_messageList->addErrorMessage(QString::fromUtf8("\xE5\xBD\x93\xE5\x89\x8D\xE6\xB2\xA1\xE6\x9C\x89\xE5\x8F\xAF\xE5\xAF\xBC\xE5\x85\xA5\xE7\x9A\x84\x20\x41\x67\x65\x6E\x74\x20\xE6\xB8\x85\xE5\x8D\x95\xE3\x80\x82"));
        return;
    }

    if (!QFileInfo(m_latestManifestPath).isFile()) {
        m_messageList->addErrorMessage(QString::fromUtf8("\x41\x67\x65\x6E\x74\x20\xE6\xB8\x85\xE5\x8D\x95\xE5\xAF\xBC\xE5\x85\xA5\xE5\xA4\xB1\xE8\xB4\xA5\xEF\xBC\x9A\x25\x31").arg(m_latestManifestPath));
        return;
    }

    Core::FITKActionOperator* oper = FITKOPERREPO->getOperatorT<Core::FITKActionOperator>(QStringLiteral("actionLoadAgentManifest"));
    if (oper == nullptr) {
        m_messageList->addErrorMessage(QString::fromUtf8("\x41\x67\x65\x6E\x74\x20\xE6\xB8\x85\xE5\x8D\x95\xE5\xAF\xBC\xE5\x85\xA5\xE6\x93\x8D\xE4\xBD\x9C\xE6\x9C\xAA\xE6\x89\xBE\xE5\x88\xB0\xE3\x80\x82"));
        return;
    }

    m_messageList->addToolMessage(QString::fromUtf8("\xE6\xAD\xA3\xE5\x9C\xA8\xE5\xAF\xBC\xE5\x85\xA5\x20\x41\x67\x65\x6E\x74\x20\xE6\xB8\x85\xE5\x8D\x95\xE5\x88\xB0\x20\x41\x50\x50\x46\x6C\x6F\x77\xEF\xBC\x9A\x25\x31").arg(m_latestManifestPath));
    m_statusBar->setStageText(QString::fromUtf8("\xE5\xAF\xBC\xE5\x85\xA5\xE4\xB8\xAD"));
    oper->setArgs("FileName", m_latestManifestPath);
    if (oper->execProfession()) {
        clearCurrentNextAction();
        m_statusBar->setStageText(QString::fromUtf8("\xE5\xB7\xB2\xE5\xAE\x8C\xE6\x88\x90"));
        m_messageList->addToolMessage(QString::fromUtf8("\x41\x67\x65\x6E\x74\x20\xE6\xB8\x85\xE5\x8D\x95\xE5\xAF\xBC\xE5\x85\xA5\xE5\xB7\xB2\xE5\xAE\x8C\xE6\x88\x90\xE3\x80\x82"));
    }
    else {
        m_statusBar->setStageText(QString::fromUtf8("\xE5\xAF\xBC\xE5\x85\xA5\xE5\xA4\xB1\xE8\xB4\xA5"));
        m_messageList->addErrorMessage(QString::fromUtf8("\x41\x67\x65\x6E\x74\x20\xE6\xB8\x85\xE5\x8D\x95\xE5\xAF\xBC\xE5\x85\xA5\xE5\xA4\xB1\xE8\xB4\xA5\xEF\xBC\x9A\x25\x31").arg(m_latestManifestPath));
    }
}

void FoamAgentChatPanel::handleNextActionReceived(const QString &id, const QString &label, const QString &endpoint, const QString &text)
{
    if (text.trimmed().isEmpty()) {
        return;
    }

    m_currentNextActionId = id.trimmed();
    m_currentNextActionLabel = label.trimmed();
    m_currentNextActionEndpoint = endpoint.trimmed();
    m_statusBar->setNextActionText(AgentChatIntent::nextActionTitle(m_currentNextActionLabel, m_currentNextActionId));
    if (m_currentNextActionId == QStringLiteral("generate")) {
        m_statusBar->setStageText(QString::fromUtf8("\xE5\xB7\xB2\xE8\xA7\x84\xE5\x88\x92"));
    }
    if (m_currentNextActionId == QStringLiteral("import_manifest")) {
        m_statusBar->setStageText(QString::fromUtf8("\xE5\xB7\xB2\xE8\xBF\x90\xE8\xA1\x8C"));
    }
    m_messageList->addToolMessage(text);
}

void FoamAgentChatPanel::requestParameterReplan(const QString &message)
{
    if (m_agentController->isRunning()) {
        m_messageList->addErrorMessage(QString::fromUtf8("\xE5\xBD\x93\xE5\x89\x8D\x20\x41\x67\x65\x6E\x74\x20\xE6\xAD\xA3\xE5\x9C\xA8\xE6\x89\xA7\xE8\xA1\x8C\xE4\xBB\xBB\xE5\x8A\xA1\xEF\xBC\x8C\xE8\xAF\xB7\xE7\xAD\x89\xE5\xBE\x85\xE5\xAE\x8C\xE6\x88\x90\xE5\x90\x8E\xE5\x86\x8D\xE4\xBF\xAE\xE6\x94\xB9\xE5\x8F\x82\xE6\x95\xB0\xE3\x80\x82"));
        return;
    }

    if (m_currentBackendTaskDir.isEmpty()) {
        m_messageList->addErrorMessage(QString::fromUtf8("\xE5\xBD\x93\xE5\x89\x8D\xE6\xB2\xA1\xE6\x9C\x89\xE5\xB7\xB2\xE8\xA7\x84\xE5\x88\x92\xE4\xBB\xBB\xE5\x8A\xA1\xEF\xBC\x8C\xE8\xAF\xB7\xE5\x85\x88\xE5\x8F\x91\xE9\x80\x81\xE9\x9C\x80\xE6\xB1\x82\xE8\xBF\x9B\xE8\xA1\x8C\xE8\xA7\x84\xE5\x88\x92\xE3\x80\x82"));
        return;
    }

    m_messageList->addToolMessage(QString::fromUtf8("\xE5\xB7\xB2\xE6\xA3\x80\xE6\xB5\x8B\xE5\x88\xB0\xE5\x8F\x82\xE6\x95\xB0\xE4\xBF\xAE\xE6\x94\xB9\xE8\xAF\xB7\xE6\xB1\x82\xEF\xBC\x8C\xE6\xAD\xA3\xE5\x9C\xA8\xE4\xBA\xA4\xE7\xBB\x99\xE5\x90\x8E\xE7\xAB\xAF\x20\x41\x67\x65\x6E\x74\x20\xE9\x87\x8D\xE6\x96\xB0\xE8\xA7\x84\xE5\x88\x92\x2E\x2E\x2E"));
    m_agentController->setHostOutputDirectory(m_workspaceBar->outputDirectory());
    m_agentController->setOutputDirectory(m_workspaceBar->backendOutputDirectory());
    m_agentController->replanTaskForMessage(m_currentBackendTaskDir, message);
}

void FoamAgentChatPanel::executeCurrentNextAction()
{
    if (m_agentController->isRunning()) {
        m_messageList->addErrorMessage(QString::fromUtf8("\xE5\xBD\x93\xE5\x89\x8D\x20\x41\x67\x65\x6E\x74\x20\xE6\xAD\xA3\xE5\x9C\xA8\xE6\x89\xA7\xE8\xA1\x8C\xE4\xBB\xBB\xE5\x8A\xA1\xEF\xBC\x8C\xE8\xAF\xB7\xE7\xAD\x89\xE5\xBE\x85\xE5\xAE\x8C\xE6\x88\x90\xE3\x80\x82"));
        return;
    }

    const QString actionId = m_currentNextActionId;
    const QString title = AgentChatIntent::nextActionTitle(m_currentNextActionLabel, actionId);
    clearCurrentNextAction();

    if (!title.isEmpty()) {
        m_messageList->addToolMessage(QString::fromUtf8("\xE6\xAD\xA3\xE5\x9C\xA8\xE6\x89\xA7\xE8\xA1\x8C\xE5\xBB\xBA\xE8\xAE\xAE\xE4\xB8\x8B\xE4\xB8\x80\xE6\xAD\xA5\xEF\xBC\x9A\x25\x31").arg(title));
    }

    if (actionId == QStringLiteral("generate")) {
        handleGenerateCaseRequested();
        return;
    }
    if (actionId == QStringLiteral("validate")) {
        handleValidateCaseRequested();
        return;
    }
    if (actionId == QStringLiteral("run")) {
        handleRunCaseRequested();
        return;
    }
    if (actionId == QStringLiteral("import_manifest")) {
        handleImportToAppFlowRequested();
        return;
    }

    m_messageList->addErrorMessage(QString::fromUtf8("\xE6\x9C\xAA\xE7\x9F\xA5\xE7\x9A\x84\xE5\xBB\xBA\xE8\xAE\xAE\xE5\x8A\xA8\xE4\xBD\x9C\xEF\xBC\x9A\x25\x31").arg(actionId));
}

void FoamAgentChatPanel::clearCurrentNextAction()
{
    m_currentNextActionId.clear();
    m_currentNextActionLabel.clear();
    m_currentNextActionEndpoint.clear();
    m_statusBar->setNextActionText(QString());
}

void FoamAgentChatPanel::handleOutputDirectoryChanged(const QString &path)
{
    AppSettings settings;
    const QString previousOutputRoot = settings.outputRootPath();
    const QString previousBackendOutputRoot = settings.backendOutputRootPath();
    const bool backendWasFollowingHost = previousBackendOutputRoot == previousOutputRoot;

    settings.setOutputRootPath(path);
    m_agentController->setHostOutputDirectory(path);

    if (m_workspaceBar->backendOutputDirectory().isEmpty() || backendWasFollowingHost) {
        settings.setBackendOutputRootPath(path);
        m_workspaceBar->setBackendOutputDirectory(settings.backendOutputRootPath());
    }
}

void FoamAgentChatPanel::handleBackendOutputDirectoryChanged(const QString &path)
{
    AppSettings settings;
    settings.setBackendOutputRootPath(path);
    m_agentController->setOutputDirectory(settings.backendOutputRootPath());
}

void FoamAgentChatPanel::handleBrowseOutputDirectoryRequested()
{
    const QString selected = QFileDialog::getExistingDirectory(
        this,
        QString::fromUtf8("\xE9\x80\x89\xE6\x8B\xA9\x20\x46\x6F\x61\x6D\x41\x67\x65\x6E\x74\x20\xE8\xBE\x93\xE5\x87\xBA\xE7\x9B\xAE\xE5\xBD\x95"),
        m_workspaceBar->outputDirectory());

    if (selected.isEmpty()) {
        return;
    }

    m_workspaceBar->setOutputDirectory(QDir::toNativeSeparators(selected));
}

void FoamAgentChatPanel::handleSettingsRequested()
{
    ApiSettingsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        ensureLocalAgentService();
    }
}

void FoamAgentChatPanel::handleStopRequested()
{
    if (!m_agentController->isRunning()) {
        return;
    }

    m_agentController->cancelCurrentTask();
    m_typingRenderer->queueMessage(QString::fromUtf8("\xE5\xBD\x93\xE5\x89\x8D\xE4\xBB\xBB\xE5\x8A\xA1\xE5\xB7\xB2\xE5\x8F\x96\xE6\xB6\x88\xE3\x80\x82"));
}
