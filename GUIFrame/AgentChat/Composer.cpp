#include "Composer.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>

Composer::Composer(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("composer");

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 12, 20, 16);
    layout->setSpacing(10);

    m_input = new QLineEdit(this);
    m_input->setObjectName("composerInput");
    m_input->setPlaceholderText(QString::fromUtf8("\xE8\xBE\x93\xE5\x85\xA5\xE8\xA6\x81\xE7\x94\x9F\xE6\x88\x90\xE7\x9A\x84\xE7\xAE\x97\xE4\xBE\x8B\xE6\x8F\x8F\xE8\xBF\xB0\xEF\xBC\x8C\xE6\x8C\x89\x20\x45\x6E\x74\x65\x72\x20\xE5\x8F\x91\xE9\x80\x81\x2E\x2E\x2E"));

    m_sendButton = new QPushButton(QString::fromUtf8("\xE5\x8F\x91\xE9\x80\x81"), this);
    m_sendButton->setObjectName("composerSendButton");

    layout->addWidget(m_input, 1);
    layout->addWidget(m_sendButton);

    connect(m_sendButton, &QPushButton::clicked, this, &Composer::submitCurrentText);
    connect(m_input, &QLineEdit::returnPressed, this, &Composer::submitCurrentText);
}

void Composer::submitCurrentText()
{
    const QString text = m_input->text().trimmed();
    if (text.isEmpty()) {
        return;
    }

    m_input->clear();
    emit messageSubmitted(text);
}
