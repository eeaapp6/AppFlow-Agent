/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#ifndef _AgentSolverSettingsPreview_H
#define _AgentSolverSettingsPreview_H

#include <QJsonObject>
#include <QStringList>

namespace ModelOper
{
    void appendAgentSolverSettingsPreviewMessages(const QJsonObject& solverSettings, QStringList& messages);
}

#endif
