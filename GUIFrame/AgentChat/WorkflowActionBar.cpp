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
    m_generateButton->setEnabled(false);

    m_validateButton = new QPushButton(QString::fromUtf8("\xE6\xA0\xA1\xE9\xAA\x8C\xE7\xAE\x97\xE4\xBE\x8B"), this);
    m_validateButton->setObjectName("workflowActionValidateButton");
    m_validateButton->setEnabled(false);

    m_runButton = new QPushButton(QString::fromUtf8("\xE8\xBF\x90\xE8\xA1\x8C\xE7\xAE\x97\xE4\xBE\x8B"), this);
    m_runButton->setObjectName("workflowActionRunButton");
    m_runButton->setEnabled(false);

    m_importToAppFlowButton = new QPushButton(QString::fromUtf8("\xE5\xAF\xBC\xE5\x85\xA5\xE5\x88\xB0\x20\x41\x50\x50\x46\x6C\x6F\x77"), this);
    m_importToAppFlowButton->setObjectName("workflowActionImportToAppFlowButton");
    m_importToAppFlowButton->setEnabled(false);

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
}

void WorkflowActionBar::setTaskRunning(bool running)
{
    m_openFolderButton->setEnabled(!running);
    m_generateButton->setEnabled(!running && m_canGenerateCase);
    m_validateButton->setEnabled(!running && m_canValidateCase);
    m_runButton->setEnabled(!running && m_canRunCase);
    m_importToAppFlowButton->setEnabled(!running && m_canImportToAppFlow);
}

void WorkflowActionBar::setGenerateCaseEnabled(bool enabled)
{
    m_canGenerateCase = enabled;
    m_generateButton->setEnabled(enabled);
}

void WorkflowActionBar::setValidateCaseEnabled(bool enabled)
{
    m_canValidateCase = enabled;
    m_validateButton->setEnabled(enabled);
}

void WorkflowActionBar::setRunCaseEnabled(bool enabled)
{
    m_canRunCase = enabled;
    m_runButton->setEnabled(enabled);
}

void WorkflowActionBar::setImportToAppFlowEnabled(bool enabled)
{
    m_canImportToAppFlow = enabled;
    m_importToAppFlowButton->setEnabled(enabled);
}
