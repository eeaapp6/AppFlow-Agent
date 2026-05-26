#include "AgentTypingRenderer.h"

#include "MessageList.h"

#include <QTimer>
#include <QtGlobal>

namespace {

const int typingIntervalMs = 18;
const int typingChunkLength = 2;

}

AgentTypingRenderer::AgentTypingRenderer(MessageList *messageList, QObject *parent)
    : QObject(parent)
    , m_messageList(messageList)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(typingIntervalMs);
    connect(m_timer, &QTimer::timeout,
            this, &AgentTypingRenderer::appendNextChunk);
}

void AgentTypingRenderer::queueMessage(const QString &text)
{
    const QString cleanText = text.trimmed();
    if (cleanText.isEmpty() || m_messageList == nullptr) {
        return;
    }

    m_pendingMessages.append(cleanText);
    if (!m_timer->isActive() && m_currentMessage.isEmpty()) {
        playNextMessage();
    }
}

void AgentTypingRenderer::finishCurrentMessage()
{
    if (m_timer != nullptr) {
        m_timer->stop();
    }
    if (m_messageList != nullptr) {
        m_messageList->finishStreamingAgentMessage();
    }
    m_currentMessage.clear();
    m_currentIndex = 0;
}

void AgentTypingRenderer::playNextMessage()
{
    if (m_pendingMessages.isEmpty() || m_messageList == nullptr) {
        return;
    }

    m_currentMessage = m_pendingMessages.takeFirst();
    m_currentIndex = 0;
    m_messageList->beginStreamingAgentMessage();
    m_timer->start();
}

void AgentTypingRenderer::appendNextChunk()
{
    if (m_currentMessage.isEmpty()) {
        finishCurrentMessage();
        playNextMessage();
        return;
    }

    const int remaining = m_currentMessage.size() - m_currentIndex;
    if (remaining <= 0) {
        finishCurrentMessage();
        playNextMessage();
        return;
    }

    const int chunkLength = qMin(typingChunkLength, remaining);
    m_messageList->appendStreamingAgentMessageText(m_currentMessage.mid(m_currentIndex, chunkLength));
    m_currentIndex += chunkLength;

    if (m_currentIndex >= m_currentMessage.size()) {
        finishCurrentMessage();
        playNextMessage();
    }
}
