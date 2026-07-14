/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "AgentManifestData.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QObject>

namespace
{
    QString firstImportBlockerMessage(const QJsonValue& blockersValue)
    {
        if (!blockersValue.isArray()) {
            return {};
        }

        for (const QJsonValue& value : blockersValue.toArray()) {
            if (!value.isObject()) {
                continue;
            }

            const QJsonValue messageValue = value.toObject().value("message");
            if (!messageValue.isString()) {
                continue;
            }

            const QString message = messageValue.toString().trimmed();
            if (!message.isEmpty()) {
                return message;
            }
        }

        return {};
    }
}

namespace ModelOper
{
    bool loadAgentManifest(const QString& fileName, AgentManifestData& data, QString& errorMessage)
    {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            errorMessage = QObject::tr("Failed opening Agent manifest %1").arg(fileName);
            return false;
        }

        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
        file.close();
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            errorMessage = QObject::tr("Invalid Agent manifest: %1").arg(parseError.errorString());
            return false;
        }

        QJsonObject root = doc.object();
        data.version = root.value("version").toInt(-1);
        data.workflow = root.value("workflow").toObject();
        data.solver = root.value("solver").toObject();
        data.artifacts = root.value("artifacts").toObject();
        data.caseSummary = root.value("case_summary").toObject();
        data.appflowHints = root.value("appflow_hints").toObject();
        data.solverSettings = root.value("solver_settings").toObject();
        data.logs = data.artifacts.value("logs").toArray();

        if (data.version != 1 || data.workflow.isEmpty() || data.solver.isEmpty() || data.artifacts.isEmpty()) {
            errorMessage = QObject::tr("Agent manifest missing required v1 fields.");
            return false;
        }

        return true;
    }

    bool validateAgentManifestImportReadiness(const AgentManifestData& data, QString& errorMessage)
    {
        if (data.version != 1) {
            errorMessage = QObject::tr("Agent manifest version must be 1.");
            return false;
        }

        const QJsonValue importReadyValue = data.appflowHints.value("import_ready");
        const QJsonValue importBlockersValue = data.appflowHints.value("import_blockers");
        if (!importReadyValue.isBool()) {
            errorMessage = QObject::tr("Agent manifest import_ready must be a JSON boolean.");
            return false;
        }
        if (!importReadyValue.toBool()) {
            const QString blockerMessage = firstImportBlockerMessage(importBlockersValue);
            errorMessage = blockerMessage.isEmpty()
                ? QObject::tr("Agent manifest is not ready for APPFlow import.")
                : blockerMessage;
            return false;
        }
        if (!importBlockersValue.isArray()) {
            errorMessage = QObject::tr("Agent manifest import_blockers must be an array.");
            return false;
        }
        if (!importBlockersValue.toArray().isEmpty()) {
            const QString blockerMessage = firstImportBlockerMessage(importBlockersValue);
            errorMessage = blockerMessage.isEmpty()
                ? QObject::tr("Agent manifest has unresolved import blockers.")
                : blockerMessage;
            return false;
        }

        const QJsonValue workflowStatusValue = data.workflow.value("status");
        if (!workflowStatusValue.isString()
            || workflowStatusValue.toString().trimmed() != QStringLiteral("succeeded")) {
            errorMessage = QObject::tr("Agent workflow status must be succeeded.");
            return false;
        }

        const QJsonValue runValue = data.workflow.value("run");
        const QJsonValue runStatusValue = runValue.isObject()
            ? runValue.toObject().value("status")
            : QJsonValue();
        if (!runValue.isObject() || !runStatusValue.isString()
            || runStatusValue.toString().trimmed() != QStringLiteral("run_completed")) {
            errorMessage = QObject::tr("Agent workflow run status must be run_completed.");
            return false;
        }

        const QJsonValue solverFamilyValue = data.solver.value("family");
        if (!solverFamilyValue.isString()
            || solverFamilyValue.toString().trimmed().compare(QStringLiteral("openfoam"), Qt::CaseInsensitive) != 0) {
            errorMessage = QObject::tr("APPFlow only supports OpenFOAM Agent manifests.");
            return false;
        }

        const QJsonValue solverCommandValue = data.solver.value("command");
        if (!solverCommandValue.isString() || solverCommandValue.toString().trimmed().isEmpty()) {
            errorMessage = QObject::tr("Agent manifest solver command is required.");
            return false;
        }

        const QJsonValue caseDirValue = data.artifacts.value("case_dir");
        if (!caseDirValue.isString() || caseDirValue.toString().trimmed().isEmpty()) {
            errorMessage = QObject::tr("Agent manifest case directory is required.");
            return false;
        }
        if (!QFileInfo(caseDirValue.toString().trimmed()).isDir()) {
            errorMessage = QObject::tr("Agent manifest case directory does not exist: %1")
                .arg(caseDirValue.toString().trimmed());
            return false;
        }

        errorMessage.clear();
        return true;
    }
}
