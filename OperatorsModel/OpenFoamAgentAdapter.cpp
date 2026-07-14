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
        const QStringList& vtkResultExtensions()
        {
            static const QStringList extensions = { "vtk", "vtp", "vtu" };
            return extensions;
        }

        QFileInfoList vtkResultFiles(const QString& resultDir)
        {
            QDir dir(resultDir);
            if (!dir.exists()) return QFileInfoList();

            QFileInfoList vtkFiles;
            const QFileInfoList files = dir.entryInfoList(QDir::Files, QDir::Name);
            for (const QFileInfo& file : files) {
                if (vtkResultExtensions().contains(file.suffix(), Qt::CaseInsensitive)) {
                    vtkFiles << file;
                }
            }
            return vtkFiles;
        }

        QStringList resultFileSummary(const QString& resultDir)
        {
            QDir dir(resultDir);
            if (!dir.exists()) return QStringList();

            QStringList messages;
            const QFileInfoList vtkFiles = vtkResultFiles(resultDir);
            for (const QString& extension : vtkResultExtensions()) {
                int count = 0;
                for (const QFileInfo& file : vtkFiles) {
                    if (file.suffix().compare(extension, Qt::CaseInsensitive) == 0) {
                        ++count;
                    }
                }
                if (count > 0) {
                    messages << QString("*.%1: %2").arg(extension).arg(count);
                }
            }

            const QStringList filters = { "*.foam", "*.png", "*.jpg", "*.jpeg", "*.csv" };
            for (const QString& filter : filters) {
                const QStringList files = dir.entryList(QStringList() << filter, QDir::Files, QDir::Name);
                if (!files.isEmpty()) {
                    messages << QString("%1: %2").arg(filter).arg(files.size());
                }
            }
            return messages;
        }

        QString firstVtkResultFile(const QString& resultDir)
        {
            const QFileInfoList files = vtkResultFiles(resultDir);
            if (files.isEmpty()) return QString();
            return files.first().absoluteFilePath();
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

        bool buildOpenFoamVTKExportRequest(const QString& caseDir,
            const QJsonObject& appflowHints,
            OpenFoamVTKExportRequest& request,
            QStringList& messages)
        {
            request = OpenFoamVTKExportRequest();
            request.caseDir = caseDir;
            request.resultDir = QDir(caseDir).absoluteFilePath("VTK");

            QString backend = appflowHints.value("foam_to_vtk_backend").toString("native").toLower();
            if (backend == "docker") {
                QString image = appflowHints.value("docker_image").toString();
                QString dockerCaseDir = appflowHints.value("docker_case_dir").toString("/case");
                if (image.isEmpty()) {
                    messages << QObject::tr("foamToVTK docker export skipped. docker_image is empty.");
                    return false;
                }

                request.program = dockerExecutable();
                QString hostCaseDir = QDir::fromNativeSeparators(QFileInfo(caseDir).absoluteFilePath());
                request.arguments << "run" << "--rm"
                    << "-v" << QString("%1:%2").arg(hostCaseDir).arg(dockerCaseDir)
                    << "-w" << dockerCaseDir
                    << image
                    << "foamToVTK" << "-ascii" << "-case" << dockerCaseDir;

                messages << QObject::tr("foamToVTK docker export prepared from: %1").arg(caseDir);
                messages << QObject::tr("foamToVTK backend: docker");
                messages << QObject::tr("Docker executable: %1").arg(request.program);
                messages << QObject::tr("Docker image: %1").arg(image);
                messages << QObject::tr("Docker case dir: %1").arg(dockerCaseDir);
            }
            else {
                request.program = foamToVTKExecutable();
                request.arguments << "-ascii" << "-case" << caseDir;

                messages << QObject::tr("foamToVTK export prepared from: %1").arg(caseDir);
                messages << QObject::tr("foamToVTK backend: native");
                messages << QObject::tr("foamToVTK executable: %1").arg(request.program);
            }
            return request.isValid();
        }

        void scanResultDir(const QString& resultDir, OpenFoamAgentResult& result, QStringList& messages, const QString& prefix)
        {
            result.firstVtk = firstVtkResultFile(resultDir);
            result.hasVtkResult = !result.firstVtk.isEmpty();

            QStringList resultSummary = resultFileSummary(resultDir);
            if (resultSummary.isEmpty()) {
                messages << QObject::tr("%1none of supported types found.").arg(prefix);
            }
            else {
                messages << prefix.left(prefix.size() - 2) + ":";
                for (const QString& line : resultSummary) {
                    messages << QString("  %1").arg(line);
                }
            }

            if (result.hasVtkResult) {
                messages << QObject::tr("VTK result is ready: %1").arg(resultDir);
                messages << QObject::tr("ParaView can open VTK file: %1").arg(result.firstVtk);
            }
        }
    }

    OpenFoamAgentResult OpenFoamAgentAdapter::processResults(const QJsonObject& artifacts,
        const QJsonObject& appflowHints,
        OpenFoamVTKExportRequest& exportRequest,
        QStringList& messages)
    {
        exportRequest = OpenFoamVTKExportRequest();
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
            buildOpenFoamVTKExportRequest(caseDir, appflowHints, exportRequest, messages);
        }
        else if (!result.hasVtkResult && exportVtk && timeDirs.isEmpty()) {
            messages << QObject::tr("foamToVTK export skipped. No OpenFOAM time directories are ready.");
        }

        return result;
    }

    OpenFoamAgentResult OpenFoamAgentAdapter::scanExportedResults(
        const OpenFoamVTKExportRequest& exportRequest,
        QStringList& messages)
    {
        OpenFoamAgentResult result;
        result.caseDir = exportRequest.caseDir;
        result.resultDir = exportRequest.resultDir;
        messages << QObject::tr("VTK result dir after export: %1").arg(result.resultDir);
        scanResultDir(result.resultDir, result, messages, QObject::tr("Result files after export: "));
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
