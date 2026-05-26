/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#ifndef _OpenFoamAgentAdapter_H
#define _OpenFoamAgentAdapter_H

#include <QJsonObject>
#include <QString>
#include <QStringList>

namespace ModelOper
{
    struct OpenFoamAgentResult
    {
        bool hasVtkResult{ false };
        QString caseDir{};
        QString resultDir{};
        QString firstVtk{};
    };

    class OpenFoamAgentAdapter
    {
    public:
        static OpenFoamAgentResult processResults(const QJsonObject& artifacts,
            const QJsonObject& appflowHints,
            QStringList& messages);

        static void openParaViewIfRequested(const OpenFoamAgentResult& result,
            const QJsonObject& appflowHints,
            QStringList& messages);
    };
}

#endif
