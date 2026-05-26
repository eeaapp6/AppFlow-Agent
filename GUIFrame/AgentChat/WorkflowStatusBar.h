#pragma once

#include <QString>
#include <QWidget>

class QLabel;

class WorkflowStatusBar : public QWidget
{
public:
    explicit WorkflowStatusBar(QWidget *parent = nullptr);

    void reset();
    void setTaskId(const QString &taskId);
    void setStageText(const QString &stage);
    void setNextActionText(const QString &nextAction);
    void setManifestReady(bool ready);

private:
    void refresh();

    QLabel *m_taskLabel = nullptr;
    QLabel *m_stageLabel = nullptr;
    QLabel *m_nextActionLabel = nullptr;
    QLabel *m_manifestLabel = nullptr;
    QString m_taskId;
    QString m_stage;
    QString m_nextAction;
    bool m_manifestReady = false;
};
