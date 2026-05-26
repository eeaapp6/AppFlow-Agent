#include "ApiSettingsDialog.h"

#include "AppSettings.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

ApiSettingsDialog::ApiSettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QString::fromUtf8("\x41\x50\x49\x20\xE8\xAE\xBE\xE7\xBD\xAE"));
    setModal(true);
    resize(460, 240);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(18, 16, 18, 16);
    rootLayout->setSpacing(12);

    auto *titleLabel = new QLabel(QString::fromUtf8("\xE6\xA8\xA1\xE5\x9E\x8B\xE6\x9C\x8D\xE5\x8A\xA1\xE9\x85\x8D\xE7\xBD\xAE"), this);
    titleLabel->setObjectName("apiSettingsTitle");

    auto *formLayout = new QFormLayout();
    formLayout->setLabelAlignment(Qt::AlignRight);
    formLayout->setFormAlignment(Qt::AlignTop);
    formLayout->setHorizontalSpacing(10);
    formLayout->setVerticalSpacing(10);

    m_providerCombo = new QComboBox(this);
    m_apiKeyEdit = new QLineEdit(this);
    m_modelEdit = new QLineEdit(this);
    m_baseUrlEdit = new QLineEdit(this);

    m_apiKeyEdit->setEchoMode(QLineEdit::Password);
    m_apiKeyEdit->setPlaceholderText(QString::fromUtf8("\xE8\xBE\x93\xE5\x85\xA5\x20\x41\x50\x49\x20\x4B\x65\x79"));
    m_modelEdit->setPlaceholderText(QString::fromUtf8("\xE8\xBE\x93\xE5\x85\xA5\xE6\xA8\xA1\xE5\x9E\x8B\xE5\x90\x8D\xE7\xA7\xB0"));
    m_baseUrlEdit->setPlaceholderText(QString::fromUtf8("\xE8\xBE\x93\xE5\x85\xA5\x20\x41\x50\x49\x20\x42\x61\x73\x65\x20\x55\x52\x4C"));

    formLayout->addRow(QString::fromUtf8("\xE6\x9C\x8D\xE5\x8A\xA1\xE5\x95\x86"), m_providerCombo);
    formLayout->addRow(QString::fromUtf8("\x41\x50\x49\x20\xE5\xAF\x86\xE9\x92\xA5"), m_apiKeyEdit);
    formLayout->addRow(QString::fromUtf8("\xE6\xA8\xA1\xE5\x9E\x8B"), m_modelEdit);
    formLayout->addRow(QString::fromUtf8("\xE6\x8E\xA5\xE5\x8F\xA3\xE5\x9C\xB0\xE5\x9D\x80"), m_baseUrlEdit);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::Save, this);
    m_buttonBox->button(QDialogButtonBox::Save)->setText(QString::fromUtf8("\xE4\xBF\x9D\xE5\xAD\x98"));
    m_buttonBox->button(QDialogButtonBox::Cancel)->setText(QString::fromUtf8("\xE5\x8F\x96\xE6\xB6\x88"));

    rootLayout->addWidget(titleLabel);
    rootLayout->addLayout(formLayout);
    rootLayout->addWidget(m_buttonBox);

    loadProviders();

    connect(m_providerCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this](int) {
        loadProviderSettings(currentProviderId());
    });

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        saveCurrentSettings();
        accept();
    });
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &ApiSettingsDialog::reject);
}

void ApiSettingsDialog::loadProviders()
{
    AppSettings settings;
    const QString activeProvider = settings.activeProvider();

    m_providerCombo->clear();
    for (const QString &providerId : settings.supportedProviders()) {
        m_providerCombo->addItem(settings.displayNameForProvider(providerId), providerId);
    }

    const int activeIndex = m_providerCombo->findData(activeProvider);
    m_providerCombo->setCurrentIndex(activeIndex >= 0 ? activeIndex : 0);
    loadProviderSettings(currentProviderId());
}

void ApiSettingsDialog::loadProviderSettings(const QString &providerId)
{
    AppSettings settings;
    m_apiKeyEdit->setText(settings.providerApiKey(providerId));
    m_modelEdit->setText(settings.providerModel(providerId));
    m_baseUrlEdit->setText(settings.providerBaseUrl(providerId));
}

void ApiSettingsDialog::saveCurrentSettings()
{
    AppSettings settings;
    const QString providerId = currentProviderId();

    settings.setActiveProvider(providerId);
    settings.setProviderApiKey(providerId, m_apiKeyEdit->text());
    settings.setProviderModel(providerId, m_modelEdit->text());
    settings.setProviderBaseUrl(providerId, m_baseUrlEdit->text());
}

QString ApiSettingsDialog::currentProviderId() const
{
    return m_providerCombo->currentData().toString();
}
