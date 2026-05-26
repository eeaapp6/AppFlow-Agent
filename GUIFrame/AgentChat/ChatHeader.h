#pragma once

#include <QWidget>

class QLabel;
class QPushButton;

class ChatHeader : public QWidget
{
    Q_OBJECT

public:
    explicit ChatHeader(QWidget *parent = nullptr);
    void setTaskRunning(bool running);

signals:
    void settingsRequested();
    void stopRequested();

private:
    QLabel *m_titleLabel = nullptr;
    QLabel *m_statusLabel = nullptr;
    QPushButton *m_settingsButton = nullptr;
    QPushButton *m_stopButton = nullptr;
};
