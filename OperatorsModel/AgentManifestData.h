/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#ifndef _AgentManifestData_H
#define _AgentManifestData_H

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

namespace ModelOper
{
    struct AgentManifestData
    {
        int version{ -1 };
        QJsonObject workflow{};
        QJsonObject solver{};
        QJsonObject artifacts{};
        QJsonObject caseSummary{};
        QJsonObject appflowHints{};
        QJsonObject solverSettings{};
        QJsonArray logs{};
    };

    bool loadAgentManifest(const QString& fileName, AgentManifestData& data, QString& errorMessage);
    bool validateAgentManifestImportReadiness(const AgentManifestData& data, QString& errorMessage);
}

#endif
