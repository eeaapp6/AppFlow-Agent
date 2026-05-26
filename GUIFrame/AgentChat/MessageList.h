#pragma once

#include "models/ChatMessage.h"

#include <QString>
#include <QWidget>

class QVBoxLayout;
class QScrollArea;
class QLabel;
class QPushButton;

class MessageList : public QWidget
{
    Q_OBJECT

public:
    explicit MessageList(QWidget *parent = nullptr);
    void addUserMessage(const QString &text);
    void addAgentMessage(const QString &text);
    void addToolMessage(const QString &text);
    void addErrorMessage(const QString &text);
    void beginStreamingAgentMessage();
    void appendStreamingAgentMessageText(const QString &text);
    void finishStreamingAgentMessage();
    void setRepairMessage(const QString &text, const QString &actionLabel = QString());

signals:
    void repairActionRequested();

private:
    QLabel *addMessage(const ChatMessage &message);
    QString displayNameForRole(ChatMessageRole role) const;
    QString styleRoleName(ChatMessageRole role) const;
    void scrollToBottom();

    QVBoxLayout *m_messageLayout = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QLabel *m_streamingAgentBubble = nullptr;
    QWidget *m_repairMessageRow = nullptr;
    QLabel *m_repairMessageBubble = nullptr;
    QPushButton *m_repairActionButton = nullptr;
};
