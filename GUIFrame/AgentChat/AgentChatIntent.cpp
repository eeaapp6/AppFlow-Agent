#include "AgentChatIntent.h"

#include <QStringList>

namespace AgentChatIntent
{

QString nextActionTitle(const QString &label, const QString &id)
{
    const QString cleanLabel = label.trimmed();
    if (!cleanLabel.isEmpty()) {
        return cleanLabel;
    }

    return id.trimmed();
}

bool isNextActionConfirmation(const QString &text)
{
    QString normalized = text.trimmed().toLower();
    normalized.remove(QStringLiteral(" "));
    normalized.remove(QStringLiteral("."));
    normalized.remove(QStringLiteral("!"));
    normalized.remove(QStringLiteral("?"));
    normalized.remove(QString::fromUtf8("\xEF\xBC\x81"));
    normalized.remove(QString::fromUtf8("\xEF\xBC\x9F"));
    normalized.remove(QString::fromUtf8("\xE3\x80\x82"));
    normalized.remove(QString::fromUtf8("\xEF\xBC\x8C"));
    normalized.remove(QString::fromUtf8("\xEF\xBC\x9B"));

    const QStringList confirmations = {
        QString::fromUtf8("\xE7\xBB\xA7\xE7\xBB\xAD"),
        QString::fromUtf8("\xE4\xB8\x8B\xE4\xB8\x80\xE6\xAD\xA5"),
        QString::fromUtf8("\xE6\x89\xA7\xE8\xA1\x8C"),
        QString::fromUtf8("\xE5\x8F\xAF\xE4\xBB\xA5"),
        QString::fromUtf8("\xE7\xA1\xAE\xE8\xAE\xA4"),
        QString::fromUtf8("\xE5\xA5\xBD\xE7\x9A\x84"),
        QString::fromUtf8("\xE5\xA5\xBD"),
        QString::fromUtf8("\xE8\xA1\x8C"),
        QString::fromUtf8("\xE7\xA1\xAE\xE5\xAE\x9A"),
        QString::fromUtf8("\xE5\xBC\x80\xE5\xA7\x8B"),
        QStringLiteral("ok"),
        QStringLiteral("yes"),
        QStringLiteral("continue"),
        QStringLiteral("next"),
        QStringLiteral("go"),
    };

    return confirmations.contains(normalized);
}

bool isParameterChangeRequest(const QString &text)
{
    const QString normalized = text.trimmed().toLower();
    if (normalized.isEmpty()) {
        return false;
    }

    const QStringList actionKeywords = {
        QString::fromUtf8("\xE6\x94\xB9"),
        QString::fromUtf8("\xE4\xBF\xAE\xE6\x94\xB9"),
        QString::fromUtf8("\xE8\xB0\x83\xE6\x95\xB4"),
        QString::fromUtf8("\xE8\xAE\xBE\xE7\xBD\xAE"),
        QString::fromUtf8("\xE8\xAE\xBE\xE4\xB8\xBA"),
        QString::fromUtf8("\xE5\x8F\x98\xE6\x88\x90"),
        QStringLiteral("change"),
        QStringLiteral("set"),
        QStringLiteral("update"),
    };
    const QStringList parameterKeywords = {
        QStringLiteral("delta_t"),
        QStringLiteral("deltat"),
        QStringLiteral("end_time"),
        QStringLiteral("endtime"),
        QStringLiteral("write_interval"),
        QStringLiteral("writeinterval"),
        QStringLiteral("nu"),
        QStringLiteral("rho"),
        QStringLiteral("viscosity"),
        QStringLiteral("velocity"),
        QStringLiteral("pressure"),
        QStringLiteral("max_co"),
        QStringLiteral("maxco"),
    };

    bool hasAction = false;
    for (const QString &keyword : actionKeywords) {
        if (normalized.contains(keyword)) {
            hasAction = true;
            break;
        }
    }
    if (!hasAction) {
        return false;
    }

    for (const QString &keyword : parameterKeywords) {
        if (normalized.contains(keyword)) {
            return true;
        }
    }

    return false;
}

QString taskIdFromDirectory(const QString &taskDir)
{
    const QString normalized = taskDir.trimmed().replace('\\', '/');
    return normalized.section('/', -1, -1);
}

}
