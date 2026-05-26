#pragma once

#include <QJsonArray>
#include <QWidget>

class QLabel;

class RepairHistoryBar : public QWidget
{
public:
    explicit RepairHistoryBar(QWidget *parent = nullptr);

    void clear();
    void setRepairHistory(const QJsonArray &history);

private:
    QLabel *m_label = nullptr;
};
