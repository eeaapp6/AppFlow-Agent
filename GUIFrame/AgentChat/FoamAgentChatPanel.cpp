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
#include <QRegularExpression>
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

QString normalizedManifestPath(const QString &path)
{
    const QString trimmed = path.trimmed();
    if (trimmed.isEmpty()) {
        return QString();
    }
    return QDir::cleanPath(QDir::fromNativeSeparators(trimmed));
}

bool manifestPathsMatch(const QString &left, const QString &right)
{
    const QString normalizedLeft = normalizedManifestPath(left);
    const QString normalizedRight = normalizedManifestPath(right);
    if (normalizedLeft.isEmpty() || normalizedRight.isEmpty()) {
        return false;
    }

    const bool windowsDrivePath = normalizedLeft.size() > 1
        && normalizedRight.size() > 1
        && normalizedLeft.at(1) == QLatin1Char(':')
        && normalizedRight.at(1) == QLatin1Char(':');
    return normalizedLeft.compare(
        normalizedRight,
        windowsDrivePath ? Qt::CaseInsensitive : Qt::CaseSensitive) == 0;
}

QString safeImportResultMessage(const QString &message)
{
    QString text = message.trimmed();
    static const QRegularExpression sensitiveValue(
        QStringLiteral("(api[_-]?key|authorization|bearer)[^\\r\\n]*"),
        QRegularExpression::CaseInsensitiveOption);
    text.replace(sensitiveValue, QStringLiteral("\\1 [redacted]"));
    const int maximumLength = 500;
    if (text.size() > maximumLength) {
        text = text.left(maximumLength - 3).trimmed() + QStringLiteral("...");
    }
    return text;
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
    connect(m_agentController, &AgentController::taskRunningChanged,
            this, [this](bool running) {
                m_agentRequestRunning = running;
                refreshInteractionAvailability();
            });
    connect(m_agentController, &AgentController::taskContextReceived,
            this, &FoamAgentChatPanel::handleTaskContextReceived);
    connect(m_agentController, &AgentController::workflowActionStarted,
            m_statusBar, &WorkflowStatusBar::beginWorkflowAction);
    connect(m_agentController, &AgentController::workflowActionStarted,
            m_gateStatusBar, &GateStatusBar::clearRequestUncertainty);
    connect(m_agentController, &AgentController::workflowStatusReceived,
            m_statusBar, &WorkflowStatusBar::applyBackendStatus);
    connect(m_agentController, &AgentController::workflowActionUncertain,
            m_statusBar, &WorkflowStatusBar::markWorkflowActionUncertain);
    connect(m_agentController, &AgentController::workflowActionUncertain,
            m_gateStatusBar, &GateStatusBar::setRequestUncertain);
    connect(m_agentController, &AgentController::manifestReadinessReceived,
            this, &FoamAgentChatPanel::handleManifestReadinessReceived);
    connect(m_agentController, &AgentController::workflowActionsReceived,
            this, &FoamAgentChatPanel::handleWorkflowActionsReceived);
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

        QFrame#workflowFailureCard {
            border: 1px solid #fca5a5;
            border-radius: 8px;
            background: #fef2f2;
        }

        QFrame#workflowFailureCard[severity="error"] {
            border-color: #f87171;
            background: #fef2f2;
        }

        QFrame#workflowFailureCard[severity="blocked"] {
            border-color: #fb923c;
            background: #fff7ed;
        }

        QFrame#workflowFailureCard[severity="unsupported"] {
            border-color: #c2410c;
            background: #fff7ed;
        }

        QFrame#workflowFailureCard[severity="uncertain"] {
            border-color: #eab308;
            background: #fefce8;
        }

        QLabel#workflowFailureStage {
            color: #7f1d1d;
            font-size: 12px;
            font-weight: 700;
        }

        QFrame#workflowFailureCard[severity="blocked"] QLabel#workflowFailureStage,
        QFrame#workflowFailureCard[severity="unsupported"] QLabel#workflowFailureStage {
            color: #9a3412;
        }

        QFrame#workflowFailureCard[severity="uncertain"] QLabel#workflowFailureStage {
            color: #854d0e;
        }

        QLabel#workflowFailureTitle {
            color: #172033;
            font-size: 14px;
            font-weight: 700;
        }

        QLabel#workflowFailureSummary,
        QLabel#workflowFailureIssue,
        QLabel#workflowFailureEvidence,
        QLabel#workflowFailureMore {
            color: #334155;
            font-size: 12px;
        }

        QLabel#workflowFailureIssue {
            padding-left: 8px;
        }

        QLabel#workflowFailureEvidence {
            color: #475569;
            padding-top: 3px;
        }

        QLabel#workflowFailureMore {
            color: #64748b;
            font-style: italic;
        }

        QLabel#workflowStatusItem {
            color: #334155;
            font-size: 12px;
            padding: 4px 10px;
            border: 1px solid #cbd5e1;
            border-radius: 8px;
            background: #ffffff;
        }

        QLabel#workflowProgressStep {
            color: #64748b;
            font-size: 12px;
            font-weight: 600;
            padding: 4px 9px;
            border: 1px solid #cbd5e1;
            border-radius: 7px;
            background: #f8fafc;
        }

        QLabel#workflowProgressStep[state="pending"] {
            color: #64748b;
            border-color: #cbd5e1;
            background: #f8fafc;
        }

        QLabel#workflowProgressStep[state="active"] {
            color: #1e3a8a;
            border-color: #60a5fa;
            background: #eff6ff;
        }

        QLabel#workflowProgressStep[state="completed"] {
            color: #14532d;
            border-color: #86efac;
            background: #f0fdf4;
        }

        QLabel#workflowProgressStep[state="failed"] {
            color: #7f1d1d;
            border-color: #fca5a5;
            background: #fef2f2;
        }

        QLabel#workflowProgressStep[state="blocked"],
        QLabel#workflowProgressStep[state="uncertain"] {
            color: #713f12;
            border-color: #fbbf24;
            background: #fffbeb;
        }

        QLabel#workflowProgressStep[state="ready"] {
            color: #581c87;
            border-color: #c084fc;
            background: #faf5ff;
        }

        QLabel#workflowProgressStep[state="submitted"] {
            color: #3730a3;
            border-color: #a5b4fc;
            background: #eef2ff;
        }

        QLabel#workflowProgressArrow {
            color: #94a3b8;
            font-size: 14px;
            font-weight: 700;
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
    if (rejectIfManifestImportPending()) {
        return;
    }

    if (m_agentController->isRunning()) {
        m_messageList->addErrorMessage(QString::fromUtf8("\xE5\xBD\x93\xE5\x89\x8D\x20\x41\x67\x65\x6E\x74\x20\xE8\xAF\xB7\xE6\xB1\x82\xE4\xBB\x8D\xE5\x9C\xA8\xE6\x89\xA7\xE8\xA1\x8C\xEF\xBC\x8C\xE8\xAF\xB7\xE7\xAD\x89\xE5\xBE\x85\xE5\xAE\x8C\xE6\x88\x90\xE5\x90\x8E\xE5\x86\x8D\xE6\x93\x8D\xE4\xBD\x9C\xE3\x80\x82"));
        return;
    }

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
    m_manifestImportReady = false;
    m_manifestBlockerMessage.clear();
    m_currentRepairActionId.clear();
    m_currentRepairAction = {};
    clearCurrentNextAction();
    m_gateStatusBar->clear();
    m_repairHistoryBar->clear();
    m_messageList->setRepairMessage(QString());
    m_statusBar->reset();
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
    m_currentBackendTaskDir = taskDir;
    m_currentHostTaskDir = hostTaskDirectoryForBackendTask(taskDir);
    m_statusBar->setTaskId(AgentChatIntent::taskIdFromDirectory(taskDir));
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
    if (rejectIfManifestImportPending()) {
        return;
    }

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

void FoamAgentChatPanel::handleWorkflowActionsReceived(const QStringList &actions)
{
    m_actionBar->setGenerateCaseEnabled(actions.contains(QStringLiteral("generate")));
    m_actionBar->setValidateCaseEnabled(actions.contains(QStringLiteral("validate")));
    m_actionBar->setRunCaseEnabled(actions.contains(QStringLiteral("run")));
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
    if (rejectIfManifestImportPending()) {
        return;
    }

    if (m_currentBackendTaskDir.isEmpty()) {
        m_messageList->addErrorMessage(QString::fromUtf8("\xE5\xBD\x93\xE5\x89\x8D\xE6\xB2\xA1\xE6\x9C\x89\xE5\xB7\xB2\xE8\xA7\x84\xE5\x88\x92\xE4\xBB\xBB\xE5\x8A\xA1\xEF\xBC\x8C\xE8\xAF\xB7\xE5\x85\x88\xE5\x8F\x91\xE9\x80\x81\xE9\x9C\x80\xE6\xB1\x82\xE8\xBF\x9B\xE8\xA1\x8C\xE8\xA7\x84\xE5\x88\x92\xE3\x80\x82"));
        return;
    }

    m_messageList->addToolMessage(QString::fromUtf8("\xE6\xAD\xA3\xE5\x9C\xA8\xE7\x94\x9F\xE6\x88\x90\xE7\xAE\x97\xE4\xBE\x8B\xE6\x96\x87\xE4\xBB\xB6\x2E\x2E\x2E"));
    m_currentRepairActionId.clear();
    m_currentRepairAction = {};
    m_messageList->setRepairMessage(QString());
    m_agentController->generateCaseForTask(m_currentBackendTaskDir, repairAction);
}

void FoamAgentChatPanel::handleValidateCaseRequested()
{
    if (rejectIfManifestImportPending()) {
        return;
    }

    if (m_currentBackendTaskDir.isEmpty()) {
        m_messageList->addErrorMessage(QString::fromUtf8("\xE5\xBD\x93\xE5\x89\x8D\xE6\xB2\xA1\xE6\x9C\x89\xE5\xB7\xB2\xE7\x94\x9F\xE6\x88\x90\xE4\xBB\xBB\xE5\x8A\xA1\xEF\xBC\x8C\xE8\xAF\xB7\xE5\x85\x88\xE7\x94\x9F\xE6\x88\x90\xE7\xAE\x97\xE4\xBE\x8B\xE3\x80\x82"));
        return;
    }

    m_messageList->addToolMessage(QString::fromUtf8("\xE6\xAD\xA3\xE5\x9C\xA8\xE6\xA0\xA1\xE9\xAA\x8C\xE7\xAE\x97\xE4\xBE\x8B\x2E\x2E\x2E"));
    m_agentController->validateCaseForTask(m_currentBackendTaskDir);
}

void FoamAgentChatPanel::handleRunCaseRequested()
{
    if (rejectIfManifestImportPending()) {
        return;
    }

    if (m_currentBackendTaskDir.isEmpty()) {
        m_messageList->addErrorMessage(QString::fromUtf8("\xE5\xBD\x93\xE5\x89\x8D\xE6\xB2\xA1\xE6\x9C\x89\xE5\xB7\xB2\xE6\xA0\xA1\xE9\xAA\x8C\xE4\xBB\xBB\xE5\x8A\xA1\xEF\xBC\x8C\xE8\xAF\xB7\xE5\x85\x88\xE6\xA0\xA1\xE9\xAA\x8C\xE7\xAE\x97\xE4\xBE\x8B\xE3\x80\x82"));
        return;
    }

    m_messageList->addToolMessage(QString::fromUtf8("\xE6\xAD\xA3\xE5\x9C\xA8\xE8\xBF\x90\xE8\xA1\x8C\x20\x4F\x70\x65\x6E\x46\x4F\x41\x4D\x2E\x2E\x2E"));
    m_agentController->runCaseForTask(m_currentBackendTaskDir);
}

void FoamAgentChatPanel::handleManifestReadinessReceived(const QString &manifestPath,
                                                         bool importReady,
                                                         const QString &blockerMessage)
{
    m_latestManifestPath = QDir::toNativeSeparators(manifestPath.trimmed());
    m_manifestImportReady = importReady && !m_latestManifestPath.isEmpty();
    m_manifestBlockerMessage = blockerMessage.trimmed();

    m_statusBar->setManifestReady(m_manifestImportReady);
    m_actionBar->setImportToAppFlowEnabled(m_manifestImportReady);
    if (!m_latestManifestPath.isEmpty()) {
        m_messageList->addToolMessage(QString::fromUtf8("\xE5\xB7\xB2\xE6\x94\xB6\xE5\x88\xB0\x20\x41\x67\x65\x6E\x74\x20\xE6\xB8\x85\xE5\x8D\x95\xEF\xBC\x9A\x25\x31").arg(m_latestManifestPath));
    }
}

void FoamAgentChatPanel::handleImportToAppFlowRequested()
{
    if (!m_pendingManifestImportPath.isEmpty()) {
        m_messageList->addErrorMessage(QString::fromUtf8("\x41\x50\x50\x46\x6C\x6F\x77\x20\xE5\xAF\xBC\xE5\x85\xA5\xE4\xBB\x8D\xE5\x9C\xA8\xE8\xBF\x9B\xE8\xA1\x8C\xEF\xBC\x8C\xE8\xAF\xB7\xE7\xAD\x89\xE5\xBE\x85\xE6\x9C\x80\xE7\xBB\x88\xE7\xBB\x93\xE6\x9E\x9C\xE3\x80\x82"));
        return;
    }

    if (!m_manifestImportReady || m_latestManifestPath.isEmpty()) {
        const QString message = m_manifestBlockerMessage.isEmpty()
            ? QString::fromUtf8("\xE5\xBD\x93\xE5\x89\x8D\x20\x41\x67\x65\x6E\x74\x20\xE6\xB8\x85\xE5\x8D\x95\xE5\xB0\x9A\xE6\x9C\xAA\xE5\xB0\xB1\xE7\xBB\xAA\xEF\xBC\x8C\xE6\x97\xA0\xE6\xB3\x95\xE5\xAF\xBC\xE5\x85\xA5\xE3\x80\x82")
            : m_manifestBlockerMessage;
        m_messageList->addErrorMessage(message);
        return;
    }

    if (!QFileInfo(m_latestManifestPath).isFile()) {
        m_statusBar->setImportSubmissionFailed();
        m_messageList->addErrorMessage(QString::fromUtf8("\x41\x67\x65\x6E\x74\x20\xE6\xB8\x85\xE5\x8D\x95\xE5\xAF\xBC\xE5\x85\xA5\xE5\xA4\xB1\xE8\xB4\xA5\xEF\xBC\x9A\x25\x31").arg(m_latestManifestPath));
        return;
    }

    Core::FITKActionOperator* oper = FITKOPERREPO->getOperatorT<Core::FITKActionOperator>(QStringLiteral("actionLoadAgentManifest"));
    if (oper == nullptr) {
        m_statusBar->setImportSubmissionFailed();
        m_messageList->addErrorMessage(QString::fromUtf8("\x41\x67\x65\x6E\x74\x20\xE6\xB8\x85\xE5\x8D\x95\xE5\xAF\xBC\xE5\x85\xA5\xE6\x93\x8D\xE4\xBD\x9C\xE6\x9C\xAA\xE6\x89\xBE\xE5\x88\xB0\xE3\x80\x82"));
        return;
    }

    Core::FITKActionOperator* resultSource = FITKOPERREPO->getOperatorT<Core::FITKActionOperator>(
        QStringLiteral("actionImportOpenFoamMesh"));
    if (resultSource == nullptr) {
        m_statusBar->setImportSubmissionFailed();
        m_messageList->addErrorMessage(QString::fromUtf8("\x41\x50\x50\x46\x6C\x6F\x77\x20\xE5\xAF\xBC\xE5\x85\xA5\xE7\xBB\x93\xE6\x9E\x9C\xE4\xBF\xA1\xE5\x8F\xB7\xE6\xBA\x90\xE4\xB8\x8D\xE5\x8F\xAF\xE7\x94\xA8\xEF\xBC\x8C\xE6\x9C\xAA\xE5\x90\xAF\xE5\x8A\xA8\xE5\xAF\xBC\xE5\x85\xA5\xE3\x80\x82"));
        return;
    }

    if (m_manifestImportResultSource.data() != resultSource) {
        if (!m_manifestImportResultSource.isNull()) {
            QObject::disconnect(m_manifestImportResultSource.data(), nullptr, this, nullptr);
            m_manifestImportResultSource.clear();
        }
        const QMetaObject::Connection connection = QObject::connect(
            resultSource,
            SIGNAL(agentManifestImportFinished(QString,bool,QString)),
            this,
            SLOT(handleAgentManifestImportFinished(QString,bool,QString)),
            Qt::UniqueConnection);
        if (!connection) {
            m_statusBar->setImportSubmissionFailed();
            m_messageList->addErrorMessage(QString::fromUtf8("\x41\x50\x50\x46\x6C\x6F\x77\x20\xE5\xAF\xBC\xE5\x85\xA5\xE7\xBB\x93\xE6\x9E\x9C\xE4\xBF\xA1\xE5\x8F\xB7\xE8\xBF\x9E\xE6\x8E\xA5\xE5\xA4\xB1\xE8\xB4\xA5\xEF\xBC\x8C\xE6\x9C\xAA\xE5\x90\xAF\xE5\x8A\xA8\xE5\xAF\xBC\xE5\x85\xA5\xE3\x80\x82"));
            return;
        }
        m_manifestImportResultSource = resultSource;
    }

    const QString manifestPathSnapshot = m_latestManifestPath;
    m_messageList->addToolMessage(QString::fromUtf8("\xE6\xAD\xA3\xE5\x9C\xA8\xE5\xAF\xBC\xE5\x85\xA5\x20\x41\x67\x65\x6E\x74\x20\xE6\xB8\x85\xE5\x8D\x95\xE5\x88\xB0\x20\x41\x50\x50\x46\x6C\x6F\x77\xEF\xBC\x9A\x25\x31").arg(m_latestManifestPath));
    oper->setArgs("FileName", m_latestManifestPath);
    if (oper->execProfession()) {
        m_pendingManifestImportPath = manifestPathSnapshot;
        m_statusBar->setImportSubmitted();
        m_actionBar->setManifestImportRunning(true);
        refreshInteractionAvailability();
        clearCurrentNextAction();
        m_messageList->addToolMessage(QString::fromUtf8("\x41\x50\x50\x46\x6C\x6F\x77\x20\xE5\xA4\x84\xE7\x90\x86\xE8\xAF\xB7\xE6\xB1\x82\xE5\xB7\xB2\xE6\x8F\x90\xE4\xBA\xA4\xE3\x80\x82"));
    }
    else {
        m_statusBar->setImportSubmissionFailed();
        m_messageList->addErrorMessage(QString::fromUtf8("\x41\x67\x65\x6E\x74\x20\xE6\xB8\x85\xE5\x8D\x95\xE5\xAF\xBC\xE5\x85\xA5\xE5\xA4\xB1\xE8\xB4\xA5\xEF\xBC\x9A\x25\x31").arg(m_latestManifestPath));
    }
}

void FoamAgentChatPanel::handleAgentManifestImportFinished(
    const QString &manifestPath,
    bool success,
    const QString &message)
{
    if (m_pendingManifestImportPath.isEmpty()
        || !manifestPathsMatch(manifestPath, m_pendingManifestImportPath)) {
        return;
    }

    m_pendingManifestImportPath.clear();
    m_actionBar->setManifestImportRunning(false);
    refreshInteractionAvailability();
    if (success) {
        m_statusBar->setImportCompleted();
        m_actionBar->setImportToAppFlowEnabled(false);
        m_messageList->addToolMessage(QString::fromUtf8("\x41\x50\x50\x46\x6C\x6F\x77\x20\xE5\xAF\xBC\xE5\x85\xA5\xE5\xB7\xB2\xE5\xAE\x8C\xE6\x88\x90\xE3\x80\x82"));
        return;
    }

    m_statusBar->setImportFailed();
    m_actionBar->setImportToAppFlowEnabled(m_manifestImportReady);
    QString detail = safeImportResultMessage(message);
    if (detail.isEmpty()) {
        detail = QString::fromUtf8("\xE6\x9C\xAA\xE6\x94\xB6\xE5\x88\xB0\xE5\x8F\xAF\xE7\x94\xA8\xE7\x9A\x84\xE5\xA4\xB1\xE8\xB4\xA5\xE8\xAF\xA6\xE6\x83\x85\xE3\x80\x82");
    }
    m_messageList->addErrorMessage(QString::fromUtf8("\x41\x50\x50\x46\x6C\x6F\x77\x20\xE5\xAF\xBC\xE5\x85\xA5\xE5\xA4\xB1\xE8\xB4\xA5\xEF\xBC\x9A\x25\x31").arg(detail));
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
    m_messageList->addToolMessage(text);
}

void FoamAgentChatPanel::requestParameterReplan(const QString &message)
{
    if (rejectIfManifestImportPending()) {
        return;
    }

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
    if (rejectIfManifestImportPending()) {
        return;
    }

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

bool FoamAgentChatPanel::rejectIfManifestImportPending()
{
    if (m_pendingManifestImportPath.isEmpty()) {
        return false;
    }

    m_messageList->addErrorMessage(QStringLiteral(
        "APPFlow \u5bfc\u5165\u4ecd\u5728\u8fdb\u884c\uff0c\u8bf7\u7b49\u5f85\u6700\u7ec8\u5b8c\u6210\u540e\u518d\u542f\u52a8\u65b0\u7684\u5de5\u4f5c\u6d41\u64cd\u4f5c\u3002"));
    return true;
}

void FoamAgentChatPanel::refreshInteractionAvailability()
{
    m_composer->setEnabled(
        !m_agentRequestRunning && m_pendingManifestImportPath.isEmpty());
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

