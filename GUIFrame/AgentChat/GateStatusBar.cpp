#include "GateStatusBar.h"

#include <QApplication>
#include <QClipboard>
#include <QFrame>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QRegularExpression>
#include <QPushButton>
#include <QSet>
#include <QSizePolicy>
#include <QStyle>
#include <QStringList>
#include <QTimer>
#include <QVariant>
#include <QVBoxLayout>

namespace {

struct GateItem
{
    const char *key;
    const char *label;
};

struct GatePresentation
{
    const char *key;
    const char *stage;
    const char *title;
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

const QList<GatePresentation> kFailurePriority = {
    {"result_review", "\xE7\xBB\x93\xE6\x9E\x9C\xE5\xAE\xA1\xE6\x9F\xA5", "\xE8\xAE\xA1\xE7\xAE\x97\xE7\xBB\x93\xE6\x9E\x9C\xE6\x9C\xAA\xE9\x80\x9A\xE8\xBF\x87\xE5\xAE\xA1\xE6\x9F\xA5"},
    {"execution", "\xE8\xBF\x90\xE8\xA1\x8C", "\x4F\x70\x65\x6E\x46\x4F\x41\x4D\x20\xE6\x89\xA7\xE8\xA1\x8C\xE5\xA4\xB1\xE8\xB4\xA5\xE6\x88\x96\xE8\xA2\xAB\xE9\x98\xBB\xE5\xA1\x9E"},
    {"physics_sanity", "\xE7\x89\xA9\xE7\x90\x86\xE5\x90\x88\xE7\x90\x86\xE6\x80\xA7", "\xE7\x89\xA9\xE7\x90\x86\xE5\x90\x88\xE7\x90\x86\xE6\x80\xA7\xE6\xA3\x80\xE6\x9F\xA5\xE5\xA4\xB1\xE8\xB4\xA5"},
    {"static_validation", "\xE6\xA0\xA1\xE9\xAA\x8C", "\xE7\xAE\x97\xE4\xBE\x8B\xE6\xA0\xA1\xE9\xAA\x8C\xE5\xA4\xB1\xE8\xB4\xA5"},
    {"generation", "\xE7\x94\x9F\xE6\x88\x90", "\xE7\xAE\x97\xE4\xBE\x8B\xE7\x94\x9F\xE6\x88\x90\xE6\x9C\xAA\xE9\x80\x9A\xE8\xBF\x87\xE6\xA3\x80\xE6\x9F\xA5"},
    {"spec", "\xE4\xBB\xBF\xE7\x9C\x9F\xE8\xA7\x84\xE6\xA0\xBC", "\xE4\xBB\xBF\xE7\x9C\x9F\xE8\xA7\x84\xE6\xA0\xBC\xE5\xAE\xA1\xE6\x9F\xA5\xE6\x9C\xAA\xE9\x80\x9A\xE8\xBF\x87"},
    {"capability", "\xE8\x83\xBD\xE5\x8A\x9B", "\xE5\xBD\x93\xE5\x89\x8D\xE9\x9C\x80\xE6\xB1\x82\xE6\x9A\x82\xE4\xB8\x8D\xE5\x8F\x97\xE6\x94\xAF\xE6\x8C\x81"},
    {"manifest", "\xE5\xAF\xBC\xE5\x85\xA5", "\x41\x50\x50\x46\x6C\x6F\x77\x20\xE5\xAF\xBC\xE5\x85\xA5\xE6\xB8\x85\xE5\x8D\x95\xE6\x9C\xAA\xE5\xB0\xB1\xE7\xBB\xAA"},
};

constexpr int kMaximumFailureIssues = 5;
constexpr int kMaximumFailureTextLength = 500;
constexpr int kMaximumFailureCopyLength = 6000;
constexpr int kFailureCopyFeedbackDurationMs = 1500;

struct FailureIssue
{
    QString code;
    QString message;
    QString severity;
};

QString limitedPlainText(const QString &value)
{
    QString text = value.trimmed();
    static const QRegularExpression apiKeyAssignment(
        QStringLiteral("(api[_-]?key\\s*[:=]\\s*)\\S+"),
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression bearerToken(
        QStringLiteral("(authorization\\s*[:=]\\s*bearer\\s+)\\S+"),
        QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression openAiStyleToken(
        QStringLiteral("\\bsk-[A-Za-z0-9_-]{8,}\\b"));
    text.replace(apiKeyAssignment, QStringLiteral("\\1[REDACTED]"));
    text.replace(bearerToken, QStringLiteral("\\1[REDACTED]"));
    text.replace(openAiStyleToken, QStringLiteral("[REDACTED]"));
    if (text.size() <= kMaximumFailureTextLength) {
        return text;
    }
    return text.left(kMaximumFailureTextLength - 1).trimmed() + QChar(0x2026);
}

QString strictString(const QJsonObject &object, const QString &key)
{
    const QJsonValue value = object.value(key);
    if (!value.isString()) {
        return QString();
    }
    return limitedPlainText(value.toString());
}

const GatePresentation *presentationForGate(const QString &gate)
{
    for (const GatePresentation &presentation : kFailurePriority) {
        if (gate == QString::fromLatin1(presentation.key)) {
            return &presentation;
        }
    }
    return nullptr;
}

QString failureStage(const QString &gate)
{
    const GatePresentation *presentation = presentationForGate(gate);
    return presentation ? QString::fromUtf8(presentation->stage) : limitedPlainText(gate);
}

QString failureTitle(const QString &gate)
{
    const GatePresentation *presentation = presentationForGate(gate);
    return presentation
        ? QString::fromUtf8(presentation->title)
        : QString::fromUtf8("\xE5\xB7\xA5\xE4\xBD\x9C\xE6\xB5\x81\xE6\xA3\x80\xE6\x9F\xA5\xE5\xA4\xB1\xE8\xB4\xA5");
}

QString actionStage(const QString &action)
{
    const QString normalized = action.trimmed().toLower();
    if (normalized == QStringLiteral("plan")) {
        return QString::fromUtf8("\xE8\xA7\x84\xE5\x88\x92");
    }
    if (normalized == QStringLiteral("replan")) {
        return QString::fromUtf8("\xE9\x87\x8D\xE6\x96\xB0\xE8\xA7\x84\xE5\x88\x92");
    }
    if (normalized == QStringLiteral("generate")) {
        return QString::fromUtf8("\xE7\x94\x9F\xE6\x88\x90");
    }
    if (normalized == QStringLiteral("validate")) {
        return QString::fromUtf8("\xE6\xA0\xA1\xE9\xAA\x8C");
    }
    if (normalized == QStringLiteral("run")) {
        return QString::fromUtf8("\xE8\xBF\x90\xE8\xA1\x8C");
    }
    return QString::fromUtf8("\xE8\xAF\xB7\xE6\xB1\x82");
}

QList<FailureIssue> failureIssues(const QJsonObject &review)
{
    QList<FailureIssue> issues;
    const QJsonValue issuesValue = review.value(QStringLiteral("issues"));
    if (!issuesValue.isArray()) {
        return issues;
    }

    QSet<QString> seen;
    for (const QJsonValue &value : issuesValue.toArray()) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject object = value.toObject();
        FailureIssue issue;
        issue.code = strictString(object, QStringLiteral("code"));
        issue.message = strictString(object, QStringLiteral("message"));
        issue.severity = strictString(object, QStringLiteral("severity")).toLower();
        if (issue.severity.isEmpty()) {
            issue.severity = QStringLiteral("error");
        }
        if (issue.code.isEmpty() && issue.message.isEmpty()) {
            continue;
        }

        const QString dedupeKey = issue.code + QChar(0x001f) + issue.message;
        if (seen.contains(dedupeKey)) {
            continue;
        }
        seen.insert(dedupeKey);
        issues.append(issue);
    }
    return issues;
}

QString failureIssueText(const FailureIssue &issue, int index)
{
    QString content;
    if (issue.code.isEmpty()) {
        content = issue.message;
    } else if (issue.message.isEmpty()) {
        content = issue.code;
    } else {
        content = QString::fromUtf8("\x25\x31\xEF\xBC\x9A\x25\x32").arg(issue.code, issue.message);
    }
    return QStringLiteral("%1. %2").arg(index + 1).arg(content);
}

QJsonObject diagnosticsObject(const QJsonObject &review)
{
    const QJsonValue diagnostics = review.value(QStringLiteral("diagnostics"));
    return diagnostics.isObject() ? diagnostics.toObject() : QJsonObject();
}

QString failureSummary(const QJsonObject &review, const QList<FailureIssue> &issues)
{
    const QJsonObject diagnostics = diagnosticsObject(review);
    const QString summary = strictString(diagnostics, QStringLiteral("summary"));
    if (!summary.isEmpty()) {
        return summary;
    }

    const QJsonValue primaryIssueValue = diagnostics.value(QStringLiteral("primary_issue"));
    if (primaryIssueValue.isObject()) {
        const QString primaryMessage = strictString(primaryIssueValue.toObject(), QStringLiteral("message"));
        if (!primaryMessage.isEmpty()) {
            return primaryMessage;
        }
    }

    for (const FailureIssue &issue : issues) {
        if (!issue.message.isEmpty()) {
            return issue.message;
        }
    }
    return failureTitle(strictString(review, QStringLiteral("gate")));
}

QString failureEvidence(const QJsonObject &review)
{
    const QJsonObject diagnostics = diagnosticsObject(review);
    const QJsonValue evidenceValue = diagnostics.value(QStringLiteral("evidence"));
    if (!evidenceValue.isObject()) {
        return QString();
    }

    const QJsonObject evidence = evidenceValue.toObject();
    QString location = strictString(evidence, QStringLiteral("log_path"));
    const QJsonValue lineValue = evidence.value(QStringLiteral("line"));
    if (!location.isEmpty() && lineValue.isDouble() && lineValue.toDouble() > 0) {
        location += QStringLiteral(":%1").arg(static_cast<qint64>(lineValue.toDouble()));
    }

    const QString excerpt = strictString(evidence, QStringLiteral("excerpt"));
    QStringList lines;
    if (!location.isEmpty()) {
        lines << QString::fromUtf8("\xE8\xAF\x81\xE6\x8D\xAE\xEF\xBC\x9A\x25\x31").arg(location);
    }
    if (!excerpt.isEmpty()) {
        lines << excerpt;
    }
    return lines.join(QChar('\n'));
}

QString failureSeverity(const QJsonObject &review, const QList<FailureIssue> &issues)
{
    const QString status = strictString(review, QStringLiteral("status")).toLower();
    if (status == QStringLiteral("unsupported")) {
        return QStringLiteral("unsupported");
    }

    const QString diagnosticsSeverity = strictString(
        diagnosticsObject(review), QStringLiteral("severity")).toLower();
    if (diagnosticsSeverity == QStringLiteral("blocked")) {
        return QStringLiteral("blocked");
    }
    for (const FailureIssue &issue : issues) {
        if (issue.severity == QStringLiteral("blocked")) {
            return QStringLiteral("blocked");
        }
    }
    return QStringLiteral("error");
}

QString failureStatusText(const QString &severity)
{
    if (severity == QStringLiteral("unsupported")) {
        return QString::fromUtf8("\xE4\xB8\x8D\xE6\x94\xAF\xE6\x8C\x81");
    }
    if (severity == QStringLiteral("blocked")) {
        return QString::fromUtf8("\xE5\xB7\xB2\xE9\x98\xBB\xE5\xA1\x9E");
    }
    if (severity == QStringLiteral("uncertain")) {
        return QString::fromUtf8("\xE7\x8A\xB6\xE6\x80\x81\xE4\xB8\x8D\xE7\xA1\xAE\xE5\xAE\x9A");
    }
    return QString::fromUtf8("\xE5\xA4\xB1\xE8\xB4\xA5");
}

QString boundedCopyText(const QStringList &lines)
{
    const QString text = lines.join(QChar('\n')).trimmed();
    if (text.size() <= kMaximumFailureCopyLength) {
        return text;
    }
    QString truncated = text.left(kMaximumFailureCopyLength - 1).trimmed();
    if (!truncated.isEmpty() && truncated.at(truncated.size() - 1).isHighSurrogate()) {
        truncated.chop(1);
    }
    return truncated + QChar(0x2026);
}

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

void refreshFailureStyle(QFrame *card)
{
    card->style()->unpolish(card);
    card->style()->polish(card);
    card->update();
    const QList<QWidget *> descendants = card->findChildren<QWidget *>();
    for (QWidget *widget : descendants) {
        widget->style()->unpolish(widget);
        widget->style()->polish(widget);
        widget->update();
    }
}

void configureFailureTextLabel(QLabel *label, const char *objectName)
{
    label->setObjectName(QString::fromLatin1(objectName));
    label->setTextFormat(Qt::PlainText);
    label->setWordWrap(true);
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    label->setMinimumWidth(0);
    label->setMaximumWidth(1100);
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

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 8, 20, 8);
    layout->setSpacing(8);

    auto *gateRow = new QHBoxLayout();
    gateRow->setContentsMargins(0, 0, 0, 0);
    gateRow->setSpacing(8);

    m_summaryLabel = new QLabel(QStringLiteral("Gates: pending"), this);
    m_summaryLabel->setObjectName("gateStatusSummary");
    gateRow->addWidget(m_summaryLabel);
    gateRow->addSpacing(4);

    for (const GateItem &item : kGateItems) {
        auto *label = new QLabel(this);
        label->setObjectName("gateStatusChip");
        label->setProperty("gate", QVariant(QString::fromLatin1(item.key)));
        label->setProperty("gateLabel", QVariant(QString::fromLatin1(item.label)));
        m_gateLabels.append(label);
        gateRow->addWidget(label);
    }

    gateRow->addStretch(1);
    layout->addLayout(gateRow);

    m_failureCard = new QFrame(this);
    m_failureCard->setObjectName("workflowFailureCard");
    m_failureCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    auto *failureLayout = new QVBoxLayout(m_failureCard);
    failureLayout->setContentsMargins(14, 12, 14, 12);
    failureLayout->setSpacing(6);

    auto *failureHeader = new QWidget(m_failureCard);
    failureHeader->setObjectName("workflowFailureHeader");
    auto *headerLayout = new QHBoxLayout(failureHeader);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(8);
    m_failureStageLabel = new QLabel(failureHeader);
    configureFailureTextLabel(m_failureStageLabel, "workflowFailureStage");
    headerLayout->addWidget(m_failureStageLabel);
    headerLayout->addStretch(1);
    m_failureCopyButton = new QPushButton(
        QString::fromUtf8("\xE5\xA4\x8D\xE5\x88\xB6\xE8\xAF\x8A\xE6\x96\xAD"), failureHeader);
    m_failureCopyButton->setObjectName("workflowFailureCopyButton");
    m_failureCopyButton->setEnabled(false);
    connect(m_failureCopyButton, &QPushButton::clicked,
        this, [this]() { copyFailureDiagnostics(); });
    headerLayout->addWidget(m_failureCopyButton);
    failureLayout->addWidget(failureHeader);

    m_failureTitleLabel = new QLabel(m_failureCard);
    configureFailureTextLabel(m_failureTitleLabel, "workflowFailureTitle");
    failureLayout->addWidget(m_failureTitleLabel);

    m_failureSummaryLabel = new QLabel(m_failureCard);
    configureFailureTextLabel(m_failureSummaryLabel, "workflowFailureSummary");
    failureLayout->addWidget(m_failureSummaryLabel);

    for (int index = 0; index < kMaximumFailureIssues; ++index) {
        auto *label = new QLabel(m_failureCard);
        configureFailureTextLabel(label, "workflowFailureIssue");
        m_failureIssueLabels.append(label);
        failureLayout->addWidget(label);
    }

    m_failureEvidenceLabel = new QLabel(m_failureCard);
    configureFailureTextLabel(m_failureEvidenceLabel, "workflowFailureEvidence");
    failureLayout->addWidget(m_failureEvidenceLabel);

    m_failureMoreLabel = new QLabel(m_failureCard);
    configureFailureTextLabel(m_failureMoreLabel, "workflowFailureMore");
    failureLayout->addWidget(m_failureMoreLabel);

    layout->addWidget(m_failureCard);
    clear();
}

void GateStatusBar::clear()
{
    m_gateReviews = QJsonArray();
    m_uncertainAction.clear();
    resetGateLabels();
    updateSummary();
    hideFailureCard();
}

void GateStatusBar::resetGateLabels()
{
    for (QLabel *label : m_gateLabels) {
        const QString gateLabel = label->property("gateLabel").toString();
        label->setText(QStringLiteral("%1: Pending").arg(gateLabel));
        label->setProperty("status", QVariant(QStringLiteral("pending")));
        label->setToolTip(QString());
        refreshStatusStyle(label);
    }
}

void GateStatusBar::setGateReviews(const QJsonArray &reviews)
{
    m_gateReviews = reviews;
    m_uncertainAction.clear();
    resetGateLabels();
    for (const QJsonValue &value : m_gateReviews) {
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
    updateFailureCard();
}

void GateStatusBar::setRequestUncertain(const QString &action)
{
    m_uncertainAction = action.trimmed().toLower();
    if (m_uncertainAction.isEmpty()) {
        m_uncertainAction = QStringLiteral("request");
    }
    showRequestUncertainty();
}

void GateStatusBar::clearRequestUncertainty()
{
    if (m_uncertainAction.isEmpty()) {
        return;
    }
    m_uncertainAction.clear();
    updateFailureCard();
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

void GateStatusBar::updateFailureCard()
{
    if (!m_uncertainAction.isEmpty()) {
        showRequestUncertainty();
        return;
    }

    QJsonObject selectedReview;
    QSet<QString> failedGates;
    for (const QJsonValue &value : m_gateReviews) {
        if (!value.isObject()) {
            continue;
        }
        const QJsonObject review = value.toObject();
        const QString gate = strictString(review, QStringLiteral("gate"));
        const QString status = strictString(review, QStringLiteral("status")).toLower();
        if (gate.isEmpty()
            || (status != QStringLiteral("failed") && status != QStringLiteral("unsupported"))) {
            continue;
        }
        failedGates.insert(gate);
    }

    for (const GatePresentation &presentation : kFailurePriority) {
        const QString priorityGate = QString::fromLatin1(presentation.key);
        for (const QJsonValue &value : m_gateReviews) {
            if (!value.isObject()) {
                continue;
            }
            const QJsonObject review = value.toObject();
            const QString status = strictString(review, QStringLiteral("status")).toLower();
            if (strictString(review, QStringLiteral("gate")) == priorityGate
                && (status == QStringLiteral("failed") || status == QStringLiteral("unsupported"))) {
                selectedReview = review;
                break;
            }
        }
        if (!selectedReview.isEmpty()) {
            break;
        }
    }

    if (selectedReview.isEmpty()) {
        for (const QJsonValue &value : m_gateReviews) {
            if (!value.isObject()) {
                continue;
            }
            const QJsonObject review = value.toObject();
            const QString gate = strictString(review, QStringLiteral("gate"));
            const QString status = strictString(review, QStringLiteral("status")).toLower();
            if (!gate.isEmpty()
                && (status == QStringLiteral("failed") || status == QStringLiteral("unsupported"))) {
                selectedReview = review;
                break;
            }
        }
    }

    if (selectedReview.isEmpty()) {
        hideFailureCard();
        return;
    }
    showFailureReview(selectedReview, qMax(0, failedGates.size() - 1));
}

void GateStatusBar::showFailureReview(const QJsonObject &review, int otherFailureCount)
{
    const QString gate = strictString(review, QStringLiteral("gate"));
    const QList<FailureIssue> issues = failureIssues(review);
    const QString severity = failureSeverity(review, issues);
    const QString stage = failureStage(gate);
    const QString status = failureStatusText(severity);
    const QString title = failureTitle(gate);
    const QString summary = failureSummary(review, issues);
    m_failureCard->setProperty("severity", QVariant(severity));
    refreshFailureStyle(m_failureCard);

    m_failureStageLabel->setText(
        QString::fromUtf8("\x5B\x25\x31\x20\xC2\xB7\x20\x25\x32\x5D").arg(
            stage, status));
    m_failureTitleLabel->setText(title);
    m_failureSummaryLabel->setText(summary);
    m_failureSummaryLabel->setVisible(!summary.isEmpty());

    const int visibleIssueCount = qMin(issues.size(), kMaximumFailureIssues);
    for (int index = 0; index < m_failureIssueLabels.size(); ++index) {
        QLabel *label = m_failureIssueLabels.at(index);
        if (index < visibleIssueCount) {
            label->setText(failureIssueText(issues.at(index), index));
            label->show();
        } else {
            label->clear();
            label->hide();
        }
    }

    const QString evidence = failureEvidence(review);
    m_failureEvidenceLabel->setText(evidence);
    m_failureEvidenceLabel->setVisible(!evidence.isEmpty());

    QStringList moreLines;
    if (issues.size() > kMaximumFailureIssues) {
        moreLines << QString::fromUtf8("\xE8\xBF\x98\xE6\x9C\x89\x20\x25\x31\x20\xE6\x9D\xA1\xE9\x97\xAE\xE9\xA2\x98\xE6\x9C\xAA\xE5\xB1\x95\xE5\xBC\x80\xE3\x80\x82").arg(
            issues.size() - kMaximumFailureIssues);
    }
    if (otherFailureCount > 0) {
        moreLines << QString::fromUtf8("\xE5\x8F\xA6\xE6\x9C\x89\x20\x25\x31\x20\xE4\xB8\xAA\xE5\xA4\xB1\xE8\xB4\xA5\xE6\xA3\x80\xE6\x9F\xA5\xE9\xA1\xB9\xE3\x80\x82").arg(otherFailureCount);
    }
    m_failureMoreLabel->setText(moreLines.join(QChar('\n')));
    m_failureMoreLabel->setVisible(!moreLines.isEmpty());

    QStringList copyLines;
    copyLines << QString::fromUtf8("APPFlow \xE5\xB7\xA5\xE4\xBD\x9C\xE6\xB5\x81\xE8\xAF\x8A\xE6\x96\xAD")
              << QString::fromUtf8("\xE9\x98\xB6\xE6\xAE\xB5\xEF\xBC\x9A\x25\x31").arg(stage)
              << QString::fromUtf8("\xE7\x8A\xB6\xE6\x80\x81\xEF\xBC\x9A\x25\x31").arg(status)
              << QString::fromUtf8("\xE6\xA0\x87\xE9\xA2\x98\xEF\xBC\x9A\x25\x31").arg(title)
              << QString::fromUtf8("\xE6\x91\x98\xE8\xA6\x81\xEF\xBC\x9A\x25\x31").arg(summary);
    if (visibleIssueCount > 0) {
        copyLines << QString::fromUtf8("\xE9\x97\xAE\xE9\xA2\x98\xEF\xBC\x9A");
        for (int index = 0; index < visibleIssueCount; ++index) {
            copyLines << failureIssueText(issues.at(index), index);
        }
    }
    if (!evidence.isEmpty()) {
        copyLines << evidence;
    }
    if (issues.size() > kMaximumFailureIssues) {
        copyLines << QString::fromUtf8("\xE6\x9C\xAA\xE5\xB1\x95\xE5\xBC\x80\xE9\x97\xAE\xE9\xA2\x98\xEF\xBC\x9A\x25\x31").arg(
            issues.size() - kMaximumFailureIssues);
    }
    if (otherFailureCount > 0) {
        copyLines << QString::fromUtf8("\xE5\x85\xB6\xE4\xBB\x96\xE5\xA4\xB1\xE8\xB4\xA5\xE6\xA3\x80\xE6\x9F\xA5\xE9\xA1\xB9\xEF\xBC\x9A\x25\x31").arg(
            otherFailureCount);
    }
    setFailureCopyText(boundedCopyText(copyLines));
    m_failureCard->show();
}

void GateStatusBar::showRequestUncertainty()
{
    const QString severity = QStringLiteral("uncertain");
    const QString stage = actionStage(m_uncertainAction);
    const QString status = failureStatusText(severity);
    const QString title = QString::fromUtf8("\xE6\x97\xA0\xE6\xB3\x95\xE7\xA1\xAE\xE8\xAE\xA4\xE5\xBD\x93\xE5\x89\x8D\xE8\xAF\xB7\xE6\xB1\x82\xE7\x8A\xB6\xE6\x80\x81");
    const QString summary = QString::fromUtf8("\xE5\xAE\xA2\xE6\x88\xB7\xE7\xAB\xAF\xE6\x9C\xAA\xE6\x94\xB6\xE5\x88\xB0\xE5\x8F\xAF\xE7\xA1\xAE\xE8\xAE\xA4\xE7\x9A\x84\xE5\x90\x8E\xE7\xAB\xAF\xE7\x8A\xB6\xE6\x80\x81\xE3\x80\x82\xE8\xAF\xB7\xE6\xA3\x80\xE6\x9F\xA5\xE6\x9C\x8D\xE5\x8A\xA1\xE7\x8A\xB6\xE6\x80\x81\xE5\x90\x8E\xE9\x87\x8D\xE8\xAF\x95\xEF\xBC\x8C\xE9\x81\xBF\xE5\x85\x8D\xE6\x8A\x8A\xE6\x9C\xAA\xE7\x9F\xA5\xE7\x8A\xB6\xE6\x80\x81\xE8\xAF\xAF\xE8\xAE\xA4\xE4\xB8\xBA\xE6\x88\x90\xE5\x8A\x9F\xE6\x88\x96\xE5\xA4\xB1\xE8\xB4\xA5\xE3\x80\x82");
    m_failureCard->setProperty("severity", QVariant(severity));
    refreshFailureStyle(m_failureCard);
    m_failureStageLabel->setText(
        QString::fromUtf8("\x5B\x25\x31\x20\xC2\xB7\x20\x25\x32\x5D").arg(
            stage, status));
    m_failureTitleLabel->setText(title);
    m_failureSummaryLabel->setText(summary);
    m_failureSummaryLabel->show();
    for (QLabel *label : m_failureIssueLabels) {
        label->clear();
        label->hide();
    }
    m_failureEvidenceLabel->clear();
    m_failureEvidenceLabel->hide();
    m_failureMoreLabel->clear();
    m_failureMoreLabel->hide();
    QStringList copyLines;
    copyLines << QString::fromUtf8("APPFlow \xE5\xB7\xA5\xE4\xBD\x9C\xE6\xB5\x81\xE8\xAF\x8A\xE6\x96\xAD")
              << QString::fromUtf8("\xE9\x98\xB6\xE6\xAE\xB5\xEF\xBC\x9A\x25\x31").arg(stage)
              << QString::fromUtf8("\xE7\x8A\xB6\xE6\x80\x81\xEF\xBC\x9A\x25\x31").arg(status)
              << QString::fromUtf8("\xE6\xA0\x87\xE9\xA2\x98\xEF\xBC\x9A\x25\x31").arg(title)
              << QString::fromUtf8("\xE6\x91\x98\xE8\xA6\x81\xEF\xBC\x9A\x25\x31").arg(summary);
    setFailureCopyText(boundedCopyText(copyLines));
    m_failureCard->show();
}

void GateStatusBar::hideFailureCard()
{
    m_failureCopyText.clear();
    resetFailureCopyFeedback();
    m_failureCopyButton->setEnabled(false);
    m_failureCard->hide();
}

void GateStatusBar::setFailureCopyText(const QString &text)
{
    m_failureCopyText = text.trimmed();
    resetFailureCopyFeedback();
}

void GateStatusBar::copyFailureDiagnostics()
{
    if (m_failureCopyText.isEmpty()) {
        return;
    }

    QClipboard *clipboard = QApplication::clipboard();
    if (clipboard == nullptr) {
        return;
    }
    clipboard->setText(m_failureCopyText, QClipboard::Clipboard);

    const quint64 generation = ++m_failureCopyFeedbackGeneration;
    m_failureCopyButton->setText(QString::fromUtf8("\xE5\xB7\xB2\xE5\xA4\x8D\xE5\x88\xB6"));
    m_failureCopyButton->setEnabled(false);
    QTimer::singleShot(kFailureCopyFeedbackDurationMs, m_failureCopyButton,
        [this, generation]() {
            if (generation != m_failureCopyFeedbackGeneration) {
                return;
            }
            m_failureCopyButton->setText(
                QString::fromUtf8("\xE5\xA4\x8D\xE5\x88\xB6\xE8\xAF\x8A\xE6\x96\xAD"));
            m_failureCopyButton->setEnabled(!m_failureCopyText.isEmpty());
        });
}

void GateStatusBar::resetFailureCopyFeedback()
{
    ++m_failureCopyFeedbackGeneration;
    m_failureCopyButton->setText(
        QString::fromUtf8("\xE5\xA4\x8D\xE5\x88\xB6\xE8\xAF\x8A\xE6\x96\xAD"));
    m_failureCopyButton->setEnabled(!m_failureCopyText.isEmpty());
}
