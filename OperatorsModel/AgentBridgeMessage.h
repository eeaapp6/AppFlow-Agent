/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#ifndef _AgentBridgeMessage_H
#define _AgentBridgeMessage_H

#include <QString>
#include <QStringList>

namespace ModelOper
{
    QString agentValueOrNotProvided(const QString& value);

    void outputAgentNormalMessages(const QStringList& messages);
}

#endif
