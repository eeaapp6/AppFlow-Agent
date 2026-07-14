#include "WorkflowStatusBar.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStyle>
#include <QTimer>
#include <QVariant>
#include <QVBoxLayout>

namespace {

QString displayText(const QString &prefix, const QString &value, const QString &emptyText)
{
    const QString cleanValue = value.trimmed();
    return prefix + (cleanValue.isEmpty() ? emptyText : cleanValue);
}

QString progressStepName(int step)
{
    static const QString names[] = {
        QStringLiteral("\u89c4\u5212"),
        QStringLiteral("\u751f\u6210"),
        QStringLiteral("\u6821\u9a8c"),
        QStringLiteral("\u8fd0\u884c"),
        QStringLiteral("\u5bfc\u5165")
    };
    return step >= 0 && step < 5 ? names[step] : QString();
}

QString progressStateText(const QString &state)
{
    if (state == QStringLiteral("active")) return QStringLiteral("\u8fdb\u884c\u4e2d");
    if (state == QStringLiteral("completed")) return QStringLiteral("\u5df2\u5b8c\u6210");
    if (state == QStringLiteral("failed")) return QStringLiteral("\u5931\u8d25");
    if (state == QStringLiteral("blocked")) return QStringLiteral("\u963b\u585e");
    if (state == QStringLiteral("uncertain")) return QStringLiteral("\u72b6\u6001\u5f85\u786e\u8ba4");
    if (state == QStringLiteral("ready")) return QStringLiteral("\u53ef\u5bfc\u5165");
    if (state == QStringLiteral("submitted")) return QStringLiteral("\u5df2\u63d0\u4ea4");
    return QStringLiteral("\u672a\u5f00\u59cb");
}

}

WorkflowStatusBar::WorkflowStatusBar(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("workflowStatusBar");

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 8, 20, 8);
    layout->setSpacing(7);

    auto *summaryLayout = new QHBoxLayout;
    summaryLayout->setContentsMargins(0, 0, 0, 0);
    summaryLayout->setSpacing(10);

    m_taskLabel = new QLabel(this);
    m_taskLabel->setObjectName("workflowStatusItem");

    m_stageLabel = new QLabel(this);
    m_stageLabel->setObjectName("workflowStatusItem");

    m_nextActionLabel = new QLabel(this);
    m_nextActionLabel->setObjectName("workflowStatusItem");

    m_manifestLabel = new QLabel(this);
    m_manifestLabel->setObjectName("workflowStatusItem");

    summaryLayout->addWidget(m_taskLabel);
    summaryLayout->addWidget(m_stageLabel);
    summaryLayout->addWidget(m_nextActionLabel);
    summaryLayout->addWidget(m_manifestLabel);
    summaryLayout->addStretch(1);
    layout->addLayout(summaryLayout);

    auto *progressLayout = new QHBoxLayout;
    progressLayout->setContentsMargins(0, 0, 0, 0);
    progressLayout->setSpacing(7);
    for (int i = 0; i < ProgressStepCount; ++i) {
        auto *stepLabel = new QLabel(this);
        stepLabel->setObjectName(QStringLiteral("workflowProgressStep"));
        m_progressLabels[i] = stepLabel;
        progressLayout->addWidget(stepLabel);
        if (i + 1 < ProgressStepCount) {
            auto *arrow = new QLabel(QStringLiteral("\u2192"), this);
            arrow->setObjectName(QStringLiteral("workflowProgressArrow"));
            progressLayout->addWidget(arrow);
        }
    }
    progressLayout->addStretch(1);
    layout->addLayout(progressLayout);

    m_refreshTimer = new QTimer(this);
    m_refreshTimer->setInterval(1000);
    connect(m_refreshTimer, &QTimer::timeout, this, [this]() {
        if (m_timingActive) refreshProgressStep(m_timedStep);
    });

    reset();
}

void WorkflowStatusBar::reset()
{
    m_taskId.clear();
    m_stage.clear();
    m_nextAction.clear();
    m_manifestReady = false;
    resetProgress();
    refresh();
}

void WorkflowStatusBar::setTaskId(const QString &taskId)
{
    m_taskId = taskId.trimmed();
    refresh();
}

void WorkflowStatusBar::setStageText(const QString &stage)
{
    m_stage = stage.trimmed();
    refresh();
}

void WorkflowStatusBar::setNextActionText(const QString &nextAction)
{
    m_nextAction = nextAction.trimmed();
    refresh();
}

void WorkflowStatusBar::setManifestReady(bool ready)
{
    m_manifestReady = ready;
    setImportReady(ready);
    refresh();
}

void WorkflowStatusBar::resetProgress()
{
    clearAllTimings();
    for (int i = 0; i < ProgressStepCount; ++i)
        setProgressState(static_cast<ProgressStep>(i), QStringLiteral("pending"));
}

void WorkflowStatusBar::beginWorkflowAction(const QString &action)
{
    const QString cleanAction = action.trimmed();
    if (cleanAction == QStringLiteral("plan")) {
        resetProgress();
        setProgressState(PlanStep, QStringLiteral("active"));
        startTiming(PlanStep);
        setStageText(QStringLiteral("\u89c4\u5212\u4e2d"));
    } else if (cleanAction == QStringLiteral("replan")) {
        setProgressState(PlanStep, QStringLiteral("active"));
        startTiming(PlanStep);
        setStageText(QStringLiteral("\u91cd\u65b0\u89c4\u5212\u4e2d"));
    } else if (cleanAction == QStringLiteral("generate")) {
        clearTiming(ValidateStep);
        clearTiming(RunStep);
        clearTiming(ImportStep);
        setProgressStates(QStringLiteral("completed"), QStringLiteral("active"),
                          QStringLiteral("pending"), QStringLiteral("pending"),
                          QStringLiteral("pending"));
        startTiming(GenerateStep);
        setStageText(QStringLiteral("\u751f\u6210\u4e2d"));
    } else if (cleanAction == QStringLiteral("validate")) {
        clearTiming(RunStep);
        clearTiming(ImportStep);
        setProgressStates(QStringLiteral("completed"), QStringLiteral("completed"),
                          QStringLiteral("active"), QStringLiteral("pending"),
                          QStringLiteral("pending"));
        startTiming(ValidateStep);
        setStageText(QStringLiteral("\u6821\u9a8c\u4e2d"));
    } else if (cleanAction == QStringLiteral("run")) {
        clearTiming(ImportStep);
        setProgressStates(QStringLiteral("completed"), QStringLiteral("completed"),
                          QStringLiteral("completed"), QStringLiteral("active"),
                          QStringLiteral("pending"));
        startTiming(RunStep);
        setStageText(QStringLiteral("\u8fd0\u884c\u4e2d"));
    }
}

void WorkflowStatusBar::applyBackendStatus(const QString &status)
{
    const QString cleanStatus = status.trimmed();
    if (cleanStatus == QStringLiteral("planning") || cleanStatus == QStringLiteral("loading_reference")) {
        setProgressStates(QStringLiteral("active"), QStringLiteral("pending"), QStringLiteral("pending"), QStringLiteral("pending"), QStringLiteral("pending"));
        setStageText(QStringLiteral("\u89c4\u5212\u4e2d"));
    } else if (cleanStatus == QStringLiteral("planned")) {
        finishTiming(PlanStep);
        clearTiming(GenerateStep);
        clearTiming(ValidateStep);
        clearTiming(RunStep);
        clearTiming(ImportStep);
        setProgressStates(QStringLiteral("completed"), QStringLiteral("pending"), QStringLiteral("pending"), QStringLiteral("pending"), QStringLiteral("pending"));
        setStageText(QStringLiteral("\u5df2\u89c4\u5212"));
    } else if (cleanStatus == QStringLiteral("generating_files")) {
        setProgressStates(QStringLiteral("completed"), QStringLiteral("active"), QStringLiteral("pending"), QStringLiteral("pending"), QStringLiteral("pending"));
        setStageText(QStringLiteral("\u751f\u6210\u4e2d"));
    } else if (cleanStatus == QStringLiteral("generated")) {
        finishTiming(GenerateStep);
        setProgressStates(QStringLiteral("completed"), QStringLiteral("completed"), QStringLiteral("pending"), QStringLiteral("pending"), QStringLiteral("pending"));
        setStageText(QStringLiteral("\u5df2\u751f\u6210"));
    } else if (cleanStatus == QStringLiteral("validating")) {
        setProgressStates(QStringLiteral("completed"), QStringLiteral("completed"), QStringLiteral("active"), QStringLiteral("pending"), QStringLiteral("pending"));
        setStageText(QStringLiteral("\u6821\u9a8c\u4e2d"));
    } else if (cleanStatus == QStringLiteral("validation_failed")) {
        finishTiming(ValidateStep);
        setProgressStates(QStringLiteral("completed"), QStringLiteral("completed"), QStringLiteral("failed"), QStringLiteral("pending"), QStringLiteral("pending"));
        setStageText(QStringLiteral("\u6821\u9a8c\u5931\u8d25"));
    } else if (cleanStatus == QStringLiteral("validated")) {
        finishTiming(ValidateStep);
        setProgressStates(QStringLiteral("completed"), QStringLiteral("completed"), QStringLiteral("completed"), QStringLiteral("pending"), QStringLiteral("pending"));
        setStageText(QStringLiteral("\u5df2\u6821\u9a8c"));
    } else if (cleanStatus == QStringLiteral("running")) {
        setProgressStates(QStringLiteral("completed"), QStringLiteral("completed"), QStringLiteral("completed"), QStringLiteral("active"), QStringLiteral("pending"));
        setStageText(QStringLiteral("\u8fd0\u884c\u4e2d"));
    } else if (cleanStatus == QStringLiteral("run_blocked")) {
        finishTiming(RunStep);
        setProgressStates(QStringLiteral("completed"), QStringLiteral("completed"), QStringLiteral("completed"), QStringLiteral("blocked"), QStringLiteral("pending"));
        setStageText(QStringLiteral("\u8fd0\u884c\u963b\u585e"));
    } else if (cleanStatus == QStringLiteral("run_failed")) {
        finishTiming(RunStep);
        setProgressStates(QStringLiteral("completed"), QStringLiteral("completed"), QStringLiteral("completed"), QStringLiteral("failed"), QStringLiteral("pending"));
        setStageText(QStringLiteral("\u8fd0\u884c\u5931\u8d25"));
    } else if (cleanStatus == QStringLiteral("run_completed")) {
        finishTiming(RunStep);
        setProgressState(PlanStep, QStringLiteral("completed"));
        setProgressState(GenerateStep, QStringLiteral("completed"));
        setProgressState(ValidateStep, QStringLiteral("completed"));
        setProgressState(RunStep, QStringLiteral("completed"));
        setStageText(QStringLiteral("\u5df2\u8fd0\u884c"));
    } else if (cleanStatus == QStringLiteral("failed")) {
        if (m_timingActive && m_timedStep < ImportStep)
            finishTiming(m_timedStep);
        for (int i = 0; i < ImportStep; ++i) {
            if (m_progressStates[i] == QStringLiteral("active")) {
                setProgressState(static_cast<ProgressStep>(i), QStringLiteral("failed"));
                break;
            }
        }
        setStageText(QStringLiteral("\u5931\u8d25"));
    } else if (cleanStatus != QStringLiteral("chat_completed")) {
        if (m_timingActive) finishTiming(m_timedStep);
        setStageText(QStringLiteral("\u672a\u77e5\u72b6\u6001"));
    }
}

void WorkflowStatusBar::markWorkflowActionUncertain(const QString &action)
{
    const QString cleanAction = action.trimmed();
    ProgressStep step = PlanStep;
    if (cleanAction == QStringLiteral("plan") || cleanAction == QStringLiteral("replan"))
        step = PlanStep;
    else if (cleanAction == QStringLiteral("generate"))
        step = GenerateStep;
    else if (cleanAction == QStringLiteral("validate"))
        step = ValidateStep;
    else if (cleanAction == QStringLiteral("run"))
        step = RunStep;
    else
        return;

    finishTiming(step);
    if (step == PlanStep)
        setProgressState(PlanStep, QStringLiteral("uncertain"));
    else if (step == GenerateStep)
        setProgressState(GenerateStep, QStringLiteral("uncertain"));
    else if (step == ValidateStep)
        setProgressState(ValidateStep, QStringLiteral("uncertain"));
    else
        setProgressState(RunStep, QStringLiteral("uncertain"));
    setStageText(QStringLiteral("\u72b6\u6001\u5f85\u786e\u8ba4"));
}

void WorkflowStatusBar::setImportReady(bool ready)
{
    clearTiming(ImportStep);
    setProgressState(ImportStep, ready ? QStringLiteral("ready") : QStringLiteral("pending"));
}

void WorkflowStatusBar::setImportSubmitted()
{
    setProgressState(ImportStep, QStringLiteral("submitted"));
    startTiming(ImportStep);
    setStageText(QStringLiteral("\u5df2\u63d0\u4ea4"));
}

void WorkflowStatusBar::setImportCompleted()
{
    finishTiming(ImportStep);
    setProgressState(ImportStep, QStringLiteral("completed"));
    setStageText(QStringLiteral("\u5bfc\u5165\u5b8c\u6210"));
}

void WorkflowStatusBar::setImportFailed()
{
    finishTiming(ImportStep);
    setProgressState(ImportStep, QStringLiteral("failed"));
    setStageText(QStringLiteral("\u5bfc\u5165\u5931\u8d25"));
}

void WorkflowStatusBar::setImportSubmissionFailed()
{
    clearTiming(ImportStep);
    setProgressState(ImportStep, QStringLiteral("failed"));
    setStageText(QStringLiteral("\u5bfc\u5165\u63d0\u4ea4\u5931\u8d25"));
}

void WorkflowStatusBar::setProgressState(ProgressStep step, const QString &state)
{
    m_progressStates[step] = state;
    refreshProgressStep(step);
}

void WorkflowStatusBar::setProgressStates(const QString &plan, const QString &generate,
                                          const QString &validate, const QString &run,
                                          const QString &importState)
{
    const QString states[] = { plan, generate, validate, run, importState };
    for (int i = 0; i < ProgressStepCount; ++i)
        setProgressState(static_cast<ProgressStep>(i), states[i]);
}

void WorkflowStatusBar::refreshProgressStep(ProgressStep step)
{
    QLabel *label = m_progressLabels[step];
    if (label == nullptr) return;
    const QString state = m_progressStates[step];
    label->setProperty("state", state);
    QString text = QStringLiteral("%1\uff1a%2").arg(
        progressStepName(step), progressStateText(state));
    const qint64 elapsed = elapsedForStep(step);
    if (elapsed >= 0)
        text += QStringLiteral(" \u00b7 %1").arg(formatDuration(elapsed));
    label->setText(text);
    label->style()->unpolish(label);
    label->style()->polish(label);
    label->update();
}

void WorkflowStatusBar::startTiming(ProgressStep step)
{
    if (m_timingActive && m_timedStep != step)
        finishTiming(m_timedStep);

    m_timedStep = step;
    m_progressDurationsMs[step] = 0;
    m_progressDurationValid[step] = true;
    m_elapsedTimer.restart();
    m_timingActive = true;
    if (m_refreshTimer != nullptr && !m_refreshTimer->isActive())
        m_refreshTimer->start();
    refreshProgressStep(step);
}

void WorkflowStatusBar::finishTiming(ProgressStep step)
{
    if (!m_timingActive || m_timedStep != step) return;

    m_progressDurationsMs[step] = m_elapsedTimer.elapsed();
    m_progressDurationValid[step] = true;
    m_timingActive = false;
    if (m_refreshTimer != nullptr) m_refreshTimer->stop();
    refreshProgressStep(step);
}

void WorkflowStatusBar::cancelTiming()
{
    m_timingActive = false;
    if (m_refreshTimer != nullptr) m_refreshTimer->stop();
}

void WorkflowStatusBar::clearTiming(ProgressStep step)
{
    if (m_timingActive && m_timedStep == step) cancelTiming();
    m_progressDurationsMs[step] = 0;
    m_progressDurationValid[step] = false;
    refreshProgressStep(step);
}

void WorkflowStatusBar::clearAllTimings()
{
    cancelTiming();
    for (int i = 0; i < ProgressStepCount; ++i) {
        m_progressDurationsMs[i] = 0;
        m_progressDurationValid[i] = false;
    }
}

qint64 WorkflowStatusBar::elapsedForStep(ProgressStep step) const
{
    if (m_timingActive && m_timedStep == step)
        return m_elapsedTimer.elapsed();
    return m_progressDurationValid[step] ? m_progressDurationsMs[step] : -1;
}

QString WorkflowStatusBar::formatDuration(qint64 milliseconds) const
{
    const qint64 totalSeconds = milliseconds / 1000;
    const qint64 hours = totalSeconds / 3600;
    const qint64 minutes = (totalSeconds % 3600) / 60;
    const qint64 seconds = totalSeconds % 60;
    if (hours > 0) {
        return QStringLiteral("%1:%2:%3")
            .arg(hours)
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0'));
    }
    return QStringLiteral("%1:%2")
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'));
}

void WorkflowStatusBar::refresh()
{
    m_taskLabel->setText(displayText(
        QString::fromUtf8("\xE4\xBB\xBB\xE5\x8A\xA1\xEF\xBC\x9A"),
        m_taskId,
        QString::fromUtf8("\xE6\x97\xA0")));
    m_stageLabel->setText(displayText(
        QString::fromUtf8("\xE9\x98\xB6\xE6\xAE\xB5\xEF\xBC\x9A"),
        m_stage,
        QString::fromUtf8("\xE6\x9C\xAA\xE5\xBC\x80\xE5\xA7\x8B")));
    m_nextActionLabel->setText(displayText(
        QString::fromUtf8("\xE4\xB8\x8B\xE4\xB8\x80\xE6\xAD\xA5\xEF\xBC\x9A"),
        m_nextAction,
        QString::fromUtf8("\xE6\x97\xA0")));
    m_manifestLabel->setText(QString::fromUtf8("\xE6\xB8\x85\xE5\x8D\x95\xEF\xBC\x9A\x25\x31").arg(
        m_manifestReady
            ? QString::fromUtf8("\xE5\xB7\xB2\xE6\x94\xB6\xE5\x88\xB0")
            : QString::fromUtf8("\xE6\x9C\xAA\xE6\x94\xB6\xE5\x88\xB0")));
}
