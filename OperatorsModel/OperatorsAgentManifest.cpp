/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "OperatorsAgentManifest.h"
#include "AgentBridgeMessage.h"
#include "AgentLogPreview.h"
#include "AgentManifestData.h"
#include "AgentSolverSettingsApplier.h"
#include "AgentSolverSettingsPreview.h"
#include "OpenFoamAgentAdapter.h"
#include "OperatorsImportManager.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"
#include "FITK_Kernel/FITKAppFramework/FITKMessage.h"
#include "FITK_Kernel/FITKCore/FITKActionOperator.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QRegularExpression>
#include <QTimer>

namespace
{
    const int VtkExportTimeoutMs = 600000;
    const int VtkExportTerminateGraceMs = 2000;
    const int MaxProcessDiagnosticBytes = 4096;

    QString sanitizedProcessOutputTail(const QByteArray& output)
    {
        QString text = QString::fromLocal8Bit(output.right(MaxProcessDiagnosticBytes)).trimmed();
        static const QRegularExpression sensitiveValue(
            QStringLiteral("(api[_-]?key|authorization|bearer)[^\\r\\n]*"),
            QRegularExpression::CaseInsensitiveOption);
        text.replace(sensitiveValue, QStringLiteral("\\1 [redacted]"));
        return text;
    }

    void appendOutputTail(QByteArray& outputTail, const QByteArray& output)
    {
        outputTail.append(output);
        if (outputTail.size() > MaxProcessDiagnosticBytes) {
            outputTail = outputTail.right(MaxProcessDiagnosticBytes);
        }
    }

    QString normalizedPath(const QString& path)
    {
        const QString trimmed = path.trimmed();
        if (trimmed.isEmpty()) return QString();
        return QDir::cleanPath(QDir::fromNativeSeparators(trimmed));
    }

    bool pathsReferToSameRequest(const QString& left, const QString& right)
    {
        const QString normalizedLeft = normalizedPath(left);
        const QString normalizedRight = normalizedPath(right);
        if (normalizedLeft.isEmpty() || normalizedRight.isEmpty()) return false;
        const bool windowsDrivePath = normalizedLeft.size() > 1
            && normalizedRight.size() > 1
            && normalizedLeft.at(1) == QLatin1Char(':')
            && normalizedRight.at(1) == QLatin1Char(':');
        return normalizedLeft.compare(
            normalizedRight,
            windowsDrivePath ? Qt::CaseInsensitive : Qt::CaseSensitive) == 0;
    }

    QString limitedCompletionMessage(const QString& message)
    {
        const int maximumLength = 500;
        const QString text = message.trimmed();
        if (text.size() <= maximumLength) return text;
        return text.left(maximumLength - 3).trimmed() + QStringLiteral("...");
    }

    void appendProcessDiagnostics(const QByteArray& standardOutputTail,
        const QByteArray& standardErrorTail,
        QStringList& messages)
    {
        const QString standardOutput = sanitizedProcessOutputTail(standardOutputTail);
        const QString standardError = sanitizedProcessOutputTail(standardErrorTail);
        if (!standardOutput.isEmpty()) {
            messages << QObject::tr("foamToVTK output tail:") << standardOutput;
        }
        if (!standardError.isEmpty()) {
            messages << QObject::tr("foamToVTK error output tail:") << standardError;
        }
    }
}

namespace ModelOper
{
    OperatorsAgentManifest::OperatorsAgentManifest()
    {

    }

    OperatorsAgentManifest::~OperatorsAgentManifest()
    {
        clearManifestImportContext();
        releaseVtkExportOnDestruction();
    }

    bool OperatorsAgentManifest::execGUI()
    {
        QWidget* mw = FITKAPP->getGlobalData()->getMainWindow();

        QString wk;
        AppFrame::FITKAppSettings* settings = FITKAPP->getAppSettings();
        if (settings)
            wk = settings->getWorkingDir();

        QString fileName = QFileDialog::getOpenFileName(
            mw,
            QObject::tr("Import Agent Result"),
            wk,
            QStringLiteral("JSON(*.json)"));
        if (fileName.isEmpty()) return false;

        this->setArgs("FileName", fileName);
        return true;
    }

    bool OperatorsAgentManifest::execProfession()
    {
        if (m_manifestImport.active || m_vtkExportProcess != nullptr) {
            this->clearArgs();
            AppFrame::FITKMessageError(tr("APPFlow manifest import is already running."));
            return false;
        }

        QString fileName;
        bool ok = this->argValue<QString>("FileName", fileName);
        this->clearArgs();
        fileName = fileName.trimmed();
        if (!ok || fileName.isEmpty()) return false;

        AgentManifestData manifest;
        QString errorMessage;
        if (!loadAgentManifest(fileName, manifest, errorMessage)) {
            AppFrame::FITKMessageError(errorMessage);
            return false;
        }
        if (!validateAgentManifestImportReadiness(manifest, errorMessage)) {
            AppFrame::FITKMessageError(errorMessage);
            return false;
        }

        QString solverFamily = manifest.solver.value("family").toString();

        QStringList messages;
        messages << tr("Agent manifest loaded: %1").arg(fileName);
        messages << tr("Workflow: %1").arg(manifest.workflow.value("id").toString());
        messages << tr("Status: %1").arg(manifest.workflow.value("status").toString());
        messages << tr("Solver family: %1").arg(solverFamily);
        messages << tr("Solver command: %1").arg(agentValueOrNotProvided(manifest.solver.value("command").toString()));
        messages << tr("Case dir: %1").arg(manifest.artifacts.value("case_dir").toString());
        messages << tr("Mesh dir: %1").arg(agentValueOrNotProvided(manifest.artifacts.value("mesh_dir").toString()));
        messages << tr("Result dir: %1").arg(agentValueOrNotProvided(manifest.artifacts.value("result_dir").toString()));
        messages << tr("Case name: %1").arg(agentValueOrNotProvided(manifest.caseSummary.value("case_name").toString()));
        messages << tr("Case dir exists: %1").arg(QFileInfo(manifest.artifacts.value("case_dir").toString()).isDir() ? "yes" : "no");
        messages << tr("Mesh dir exists: %1").arg(QFileInfo(manifest.artifacts.value("mesh_dir").toString()).isDir() ? "yes" : "no");

        bool isOpenFoam = solverFamily.compare("openfoam", Qt::CaseInsensitive) == 0;
        OpenFoamAgentResult result;
        OpenFoamVTKExportRequest exportRequest;
        if (isOpenFoam) {
            result = OpenFoamAgentAdapter::processResults(
                manifest.artifacts,
                manifest.appflowHints,
                exportRequest,
                messages);
        }
        else {
            messages << tr("APPFlow only supports openfoam Agent manifests.");
            messages << tr("Solver family %1 is not handled by this application.").arg(solverFamily);
        }

        appendAgentSolverSettingsPreviewMessages(manifest.solverSettings, messages);
        applyAgentSolverSettingsToAppFlowModel(manifest.solver, manifest.solverSettings, messages);
        appendAgentLogPreviewMessages(manifest.logs, messages);

        const QString caseDir = manifest.artifacts.value("case_dir").toString().trimmed();
        const bool importMesh = manifest.appflowHints.value("import_mesh").toBool(false);
        const bool vtkRequested = isOpenFoam
            && !result.hasVtkResult
            && (manifest.appflowHints.value("run_foam_to_vtk").toBool(false)
                || manifest.appflowHints.value("export_vtk").toBool(false));
        beginManifestImport(fileName, caseDir);

        bool asynchronousBranchAccepted = false;
        QStringList startupFailures;
        if (importMesh) {
            m_manifestImport.meshRequired = true;
            QString meshFailure;
            if (!isOpenFoam) {
                meshFailure = tr("OpenFOAM mesh import requires solver family openfoam.");
            }
            else if (!QFileInfo(caseDir).isDir()) {
                meshFailure = tr("OpenFOAM mesh import failed because case_dir is no longer an existing directory: %1").arg(caseDir);
            }
            else {
                OperatorsImportManager* oper = FITKOPERREPO->getOperatorT<OperatorsImportManager>(
                    "actionImportOpenFoamMesh");
                if (oper == nullptr) {
                    meshFailure = tr("OpenFOAM mesh import operator not found: actionImportOpenFoamMesh.");
                }
                else {
                    m_meshImportConnection = connect(
                        oper,
                        &OperatorsImportManager::openFoamMeshImportFinished,
                        this,
                        &OperatorsAgentManifest::handleOpenFoamMeshImportFinished,
                        Qt::UniqueConnection);
                    if (!m_meshImportConnection) {
                        meshFailure = tr("OpenFOAM mesh import result connection could not be established.");
                    }
                    else {
                        oper->setArgs("FileName", caseDir);
                        oper->setArgs("SenderName", "actionImportOpenFoamMesh");
                        const bool importAccepted = oper->execProfession();
                        if (importAccepted) {
                            asynchronousBranchAccepted = true;
                            messages << tr("OpenFOAM mesh import requested from: %1").arg(caseDir);
                        }
                        else {
                            QObject::disconnect(m_meshImportConnection);
                            m_meshImportConnection = QMetaObject::Connection();
                            meshFailure = tr("OpenFOAM mesh import operator rejected or could not start the request for: %1").arg(caseDir);
                        }
                    }
                }
            }
            if (!meshFailure.isEmpty()) {
                recordMeshResult(false, meshFailure);
                startupFailures << meshFailure;
            }
        }

        if (vtkRequested) {
            m_manifestImport.vtkRequired = true;
            QString vtkFailure;
            if (!exportRequest.isValid()) {
                vtkFailure = tr("foamToVTK export was requested but no valid export request could be built.");
            }
            else if (startVtkExport(exportRequest, manifest.appflowHints, messages)) {
                asynchronousBranchAccepted = true;
            }
            else {
                vtkFailure = tr("foamToVTK export request could not be submitted.");
            }
            if (!vtkFailure.isEmpty()) {
                recordVtkResult(false, vtkFailure);
                startupFailures << vtkFailure;
            }
        }
        else if (isOpenFoam) {
            OpenFoamAgentAdapter::openParaViewIfRequested(result, manifest.appflowHints, messages);
        }

        outputAgentNormalMessages(messages);

        if (!m_manifestImport.meshRequired && !m_manifestImport.vtkRequired) {
            const QString manifestPathSnapshot = m_manifestImport.manifestPath;
            QTimer::singleShot(0, this, [this, manifestPathSnapshot]() {
                if (!m_manifestImport.active
                    || !pathsReferToSameRequest(manifestPathSnapshot, m_manifestImport.manifestPath)) {
                    return;
                }
                finalizeManifestImportIfReady();
            });
            return true;
        }

        if (asynchronousBranchAccepted) {
            return true;
        }

        const QString error = startupFailures.isEmpty()
            ? tr("APPFlow manifest import could not start any requested asynchronous work.")
            : startupFailures.join(QStringLiteral("; "));
        AppFrame::FITKMessageError(error);
        clearManifestImportContext();
        return false;
    }

    void OperatorsAgentManifest::beginManifestImport(
        const QString& manifestPath,
        const QString& caseDir)
    {
        m_manifestImport = ManifestImportContext();
        m_manifestImport.active = true;
        m_manifestImport.manifestPath = manifestPath;
        m_manifestImport.meshCaseDir = caseDir;
    }

    void OperatorsAgentManifest::recordMeshResult(bool success, const QString& message)
    {
        if (!m_manifestImport.active
            || !m_manifestImport.meshRequired
            || m_manifestImport.meshFinished) {
            return;
        }
        m_manifestImport.meshFinished = true;
        m_manifestImport.meshSuccess = success;
        const QString detail = limitedCompletionMessage(message);
        if (!success && !detail.isEmpty() && !m_manifestImport.failureMessages.contains(detail)) {
            m_manifestImport.failureMessages << detail;
        }
    }

    void OperatorsAgentManifest::recordVtkResult(bool success, const QString& message)
    {
        if (!m_manifestImport.active
            || !m_manifestImport.vtkRequired
            || m_manifestImport.vtkFinished) {
            return;
        }
        m_manifestImport.vtkFinished = true;
        m_manifestImport.vtkSuccess = success;
        const QString detail = limitedCompletionMessage(message);
        if (!success && !detail.isEmpty() && !m_manifestImport.failureMessages.contains(detail)) {
            m_manifestImport.failureMessages << detail;
        }
    }

    void OperatorsAgentManifest::finalizeManifestImportIfReady()
    {
        if (!m_manifestImport.active || m_manifestImport.finalEmitted) return;
        if (m_manifestImport.meshRequired && !m_manifestImport.meshFinished) return;
        if (m_manifestImport.vtkRequired && !m_manifestImport.vtkFinished) return;
        if (m_vtkExportProcess != nullptr) return;

        const bool success = (!m_manifestImport.meshRequired || m_manifestImport.meshSuccess)
            && (!m_manifestImport.vtkRequired || m_manifestImport.vtkSuccess);
        const QString manifestPath = m_manifestImport.manifestPath;
        QString message;
        if (success) {
            message = tr("APPFlow manifest import completed.");
        }
        else if (!m_manifestImport.failureMessages.isEmpty()) {
            message = m_manifestImport.failureMessages.join(QStringLiteral("; "));
        }
        else {
            message = tr("APPFlow manifest import failed.");
        }

        m_manifestImport.finalEmitted = true;
        OperatorsImportManager* resultSource = FITKOPERREPO->getOperatorT<OperatorsImportManager>(
            "actionImportOpenFoamMesh");
        if (resultSource != nullptr) {
            resultSource->publishAgentManifestImportFinished(
                manifestPath,
                success,
                limitedCompletionMessage(message));
        }
        else {
            AppFrame::FITKMessageError(tr("APPFlow manifest import result source is unavailable."));
        }
        clearManifestImportContext();
    }

    void OperatorsAgentManifest::clearManifestImportContext()
    {
        if (m_meshImportConnection) {
            QObject::disconnect(m_meshImportConnection);
            m_meshImportConnection = QMetaObject::Connection();
        }
        m_manifestImport = ManifestImportContext();
    }

    void OperatorsAgentManifest::handleOpenFoamMeshImportFinished(
        const QString& caseDir,
        bool success,
        const QString& message)
    {
        if (!m_manifestImport.active
            || !m_manifestImport.meshRequired
            || m_manifestImport.meshFinished
            || !pathsReferToSameRequest(caseDir, m_manifestImport.meshCaseDir)) {
            return;
        }
        recordMeshResult(success, message);
        finalizeManifestImportIfReady();
    }

    bool OperatorsAgentManifest::startVtkExport(
        const OpenFoamVTKExportRequest& request,
        const QJsonObject& appflowHints,
        QStringList& messages)
    {
        if (m_vtkExportProcess != nullptr || !request.isValid()) return false;

        m_vtkExportRequest = request;
        m_vtkExportAppflowHints = appflowHints;
        m_vtkExportStandardOutputTail.clear();
        m_vtkExportStandardErrorTail.clear();
        m_vtkExportFinalized = false;
        m_vtkExportProcess = new QProcess(this);
        m_vtkExportTimeout = new QTimer(this);
        m_vtkExportTimeout->setSingleShot(true);

        connect(m_vtkExportProcess, &QProcess::started, this, [this]() {
            if (m_vtkExportProcess == nullptr || m_vtkExportFinalized) return;
            m_vtkExportTimeout->start(VtkExportTimeoutMs);
            outputAgentNormalMessages(QStringList()
                << tr("foamToVTK process started. Waiting for export completion."));
        });
        connect(m_vtkExportProcess,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this,
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                handleVtkExportFinished(exitCode, exitStatus);
            });
        connect(m_vtkExportProcess, &QProcess::errorOccurred, this,
            [this](QProcess::ProcessError error) { handleVtkExportError(error); });
        connect(m_vtkExportProcess, &QProcess::readyReadStandardOutput, this,
            [this]() { captureVtkExportOutput(); });
        connect(m_vtkExportProcess, &QProcess::readyReadStandardError, this,
            [this]() { captureVtkExportOutput(); });
        connect(m_vtkExportTimeout, &QTimer::timeout, this,
            [this]() { handleVtkExportTimeout(); });

        m_vtkExportProcess->start(request.program, request.arguments);
        messages << tr("foamToVTK export request submitted. Completion will be reported asynchronously.");
        return true;
    }

    void OperatorsAgentManifest::handleVtkExportFinished(
        int exitCode,
        QProcess::ExitStatus exitStatus)
    {
        if (m_vtkExportProcess == nullptr) return;
        if (m_vtkExportFinalized) {
            cleanupVtkExport();
            finalizeManifestImportIfReady();
            return;
        }

        m_vtkExportFinalized = true;
        if (m_vtkExportTimeout != nullptr) m_vtkExportTimeout->stop();

        QStringList messages;
        captureVtkExportOutput();
        appendProcessDiagnostics(
            m_vtkExportStandardOutputTail,
            m_vtkExportStandardErrorTail,
            messages);
        if (exitStatus != QProcess::NormalExit || exitCode != 0) {
            const QString failure = tr("foamToVTK export failed. Exit code: %1").arg(exitCode);
            outputAgentNormalMessages(messages);
            AppFrame::FITKMessageError(failure);
            recordVtkResult(false, failure);
            cleanupVtkExport();
            finalizeManifestImportIfReady();
            return;
        }

        OpenFoamAgentResult result = OpenFoamAgentAdapter::scanExportedResults(
            m_vtkExportRequest,
            messages);
        if (!result.hasVtkResult || result.firstVtk.isEmpty()) {
            const QString failure = tr(
                "foamToVTK exited successfully but no supported VTK result file was found.");
            outputAgentNormalMessages(messages);
            AppFrame::FITKMessageError(failure);
            recordVtkResult(false, failure);
            cleanupVtkExport();
            finalizeManifestImportIfReady();
            return;
        }

        messages << tr("foamToVTK export completed with a supported VTK result.");
        OpenFoamAgentAdapter::openParaViewIfRequested(
            result,
            m_vtkExportAppflowHints,
            messages);
        outputAgentNormalMessages(messages);
        recordVtkResult(true, tr("foamToVTK export completed with a supported VTK result."));
        cleanupVtkExport();
        finalizeManifestImportIfReady();
    }

    void OperatorsAgentManifest::handleVtkExportError(QProcess::ProcessError error)
    {
        Q_UNUSED(error);
        if (m_vtkExportProcess == nullptr || m_vtkExportFinalized) return;

        m_vtkExportFinalized = true;
        if (m_vtkExportTimeout != nullptr) m_vtkExportTimeout->stop();

        QStringList messages;
        captureVtkExportOutput();
        appendProcessDiagnostics(
            m_vtkExportStandardOutputTail,
            m_vtkExportStandardErrorTail,
            messages);
        outputAgentNormalMessages(messages);
        const QString failure = tr("foamToVTK process error: %1").arg(
            m_vtkExportProcess->errorString());
        AppFrame::FITKMessageError(failure);
        recordVtkResult(false, failure);

        if (m_vtkExportProcess->state() == QProcess::NotRunning) {
            cleanupVtkExport();
            finalizeManifestImportIfReady();
        }
        else {
            requestVtkExportStop();
        }
    }

    void OperatorsAgentManifest::handleVtkExportTimeout()
    {
        if (m_vtkExportProcess == nullptr || m_vtkExportFinalized) return;

        m_vtkExportFinalized = true;
        QStringList messages;
        captureVtkExportOutput();
        appendProcessDiagnostics(
            m_vtkExportStandardOutputTail,
            m_vtkExportStandardErrorTail,
            messages);
        outputAgentNormalMessages(messages);
        const QString failure = tr("foamToVTK export timed out after 600000 ms.");
        AppFrame::FITKMessageError(failure);
        recordVtkResult(false, failure);
        requestVtkExportStop();
    }

    void OperatorsAgentManifest::captureVtkExportOutput()
    {
        if (m_vtkExportProcess == nullptr) return;
        appendOutputTail(
            m_vtkExportStandardOutputTail,
            m_vtkExportProcess->readAllStandardOutput());
        appendOutputTail(
            m_vtkExportStandardErrorTail,
            m_vtkExportProcess->readAllStandardError());
    }

    void OperatorsAgentManifest::requestVtkExportStop()
    {
        QProcess* process = m_vtkExportProcess;
        if (process == nullptr || process->state() == QProcess::NotRunning) {
            cleanupVtkExport();
            finalizeManifestImportIfReady();
            return;
        }

        process->terminate();
        QTimer::singleShot(VtkExportTerminateGraceMs, process, [process]() {
            if (process->state() != QProcess::NotRunning) process->kill();
        });
    }

    void OperatorsAgentManifest::cleanupVtkExport()
    {
        if (m_vtkExportTimeout != nullptr) {
            m_vtkExportTimeout->stop();
            m_vtkExportTimeout->deleteLater();
            m_vtkExportTimeout = nullptr;
        }
        if (m_vtkExportProcess != nullptr) {
            disconnect(m_vtkExportProcess, nullptr, this, nullptr);
            m_vtkExportProcess->deleteLater();
            m_vtkExportProcess = nullptr;
        }
        m_vtkExportRequest = OpenFoamVTKExportRequest();
        m_vtkExportAppflowHints = QJsonObject();
        m_vtkExportStandardOutputTail.clear();
        m_vtkExportStandardErrorTail.clear();
        m_vtkExportFinalized = false;
    }

    void OperatorsAgentManifest::releaseVtkExportOnDestruction()
    {
        if (m_vtkExportTimeout != nullptr) {
            m_vtkExportTimeout->stop();
            m_vtkExportTimeout = nullptr;
        }
        if (m_vtkExportProcess == nullptr) return;

        QProcess* process = m_vtkExportProcess;
        disconnect(process, nullptr, this, nullptr);
        m_vtkExportProcess = nullptr;
        if (process->state() == QProcess::NotRunning) return;

        process->setParent(QCoreApplication::instance());
        connect(process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            process,
            &QObject::deleteLater);
        process->terminate();
        QTimer::singleShot(VtkExportTerminateGraceMs, process, [process]() {
            if (process->state() != QProcess::NotRunning) process->kill();
        });
    }
}
