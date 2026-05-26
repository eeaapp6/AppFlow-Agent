#pragma once

#include <QDialog>
#include <QString>

class QComboBox;
class QDialogButtonBox;
class QLineEdit;

class ApiSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ApiSettingsDialog(QWidget *parent = nullptr);

private:
    void loadProviders();
    void loadProviderSettings(const QString &providerId);
    void saveCurrentSettings();
    QString currentProviderId() const;

    QComboBox *m_providerCombo = nullptr;
    QLineEdit *m_apiKeyEdit = nullptr;
    QLineEdit *m_modelEdit = nullptr;
    QLineEdit *m_baseUrlEdit = nullptr;
    QDialogButtonBox *m_buttonBox = nullptr;
};
