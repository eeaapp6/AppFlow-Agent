#pragma once

#include <QString>
#include <QWidget>

class QLineEdit;
class QPushButton;

class Composer : public QWidget
{
    Q_OBJECT

public:
    explicit Composer(QWidget *parent = nullptr);

signals:
    void messageSubmitted(const QString &text);

private:
    void submitCurrentText();

private:
    QLineEdit *m_input = nullptr;
    QPushButton *m_sendButton = nullptr;
};
