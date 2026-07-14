#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QWidget>

class QLabel;
class QFrame;
class QPushButton;

class GateStatusBar : public QWidget
{
public:
    explicit GateStatusBar(QWidget *parent = nullptr);

    void clear();
    void setGateReviews(const QJsonArray &reviews);
    void setRequestUncertain(const QString &action);
    void clearRequestUncertainty();

private:
    void resetGateLabels();
    void applyGateReview(const QJsonObject &review);
    void updateSummary();
    void updateFailureCard();
    void showFailureReview(const QJsonObject &review, int otherFailureCount);
    void showRequestUncertainty();
    void hideFailureCard();
    void setFailureCopyText(const QString &text);
    void copyFailureDiagnostics();
    void resetFailureCopyFeedback();

    QLabel *m_summaryLabel = nullptr;
    QList<QLabel *> m_gateLabels;
    QFrame *m_failureCard = nullptr;
    QLabel *m_failureStageLabel = nullptr;
    QPushButton *m_failureCopyButton = nullptr;
    QLabel *m_failureTitleLabel = nullptr;
    QLabel *m_failureSummaryLabel = nullptr;
    QList<QLabel *> m_failureIssueLabels;
    QLabel *m_failureEvidenceLabel = nullptr;
    QLabel *m_failureMoreLabel = nullptr;
    QJsonArray m_gateReviews;
    QString m_uncertainAction;
    QString m_failureCopyText;
    quint64 m_failureCopyFeedbackGeneration = 0;
};
