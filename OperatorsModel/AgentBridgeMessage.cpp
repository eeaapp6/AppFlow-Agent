/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "AgentBridgeMessage.h"

#include "FITK_Kernel/FITKAppFramework/FITKMessage.h"

#include <QObject>

namespace ModelOper
{
    QString agentValueOrNotProvided(const QString& value)
    {
        return value.isEmpty() ? QObject::tr("[not provided]") : value;
    }

    void outputAgentNormalMessages(const QStringList& messages)
    {
        for (const QString& message : messages) {
            const QStringList lines = message.split('\n');
            for (const QString& line : lines) {
                AppFrame::FITKMessageNormal(line);
            }
        }
    }
}
