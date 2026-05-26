#include "MessageList.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QVariant>
#include <QVBoxLayout>

MessageList::MessageList(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("messageList");

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setObjectName("messageScrollArea");
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    auto *content = new QWidget(m_scrollArea);
    m_messageLayout = new QVBoxLayout(content);
    m_messageLayout->setContentsMargins(24, 18, 24, 18);
    m_messageLayout->setSpacing(14);

    m_messageLayout->addStretch(1);

    m_scrollArea->setWidget(content);
    rootLayout->addWidget(m_scrollArea);
}

void MessageList::addUserMessage(const QString &text)
{
    addMessage(ChatMessage{ChatMessageRole::User, text});
}

void MessageList::addAgentMessage(const QString &text)
{
    finishStreamingAgentMessage();
    addMessage(ChatMessage{ChatMessageRole::Agent, text});
}

void MessageList::addToolMessage(const QString &text)
{
    addMessage(ChatMessage{ChatMessageRole::Tool, text});
}

void MessageList::addErrorMessage(const QString &text)
{
    addMessage(ChatMessage{ChatMessageRole::Error, text});
}

void MessageList::beginStreamingAgentMessage()
{
    finishStreamingAgentMessage();
    m_streamingAgentBubble = addMessage(ChatMessage{ChatMessageRole::Agent, QString()});
}

void MessageList::appendStreamingAgentMessageText(const QString &text)
{
    if (text.isEmpty()) {
        return;
    }
    if (m_streamingAgentBubble == nullptr) {
        beginStreamingAgentMessage();
    }

    m_streamingAgentBubble->setText(m_streamingAgentBubble->text() + text);
    scrollToBottom();
}

void MessageList::finishStreamingAgentMessage()
{
    m_streamingAgentBubble = nullptr;
}

void MessageList::setRepairMessage(const QString &text, const QString &actionLabel)
{
    const QString content = text.trimmed();
    if (content.isEmpty()) {
        if (m_repairMessageRow != nullptr) {
            m_messageLayout->removeWidget(m_repairMessageRow);
            m_repairMessageRow->deleteLater();
            m_repairMessageRow = nullptr;
            m_repairMessageBubble = nullptr;
            m_repairActionButton = nullptr;
        }
        return;
    }

    if (m_repairMessageRow == nullptr) {
        m_repairMessageBubble = addMessage(ChatMessage{ChatMessageRole::Tool, content});
        QWidget *stack = m_repairMessageBubble ? m_repairMessageBubble->parentWidget() : nullptr;
        m_repairMessageRow = stack ? stack->parentWidget() : nullptr;
        auto *stackLayout = stack ? qobject_cast<QVBoxLayout *>(stack->layout()) : nullptr;
        if (stackLayout != nullptr) {
            m_repairActionButton = new QPushButton(stack);
            m_repairActionButton->setObjectName("repairActionButton");
            connect(m_repairActionButton, &QPushButton::clicked,
                    this, &MessageList::repairActionRequested);
            stackLayout->addWidget(m_repairActionButton, 0, Qt::AlignLeft);
        }
    }

    if (m_repairMessageBubble != nullptr) {
        m_repairMessageBubble->setText(content);
    }
    if (m_repairActionButton != nullptr) {
        const QString buttonText = actionLabel.trimmed();
        m_repairActionButton->setVisible(!buttonText.isEmpty());
        m_repairActionButton->setText(buttonText);
    }
    scrollToBottom();
}

QLabel *MessageList::addMessage(const ChatMessage &message)
{
    const QString sender = displayNameForRole(message.role);
    auto *row = new QWidget(this);
    row->setObjectName("messageRow");
    row->setProperty("role", QVariant(styleRoleName(message.role)));

    auto *rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(0);

    auto *stack = new QWidget(row);
    stack->setObjectName("messageStack");
    stack->setMaximumWidth(760);

    auto *stackLayout = new QVBoxLayout(stack);
    stackLayout->setContentsMargins(0, 0, 0, 0);
    stackLayout->setSpacing(4);

    auto *senderLabel = new QLabel(sender, stack);
    senderLabel->setObjectName("messageSender");
    senderLabel->setProperty("role", QVariant(styleRoleName(message.role)));

    auto *bubble = new QLabel(message.content, stack);
    bubble->setObjectName("messageBubble");
    bubble->setProperty("role", QVariant(styleRoleName(message.role)));
    bubble->setWordWrap(true);
    bubble->setMinimumHeight(40);
    bubble->setTextInteractionFlags(Qt::TextSelectableByMouse);

    stackLayout->addWidget(senderLabel);
    stackLayout->addWidget(bubble);

    if (message.role == ChatMessageRole::User) {
        rowLayout->addStretch(1);
        rowLayout->addWidget(stack);
    } else {
        rowLayout->addWidget(stack);
        rowLayout->addStretch(1);
    }

    const int insertIndex = qMax(0, m_messageLayout->count() - 1);
    m_messageLayout->insertWidget(insertIndex, row);

    scrollToBottom();
    return bubble;
}

QString MessageList::displayNameForRole(ChatMessageRole role) const
{
    switch (role) {
    case ChatMessageRole::User:
        return QString::fromUtf8("\xE7\x94\xA8\xE6\x88\xB7");
    case ChatMessageRole::Agent:
        return QString::fromUtf8("\xE6\x99\xBA\xE8\x83\xBD\xE4\xBD\x93");
    case ChatMessageRole::Tool:
        return QString::fromUtf8("\xE5\xB7\xA5\xE4\xBD\x9C\xE6\xB5\x81");
    case ChatMessageRole::Error:
        return QString::fromUtf8("\xE9\x94\x99\xE8\xAF\xAF");
    }

    return QString::fromUtf8("\xE6\xB6\x88\xE6\x81\xAF");
}

QString MessageList::styleRoleName(ChatMessageRole role) const
{
    switch (role) {
    case ChatMessageRole::User:
        return QStringLiteral("user");
    case ChatMessageRole::Agent:
        return QStringLiteral("agent");
    case ChatMessageRole::Tool:
        return QStringLiteral("tool");
    case ChatMessageRole::Error:
        return QStringLiteral("error");
    }

    return QStringLiteral("message");
}

void MessageList::scrollToBottom()
{
    QTimer::singleShot(0, this, [this]() {
        if (m_scrollArea && m_scrollArea->verticalScrollBar()) {
            m_scrollArea->verticalScrollBar()->setValue(m_scrollArea->verticalScrollBar()->maximum());
        }
    });
}
