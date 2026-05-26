#include "WorkflowStatusBar.h"

#include <QHBoxLayout>
#include <QLabel>

namespace {

QString displayText(const QString &prefix, const QString &value, const QString &emptyText)
{
    const QString cleanValue = value.trimmed();
    return prefix + (cleanValue.isEmpty() ? emptyText : cleanValue);
}

}

WorkflowStatusBar::WorkflowStatusBar(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("workflowStatusBar");

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 8, 20, 8);
    layout->setSpacing(10);

    m_taskLabel = new QLabel(this);
    m_taskLabel->setObjectName("workflowStatusItem");

    m_stageLabel = new QLabel(this);
    m_stageLabel->setObjectName("workflowStatusItem");

    m_nextActionLabel = new QLabel(this);
    m_nextActionLabel->setObjectName("workflowStatusItem");

    m_manifestLabel = new QLabel(this);
    m_manifestLabel->setObjectName("workflowStatusItem");

    layout->addWidget(m_taskLabel);
    layout->addWidget(m_stageLabel);
    layout->addWidget(m_nextActionLabel);
    layout->addWidget(m_manifestLabel);
    layout->addStretch(1);

    reset();
}

void WorkflowStatusBar::reset()
{
    m_taskId.clear();
    m_stage.clear();
    m_nextAction.clear();
    m_manifestReady = false;
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
    refresh();
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
