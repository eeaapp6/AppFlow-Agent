#include "WorkflowWorkspaceBar.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

WorkflowWorkspaceBar::WorkflowWorkspaceBar(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("workflowWorkspaceBar");

    auto *layout = new QGridLayout(this);
    layout->setContentsMargins(20, 12, 20, 10);
    layout->setHorizontalSpacing(10);
    layout->setVerticalSpacing(8);

    auto *hostLabel = new QLabel(QString::fromUtf8("\xE6\x9C\xAC\xE6\x9C\xBA\xE8\xBE\x93\xE5\x87\xBA"), this);
    hostLabel->setObjectName("workflowWorkspaceLabel");

    m_outputDirectoryEdit = new QLineEdit(this);
    m_outputDirectoryEdit->setObjectName("workflowWorkspacePath");
    m_outputDirectoryEdit->setPlaceholderText(QString::fromUtf8("\x41\x50\x50\x46\x6C\x6F\x77\x20\xE4\xB8\x8E\xE6\x89\x93\xE5\xBC\x80\xE7\x9B\xAE\xE5\xBD\x95\xE4\xBD\xBF\xE7\x94\xA8\xE7\x9A\x84\xE6\x9C\xAC\xE6\x9C\xBA\xE8\xB7\xAF\xE5\xBE\x84"));

    auto *backendLabel = new QLabel(QString::fromUtf8("\xE5\x90\x8E\xE7\xAB\xAF\xE8\xBE\x93\xE5\x87\xBA"), this);
    backendLabel->setObjectName("workflowWorkspaceLabel");

    m_backendOutputDirectoryEdit = new QLineEdit(this);
    m_backendOutputDirectoryEdit->setObjectName("workflowWorkspacePath");
    m_backendOutputDirectoryEdit->setPlaceholderText(QString::fromUtf8("\xE5\x90\x8E\xE7\xAB\xAF\x2F\xE5\xAE\xB9\xE5\x99\xA8\xE8\xB7\xAF\xE5\xBE\x84\xEF\xBC\x8C\xE4\xBE\x8B\xE5\xA6\x82\x20\x2F\x77\x6F\x72\x6B\x73\x70\x61\x63\x65\x2F\x6F\x75\x74\x70\x75\x74\x73"));

    m_browseButton = new QPushButton(QString::fromUtf8("\xE6\xB5\x8F\xE8\xA7\x88"), this);
    m_browseButton->setObjectName("workflowWorkspaceBrowseButton");

    m_statusLabel = new QLabel(QString::fromUtf8("\xE7\xA9\xBA\xE9\x97\xB2"), this);
    m_statusLabel->setObjectName("workflowWorkspaceStatus");

    layout->addWidget(hostLabel, 0, 0);
    layout->addWidget(m_outputDirectoryEdit, 0, 1);
    layout->addWidget(m_browseButton, 0, 2);
    layout->addWidget(m_statusLabel, 0, 3);
    layout->addWidget(backendLabel, 1, 0);
    layout->addWidget(m_backendOutputDirectoryEdit, 1, 1, 1, 3);
    layout->setColumnStretch(1, 1);

    connect(m_browseButton, &QPushButton::clicked,
            this, &WorkflowWorkspaceBar::browseOutputDirectoryRequested);
    connect(m_outputDirectoryEdit, &QLineEdit::editingFinished, this, [this]() {
        emit outputDirectoryChanged(outputDirectory());
    });
    connect(m_backendOutputDirectoryEdit, &QLineEdit::editingFinished, this, [this]() {
        emit backendOutputDirectoryChanged(backendOutputDirectory());
    });
}

QString WorkflowWorkspaceBar::outputDirectory() const
{
    return m_outputDirectoryEdit->text().trimmed();
}

QString WorkflowWorkspaceBar::backendOutputDirectory() const
{
    return m_backendOutputDirectoryEdit->text().trimmed();
}

void WorkflowWorkspaceBar::setOutputDirectory(const QString &path)
{
    if (m_outputDirectoryEdit->text() == path) {
        return;
    }

    m_outputDirectoryEdit->setText(path);
    emit outputDirectoryChanged(path);
}

void WorkflowWorkspaceBar::setBackendOutputDirectory(const QString &path)
{
    if (m_backendOutputDirectoryEdit->text() == path) {
        return;
    }

    m_backendOutputDirectoryEdit->setText(path);
    emit backendOutputDirectoryChanged(path);
}

void WorkflowWorkspaceBar::setTaskRunning(bool running)
{
    m_statusLabel->setText(running ? QString::fromUtf8("\xE7\x94\x9F\xE6\x88\x90\xE4\xB8\xAD") : QString::fromUtf8("\xE7\xA9\xBA\xE9\x97\xB2"));
    m_outputDirectoryEdit->setEnabled(!running);
    m_backendOutputDirectoryEdit->setEnabled(!running);
    m_browseButton->setEnabled(!running);
}
