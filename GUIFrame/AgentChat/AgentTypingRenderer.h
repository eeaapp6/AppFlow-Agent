#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

class MessageList;
class QTimer;

class AgentTypingRenderer : public QObject
{
public:
    explicit AgentTypingRenderer(MessageList *messageList, QObject *parent = nullptr);

    void queueMessage(const QString &text);
    void finishCurrentMessage();

private:
    void playNextMessage();
    void appendNextChunk();

    MessageList *m_messageList = nullptr;
    QTimer *m_timer = nullptr;
    QStringList m_pendingMessages;
    QString m_currentMessage;
    int m_currentIndex = 0;
};
