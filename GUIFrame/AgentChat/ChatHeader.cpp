#include "ChatHeader.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

ChatHeader::ChatHeader(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("chatHeader");

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 14, 20, 12);
    layout->setSpacing(10);

    m_titleLabel = new QLabel(QString::fromUtf8("\x46\x6F\x61\x6D\x2D\x41\x67\x65\x6E\x74\x20\xE4\xBB\xBF\xE7\x9C\x9F\xE5\x8A\xA9\xE6\x89\x8B"), this);
    m_titleLabel->setObjectName("chatHeaderTitle");

    m_statusLabel = new QLabel(QString::fromUtf8("\xE7\xA9\xBA\xE9\x97\xB2"), this);
    m_statusLabel->setObjectName("chatHeaderStatus");

    m_settingsButton = new QPushButton(QString::fromUtf8("\xE8\xAE\xBE\xE7\xBD\xAE"), this);
    m_settingsButton->setObjectName("chatHeaderSettingsButton");

    m_stopButton = new QPushButton(QString::fromUtf8("\xE5\x81\x9C\xE6\xAD\xA2"), this);
    m_stopButton->setObjectName("chatHeaderStopButton");
    m_stopButton->setEnabled(false);

    layout->addWidget(m_titleLabel);
    layout->addStretch(1);
    layout->addWidget(m_settingsButton);
    layout->addWidget(m_statusLabel);
    layout->addWidget(m_stopButton);

    connect(m_settingsButton, &QPushButton::clicked, this, &ChatHeader::settingsRequested);
    connect(m_stopButton, &QPushButton::clicked, this, &ChatHeader::stopRequested);
}

void ChatHeader::setTaskRunning(bool running)
{
    m_statusLabel->setText(running ? QString::fromUtf8("\xE8\xBF\x90\xE8\xA1\x8C\xE4\xB8\xAD") : QString::fromUtf8("\xE7\xA9\xBA\xE9\x97\xB2"));
    m_stopButton->setEnabled(running);
}
