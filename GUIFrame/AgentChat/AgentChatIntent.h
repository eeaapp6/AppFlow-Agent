#pragma once

#include <QString>

namespace AgentChatIntent
{
    QString nextActionTitle(const QString &label, const QString &id);
    bool isNextActionConfirmation(const QString &text);
    bool isParameterChangeRequest(const QString &text);
    QString taskIdFromDirectory(const QString &taskDir);
}
