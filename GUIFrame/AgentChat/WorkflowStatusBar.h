#pragma once

#include <QElapsedTimer>
#include <QString>
#include <QWidget>

class QLabel;
class QTimer;

class WorkflowStatusBar : public QWidget
{
public:
    explicit WorkflowStatusBar(QWidget *parent = nullptr);

    void reset();
    void setTaskId(const QString &taskId);
    void setStageText(const QString &stage);
    void setNextActionText(const QString &nextAction);
    void setManifestReady(bool ready);
    void resetProgress();
    void beginWorkflowAction(const QString &action);
    void applyBackendStatus(const QString &status);
    void markWorkflowActionUncertain(const QString &action);
    void setImportReady(bool ready);
    void setImportSubmitted();
    void setImportCompleted();
    void setImportFailed();
    void setImportSubmissionFailed();

private:
    enum ProgressStep {
        PlanStep = 0,
        GenerateStep,
        ValidateStep,
        RunStep,
        ImportStep,
        ProgressStepCount
    };

    void refresh();
    void setProgressState(ProgressStep step, const QString &state);
    void setProgressStates(const QString &plan, const QString &generate,
                           const QString &validate, const QString &run,
                           const QString &importState);
    void refreshProgressStep(ProgressStep step);
    void startTiming(ProgressStep step);
    void finishTiming(ProgressStep step);
    void cancelTiming();
    void clearTiming(ProgressStep step);
    void clearAllTimings();
    qint64 elapsedForStep(ProgressStep step) const;
    QString formatDuration(qint64 milliseconds) const;

    QLabel *m_taskLabel = nullptr;
    QLabel *m_stageLabel = nullptr;
    QLabel *m_nextActionLabel = nullptr;
    QLabel *m_manifestLabel = nullptr;
    QString m_taskId;
    QString m_stage;
    QString m_nextAction;
    bool m_manifestReady = false;
    QLabel *m_progressLabels[ProgressStepCount] = {};
    QString m_progressStates[ProgressStepCount];
    qint64 m_progressDurationsMs[ProgressStepCount] = {};
    bool m_progressDurationValid[ProgressStepCount] = {};
    ProgressStep m_timedStep = PlanStep;
    bool m_timingActive = false;
    QElapsedTimer m_elapsedTimer;
    QTimer *m_refreshTimer = nullptr;
};
