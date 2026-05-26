/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "AgentManifestData.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QObject>

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
}
