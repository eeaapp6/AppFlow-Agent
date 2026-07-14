/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#ifndef _OperatorsAgentManifest_H
#define _OperatorsAgentManifest_H

#include "OperManagerBase.h"
#include "OpenFoamAgentAdapter.h"

#include <QByteArray>
#include <QJsonObject>
#include <QMetaObject>
#include <QProcess>
#include <QStringList>

class QTimer;

namespace ModelOper
{
    class OperatorsAgentManifest : public Core::FITKActionOperator
    {
    public:
        OperatorsAgentManifest();
        ~OperatorsAgentManifest();

        virtual bool execGUI();

        virtual bool execProfession();

    private:
        struct ManifestImportContext
        {
            bool active{ false };
            QString manifestPath{};
            QString meshCaseDir{};
            bool meshRequired{ false };
            bool meshFinished{ false };
            bool meshSuccess{ false };
            bool vtkRequired{ false };
            bool vtkFinished{ false };
            bool vtkSuccess{ false };
            bool finalEmitted{ false };
            QStringList failureMessages{};
        };

        bool startVtkExport(const OpenFoamVTKExportRequest& request,
            const QJsonObject& appflowHints,
            QStringList& messages);
        void beginManifestImport(const QString &manifestPath, const QString &caseDir);
        void recordMeshResult(bool success, const QString &message);
        void recordVtkResult(bool success, const QString &message);
        void finalizeManifestImportIfReady();
        void clearManifestImportContext();
        void handleOpenFoamMeshImportFinished(const QString &caseDir,
                                              bool success,
                                              const QString &message);
        void handleVtkExportFinished(int exitCode, QProcess::ExitStatus exitStatus);
        void handleVtkExportError(QProcess::ProcessError error);
        void handleVtkExportTimeout();
        void captureVtkExportOutput();
        void requestVtkExportStop();
        void cleanupVtkExport();
        void releaseVtkExportOnDestruction();

        QProcess* m_vtkExportProcess{ nullptr };
        QTimer* m_vtkExportTimeout{ nullptr };
        OpenFoamVTKExportRequest m_vtkExportRequest{};
        QJsonObject m_vtkExportAppflowHints{};
        QByteArray m_vtkExportStandardOutputTail{};
        QByteArray m_vtkExportStandardErrorTail{};
        bool m_vtkExportFinalized{ false };
        ManifestImportContext m_manifestImport{};
        QMetaObject::Connection m_meshImportConnection{};
    };

    // 按钮注册相关操作
    Register2FITKOperatorRepo(actionLoadAgentManifest, OperatorsAgentManifest);
}

#endif
