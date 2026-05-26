#pragma once

#include <QString>

enum class ChatMessageRole
{
    User,
    Agent,
    Tool,
    Error
};

struct ChatMessage
{
    ChatMessageRole role = ChatMessageRole::Agent;
    QString content;
};
