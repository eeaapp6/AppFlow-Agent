#include "WorkflowActionBar.h"

#include <QHBoxLayout>
#include <QPushButton>

WorkflowActionBar::WorkflowActionBar(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("workflowActionBar");

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 10, 20, 10);
    layout->setSpacing(10);

    m_openFolderButton = new QPushButton(QString::fromUtf8("\xE6\x89\x93\xE5\xBC\x80\xE7\x9B\xAE\xE5\xBD\x95"), this);
    m_openFolderButton->setObjectName("workflowActionOpenFolderButton");

    m_generateButton = new QPushButton(QString::fromUtf8("\xE7\x94\x9F\xE6\x88\x90\xE7\xAE\x97\xE4\xBE\x8B"), this);
    m_generateButton->setObjectName("workflowActionGenerateButton");

    m_validateButton = new QPushButton(QString::fromUtf8("\xE6\xA0\xA1\xE9\xAA\x8C\xE7\xAE\x97\xE4\xBE\x8B"), this);
    m_validateButton->setObjectName("workflowActionValidateButton");

    m_runButton = new QPushButton(QString::fromUtf8("\xE8\xBF\x90\xE8\xA1\x8C\xE7\xAE\x97\xE4\xBE\x8B"), this);
    m_runButton->setObjectName("workflowActionRunButton");

    m_importToAppFlowButton = new QPushButton(QString::fromUtf8("\xE5\xAF\xBC\xE5\x85\xA5\xE5\x88\xB0\x20\x41\x50\x50\x46\x6C\x6F\x77"), this);
    m_importToAppFlowButton->setObjectName("workflowActionImportToAppFlowButton");

    layout->addWidget(m_openFolderButton);
    layout->addSpacing(6);
    layout->addWidget(m_generateButton);
    layout->addWidget(m_validateButton);
    layout->addWidget(m_runButton);
    layout->addWidget(m_importToAppFlowButton);
    layout->addStretch(1);

    connect(m_openFolderButton, &QPushButton::clicked,
            this, &WorkflowActionBar::openFolderRequested);
    connect(m_generateButton, &QPushButton::clicked,
            this, &WorkflowActionBar::generateCaseRequested);
    connect(m_validateButton, &QPushButton::clicked,
            this, &WorkflowActionBar::validateCaseRequested);
    connect(m_runButton, &QPushButton::clicked,
            this, &WorkflowActionBar::runCaseRequested);
    connect(m_importToAppFlowButton, &QPushButton::clicked,
            this, &WorkflowActionBar::importToAppFlowRequested);

    refreshButtonStates();
}

void WorkflowActionBar::setTaskRunning(bool running)
{
    m_taskRunning = running;
    refreshButtonStates();
}

void WorkflowActionBar::setManifestImportRunning(bool running)
{
    m_manifestImportRunning = running;
    refreshButtonStates();
}

void WorkflowActionBar::setGenerateCaseEnabled(bool enabled)
{
    m_canGenerateCase = enabled;
    refreshButtonStates();
}

void WorkflowActionBar::setValidateCaseEnabled(bool enabled)
{
    m_canValidateCase = enabled;
    refreshButtonStates();
}

void WorkflowActionBar::setRunCaseEnabled(bool enabled)
{
    m_canRunCase = enabled;
    refreshButtonStates();
}

void WorkflowActionBar::setImportToAppFlowEnabled(bool enabled)
{
    m_canImportToAppFlow = enabled;
    refreshButtonStates();
}

void WorkflowActionBar::refreshButtonStates()
{
    const bool workflowWriteEnabled = !m_taskRunning && !m_manifestImportRunning;
    m_openFolderButton->setEnabled(!m_taskRunning);
    m_generateButton->setEnabled(workflowWriteEnabled && m_canGenerateCase);
    m_validateButton->setEnabled(workflowWriteEnabled && m_canValidateCase);
    m_runButton->setEnabled(workflowWriteEnabled && m_canRunCase);
    m_importToAppFlowButton->setEnabled(workflowWriteEnabled && m_canImportToAppFlow);
}
