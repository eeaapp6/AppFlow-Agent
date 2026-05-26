#pragma once

#include <QString>
#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;

class WorkflowWorkspaceBar : public QWidget
{
    Q_OBJECT

public:
    explicit WorkflowWorkspaceBar(QWidget *parent = nullptr);

    QString outputDirectory() const;
    QString backendOutputDirectory() const;
    void setOutputDirectory(const QString &path);
    void setBackendOutputDirectory(const QString &path);
    void setTaskRunning(bool running);

signals:
    void outputDirectoryChanged(const QString &path);
    void backendOutputDirectoryChanged(const QString &path);
    void browseOutputDirectoryRequested();

private:
    QLabel *m_statusLabel = nullptr;
    QLineEdit *m_outputDirectoryEdit = nullptr;
    QLineEdit *m_backendOutputDirectoryEdit = nullptr;
    QPushButton *m_browseButton = nullptr;
};
