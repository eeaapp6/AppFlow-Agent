#include "GateStatusBar.h"

#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QStyle>
#include <QStringList>
#include <QVariant>

namespace {

struct GateItem
{
    const char *key;
    const char *label;
};

const QList<GateItem> kGateItems = {
    {"capability", "Capability"},
    {"spec", "Spec"},
    {"generation", "Generation"},
    {"static_validation", "Validation"},
    {"execution", "Execution"},
    {"result_review", "Result"},
    {"manifest", "Manifest"},
};

QString normalizedStatus(const QString &status)
{
    const QString value = status.trimmed().toLower();
    if (value == QStringLiteral("passed")
        || value == QStringLiteral("warning")
        || value == QStringLiteral("failed")
        || value == QStringLiteral("unsupported")) {
        return value;
    }
    return QStringLiteral("pending");
}

QString statusText(const QString &status)
{
    if (status == QStringLiteral("passed")) {
        return QStringLiteral("Passed");
    }
    if (status == QStringLiteral("warning")) {
        return QStringLiteral("Warning");
    }
    if (status == QStringLiteral("failed")) {
        return QStringLiteral("Failed");
    }
    if (status == QStringLiteral("unsupported")) {
        return QStringLiteral("Unsupported");
    }
    return QStringLiteral("Pending");
}

void refreshStatusStyle(QLabel *label)
{
    label->style()->unpolish(label);
    label->style()->polish(label);
    label->update();
}

QString issueLine(const QJsonObject &issue)
{
    const QString code = issue.value(QStringLiteral("code")).toString().trimmed();
    const QString message = issue.value(QStringLiteral("message")).toString().trimmed();
    if (code.isEmpty()) {
        return message;
    }
    if (message.isEmpty()) {
        return code;
    }
    return QStringLiteral("%1: %2").arg(code, message);
}

QString gateToolTip(const QJsonObject &review)
{
    QStringList sections;

    const QJsonArray issues = review.value(QStringLiteral("issues")).toArray();
    if (!issues.isEmpty()) {
        QStringList issueLines;
        const int limit = qMin(issues.size(), 5);
        for (int index = 0; index < limit; ++index) {
            if (!issues.at(index).isObject()) {
                continue;
            }
            const QString line = issueLine(issues.at(index).toObject());
            if (!line.isEmpty()) {
                issueLines << QStringLiteral("- %1").arg(line);
            }
        }
        if (issues.size() > limit) {
            issueLines << QStringLiteral("- ...");
        }
        if (!issueLines.isEmpty()) {
            sections << QStringLiteral("Issues:\n%1").arg(issueLines.join(QStringLiteral("\n")));
        }
    }

    return sections.join(QStringLiteral("\n\n"));
}

}

GateStatusBar::GateStatusBar(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("gateStatusBar");

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 8, 20, 8);
    layout->setSpacing(8);

    m_summaryLabel = new QLabel(QStringLiteral("Gates: pending"), this);
    m_summaryLabel->setObjectName("gateStatusSummary");
    layout->addWidget(m_summaryLabel);
    layout->addSpacing(4);

    for (const GateItem &item : kGateItems) {
        auto *label = new QLabel(this);
        label->setObjectName("gateStatusChip");
        label->setProperty("gate", QVariant(QString::fromLatin1(item.key)));
        label->setProperty("gateLabel", QVariant(QString::fromLatin1(item.label)));
        m_gateLabels.append(label);
        layout->addWidget(label);
    }

    layout->addStretch(1);
    clear();
}

void GateStatusBar::clear()
{
    for (QLabel *label : m_gateLabels) {
        const QString gateLabel = label->property("gateLabel").toString();
        label->setText(QStringLiteral("%1: Pending").arg(gateLabel));
        label->setProperty("status", QVariant(QStringLiteral("pending")));
        label->setToolTip(QString());
        refreshStatusStyle(label);
    }
    updateSummary();
}

void GateStatusBar::setGateReviews(const QJsonArray &reviews)
{
    clear();
    for (const QJsonValue &value : reviews) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject review = value.toObject();
        const QString gate = review.value(QStringLiteral("gate")).toString().trimmed();
        if (gate.isEmpty()) {
            continue;
        }
        applyGateReview(review);
    }
    updateSummary();
}

void GateStatusBar::applyGateReview(const QJsonObject &review)
{
    const QString gate = review.value(QStringLiteral("gate")).toString().trimmed();
    const QString status = review.value(QStringLiteral("status")).toString();
    const QString normalized = normalizedStatus(status);
    for (QLabel *label : m_gateLabels) {
        if (label->property("gate").toString() != gate) {
            continue;
        }

        const QString gateLabel = label->property("gateLabel").toString();
        label->setText(QStringLiteral("%1: %2").arg(gateLabel, statusText(normalized)));
        label->setProperty("status", QVariant(normalized));
        label->setToolTip(gateToolTip(review));
        refreshStatusStyle(label);
        return;
    }
}

void GateStatusBar::updateSummary()
{
    int completed = 0;
    int failed = 0;
    int warnings = 0;
    for (const QLabel *label : m_gateLabels) {
        const QString status = label->property("status").toString();
        if (status == QStringLiteral("passed")) {
            ++completed;
        } else if (status == QStringLiteral("warning")) {
            ++completed;
            ++warnings;
        } else if (status == QStringLiteral("failed") || status == QStringLiteral("unsupported")) {
            ++completed;
            ++failed;
        }
    }

    QString state = QStringLiteral("pending");
    if (failed > 0) {
        state = QStringLiteral("failed");
    } else if (warnings > 0) {
        state = QStringLiteral("warning");
    } else if (completed > 0) {
        state = QStringLiteral("passed");
    }

    m_summaryLabel->setText(QStringLiteral("Gates: %1/%2 %3").arg(completed).arg(m_gateLabels.size()).arg(state));
}
