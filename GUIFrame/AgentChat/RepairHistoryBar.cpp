#include "RepairHistoryBar.h"

#include <QHBoxLayout>
#include <QJsonObject>
#include <QLabel>
#include <QStringList>

namespace {

QString displayValue(const QJsonObject &record, const QString &key)
{
    return record.value(key).toString().trimmed();
}

QString latestRepairText(const QJsonObject &record)
{
    const QString label = displayValue(record, QStringLiteral("label"));
    const QString actionId = displayValue(record, QStringLiteral("action_id"));
    const QString sourceGate = displayValue(record, QStringLiteral("source_gate"));
    const QString status = displayValue(record, QStringLiteral("status"));
    const QJsonObject followUp = record.value(QStringLiteral("follow_up")).toObject();
    const QString validationStatus = displayValue(followUp, QStringLiteral("validation_status"));

    QStringList parts;
    parts << QStringLiteral("Repair: %1").arg(label.isEmpty() ? actionId : label);
    if (!sourceGate.isEmpty()) {
        parts << QStringLiteral("Source: %1").arg(sourceGate);
    }
    if (!status.isEmpty()) {
        parts << QStringLiteral("Status: %1").arg(status);
    }
    if (!validationStatus.isEmpty()) {
        parts << QStringLiteral("Follow-up: %1").arg(validationStatus);
    }
    return parts.join(QStringLiteral("  |  "));
}

}

RepairHistoryBar::RepairHistoryBar(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("repairHistoryBar");

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 7, 20, 7);
    layout->setSpacing(8);

    m_label = new QLabel(this);
    m_label->setObjectName("repairHistoryLabel");
    m_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(m_label);
    layout->addStretch(1);

    clear();
}

void RepairHistoryBar::clear()
{
    m_label->setText(QString());
    setVisible(false);
}

void RepairHistoryBar::setRepairHistory(const QJsonArray &history)
{
    QJsonObject latest;
    for (int index = history.size() - 1; index >= 0; --index) {
        if (history.at(index).isObject()) {
            latest = history.at(index).toObject();
            break;
        }
    }

    if (latest.isEmpty()) {
        clear();
        return;
    }

    m_label->setText(latestRepairText(latest));
    setVisible(true);
}
