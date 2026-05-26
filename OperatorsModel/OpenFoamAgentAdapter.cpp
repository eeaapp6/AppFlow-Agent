/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "OpenFoamAgentAdapter.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QObject>
#include <QProcess>
#include <QStandardPaths>

namespace ModelOper
{
    namespace
    {
        QStringList resultFileSummary(const QString& resultDir)
        {
            QDir dir(resultDir);
            if (!dir.exists()) return QStringList();

            QStringList messages;
            const QStringList filters = { "*.vtk", "*.vtp", "*.vtu", "*.foam", "*.png", "*.jpg", "*.jpeg", "*.csv" };
            for (const QString& filter : filters) {
                const QStringList files = dir.entryList(QStringList() << filter, QDir::Files, QDir::Name);
                if (!files.isEmpty()) {
                    messages << QString("%1: %2").arg(filter).arg(files.size());
                }
            }
            return messages;
        }

        QString firstResultFile(const QString& resultDir, const QStringList& filters)
        {
            QDir dir(resultDir);
            if (!dir.exists()) return QString();

            const QStringList files = dir.entryList(filters, QDir::Files, QDir::Name);
            if (files.isEmpty()) return QString();
            return dir.absoluteFilePath(files.first());
        }

        QStringList openFoamTimeDirs(const QString& caseDir)
        {
            QDir dir(caseDir);
            if (!dir.exists()) return QStringList();

            QStringList timeDirs;
            const QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
            for (const QFileInfo& entry : entries) {
                bool ok = false;
                double value = entry.fileName().toDouble(&ok);
                if (ok && value > 0.0) {
                    timeDirs << entry.fileName();
                }
            }
            return timeDirs;
        }

        QString paraViewExecutable()
        {
            QString exe = QStandardPaths::findExecutable("paraview.exe");
            if (!exe.isEmpty()) return exe;

            exe = QStandardPaths::findExecutable("paraview");
            if (!exe.isEmpty()) return exe;

            return "paraview.exe";
        }

        QString normalizeParaViewOpenMode(const QString& mode)
        {
            QString normalized = mode.trimmed().toLower();
            if (normalized == "openfoam_case") return normalized;
            if (normalized == "first_vtk") return normalized;
            return "first_vtk";
        }

        QString createOpenFoamCaseFile(const QString& caseDir, QStringList& messages)
        {
            if (!QFileInfo(caseDir).isDir()) {
                messages << QObject::tr("ParaView OpenFOAM case mode skipped. Case dir is not ready: %1").arg(caseDir);
                return QString();
            }

            QString foamFile = QDir(caseDir).absoluteFilePath("case.foam");
            QFile file(foamFile);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                messages << QObject::tr("ParaView OpenFOAM case file create failed: %1").arg(foamFile);
                messages << QObject::tr("ParaView OpenFOAM case file error: %1").arg(file.errorString());
                return QString();
            }
            file.close();
            return foamFile;
        }

        QString foamToVTKExecutable()
        {
            QString exe = QStandardPaths::findExecutable("foamToVTK.exe");
            if (!exe.isEmpty()) return exe;

            exe = QStandardPaths::findExecutable("foamToVTK");
            if (!exe.isEmpty()) return exe;

            return "foamToVTK";
        }

        QString dockerExecutable()
        {
            QString exe = QStandardPaths::findExecutable("docker.exe");
            if (!exe.isEmpty()) return exe;

            exe = QStandardPaths::findExecutable("docker");
            if (!exe.isEmpty()) return exe;

            return "docker";
        }

        QString processOutputPreview(const QString& text, int maxChars)
        {
            if (text.size() <= maxChars) return text;
            return text.left(maxChars) + "\n...";
        }

        bool exportOpenFoamVTK(const QString& caseDir, const QJsonObject& appflowHints, QStringList& messages)
        {
            QString backend = appflowHints.value("foam_to_vtk_backend").toString("native").toLower();
            QString exe;
            QStringList args;
            if (backend == "docker") {
                QString image = appflowHints.value("docker_image").toString();
                QString dockerCaseDir = appflowHints.value("docker_case_dir").toString("/case");
                if (image.isEmpty()) {
                    messages << QObject::tr("foamToVTK docker export skipped. docker_image is empty.");
                    return false;
                }

                exe = dockerExecutable();
                QString hostCaseDir = QDir::fromNativeSeparators(QFileInfo(caseDir).absoluteFilePath());
                args << "run" << "--rm"
                    << "-v" << QString("%1:%2").arg(hostCaseDir).arg(dockerCaseDir)
                    << "-w" << dockerCaseDir
                    << image
                    << "foamToVTK" << "-ascii" << "-case" << dockerCaseDir;

                messages << QObject::tr("foamToVTK docker export requested from: %1").arg(caseDir);
                messages << QObject::tr("foamToVTK backend: docker");
                messages << QObject::tr("Docker executable: %1").arg(exe);
                messages << QObject::tr("Docker image: %1").arg(image);
                messages << QObject::tr("Docker case dir: %1").arg(dockerCaseDir);
            }
            else {
                exe = foamToVTKExecutable();
                args << "-ascii" << "-case" << caseDir;

                messages << QObject::tr("foamToVTK export requested from: %1").arg(caseDir);
                messages << QObject::tr("foamToVTK backend: native");
                messages << QObject::tr("foamToVTK executable: %1").arg(exe);
            }

            QProcess process;
            process.start(exe, args);
            if (!process.waitForStarted(5000)) {
                messages << QObject::tr("foamToVTK export failed. Check whether the selected backend is available in PATH.");
                messages << QObject::tr("foamToVTK error: %1").arg(process.errorString());
                return false;
            }

            if (!process.waitForFinished(600000)) {
                process.kill();
                process.waitForFinished(3000);
                messages << QObject::tr("foamToVTK export timeout.");
                return false;
            }

            QString standardOutput = QString::fromLocal8Bit(process.readAllStandardOutput());
            QString standardError = QString::fromLocal8Bit(process.readAllStandardError());
            if (!standardOutput.isEmpty()) {
                messages << QObject::tr("foamToVTK output:");
                messages << processOutputPreview(standardOutput, 4000);
            }
            if (!standardError.isEmpty()) {
                messages << QObject::tr("foamToVTK error output:");
                messages << processOutputPreview(standardError, 4000);
            }

            if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
                messages << QObject::tr("foamToVTK export failed. Exit code: %1").arg(process.exitCode());
                return false;
            }

            messages << QObject::tr("foamToVTK export finished.");
            return true;
        }

        void scanResultDir(const QString& resultDir, OpenFoamAgentResult& result, QStringList& messages, const QString& prefix)
        {
            QStringList resultSummary = resultFileSummary(resultDir);
            if (resultSummary.isEmpty()) {
                messages << QObject::tr("%1none of supported types found.").arg(prefix);
            }
            else {
                messages << prefix.left(prefix.size() - 2) + ":";
                for (const QString& line : resultSummary) {
                    messages << QString("  %1").arg(line);
                    if (line.startsWith("*.vtk:")) {
                        result.hasVtkResult = true;
                    }
                }
            }

            if (result.hasVtkResult) {
                messages << QObject::tr("VTK result is ready: %1").arg(resultDir);
                result.firstVtk = firstResultFile(resultDir, QStringList() << "*.vtk");
                if (!result.firstVtk.isEmpty()) {
                    messages << QObject::tr("ParaView can open VTK file: %1").arg(result.firstVtk);
                }
            }
        }
    }

    OpenFoamAgentResult OpenFoamAgentAdapter::processResults(const QJsonObject& artifacts,
        const QJsonObject& appflowHints,
        QStringList& messages)
    {
        OpenFoamAgentResult result;
        result.caseDir = artifacts.value("case_dir").toString();
        result.resultDir = artifacts.value("result_dir").toString();

        bool resultDirExists = QFileInfo(result.resultDir).isDir();
        messages << QObject::tr("Result dir exists: %1").arg(resultDirExists ? "yes" : "no");
        if (resultDirExists) {
            scanResultDir(result.resultDir, result, messages, QObject::tr("Result files: "));
        }

        QString caseDir = result.caseDir;
        QStringList timeDirs;
        if (!result.hasVtkResult) {
            timeDirs = openFoamTimeDirs(caseDir);
            if (!timeDirs.isEmpty()) {
                messages << QObject::tr("OpenFOAM time directories detected: %1").arg(timeDirs.join(", "));
                messages << QObject::tr("VTK export not detected. foamToVTK may be required.");
            }
            else {
                messages << QObject::tr("No OpenFOAM result time directories detected.");
            }
        }

        bool exportVtk = appflowHints.value("run_foam_to_vtk").toBool(false)
            || appflowHints.value("export_vtk").toBool(false);
        if (!result.hasVtkResult && exportVtk && !timeDirs.isEmpty() && QFileInfo(caseDir).isDir()) {
            if (exportOpenFoamVTK(caseDir, appflowHints, messages)) {
                result.resultDir = QDir(caseDir).absoluteFilePath("VTK");
                messages << QObject::tr("VTK result dir after export: %1").arg(result.resultDir);
                result.hasVtkResult = false;
                result.firstVtk.clear();
                scanResultDir(result.resultDir, result, messages, QObject::tr("Result files after export: "));
            }
        }
        else if (!result.hasVtkResult && exportVtk && timeDirs.isEmpty()) {
            messages << QObject::tr("foamToVTK export skipped. No OpenFOAM time directories are ready.");
        }

        return result;
    }

    void OpenFoamAgentAdapter::openParaViewIfRequested(const OpenFoamAgentResult& result,
        const QJsonObject& appflowHints,
        QStringList& messages)
    {
        bool openParaView = appflowHints.value("open_paraview").toBool(false);
        if (!openParaView) return;

        QString mode = normalizeParaViewOpenMode(appflowHints.value("paraview_open_mode").toString("first_vtk"));
        messages << QObject::tr("ParaView open mode: %1").arg(mode);

        QString paraViewExe = paraViewExecutable();
        QStringList args;
        QString openTarget;
        if (mode == "openfoam_case") {
            QString foamFile = createOpenFoamCaseFile(result.caseDir, messages);
            if (foamFile.isEmpty()) {
                messages << QObject::tr("ParaView open skipped. OpenFOAM case file is not ready.");
                return;
            }
            messages << QObject::tr("ParaView OpenFOAM case file: %1").arg(foamFile);
            args << "--case" << foamFile;
            openTarget = foamFile;
        }
        else if (!result.firstVtk.isEmpty()) {
            args << result.firstVtk;
            openTarget = result.firstVtk;
        }
        else {
            messages << QObject::tr("ParaView open skipped. No VTK file is ready.");
            return;
        }

        if (QProcess::startDetached(paraViewExe, args)) {
            if (mode == "openfoam_case")
                messages << QObject::tr("ParaView open requested for OpenFOAM case: %1").arg(openTarget);
            else
                messages << QObject::tr("ParaView open requested for: %1").arg(openTarget);
            messages << QObject::tr("ParaView executable: %1").arg(paraViewExe);
        }
        else {
            messages << QObject::tr("ParaView open failed. Check whether paraview.exe is available in PATH.");
            messages << QObject::tr("Tried ParaView executable: %1").arg(paraViewExe);
        }
    }
}
