#pragma once

#include <QWidget>

class QPushButton;

class WorkflowActionBar : public QWidget
{
    Q_OBJECT

public:
    explicit WorkflowActionBar(QWidget *parent = nullptr);

    void setTaskRunning(bool running);
    void setManifestImportRunning(bool running);
    void setGenerateCaseEnabled(bool enabled);
    void setValidateCaseEnabled(bool enabled);
    void setRunCaseEnabled(bool enabled);
    void setImportToAppFlowEnabled(bool enabled);

signals:
    void openFolderRequested();
    void generateCaseRequested();
    void validateCaseRequested();
    void runCaseRequested();
    void importToAppFlowRequested();

private:
    void refreshButtonStates();

    QPushButton *m_openFolderButton = nullptr;
    QPushButton *m_generateButton = nullptr;
    QPushButton *m_validateButton = nullptr;
    QPushButton *m_runButton = nullptr;
    QPushButton *m_importToAppFlowButton = nullptr;
    bool m_taskRunning = false;
    bool m_manifestImportRunning = false;
    bool m_canGenerateCase = false;
    bool m_canValidateCase = false;
    bool m_canRunCase = false;
    bool m_canImportToAppFlow = false;
};
