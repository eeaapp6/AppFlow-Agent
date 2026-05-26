#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QList>
#include <QWidget>

class QLabel;

class GateStatusBar : public QWidget
{
public:
    explicit GateStatusBar(QWidget *parent = nullptr);

    void clear();
    void setGateReviews(const QJsonArray &reviews);

private:
    void applyGateReview(const QJsonObject &review);
    void updateSummary();

    QLabel *m_summaryLabel = nullptr;
    QList<QLabel *> m_gateLabels;
};
