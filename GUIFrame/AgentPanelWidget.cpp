/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "AgentPanelWidget.h"
#include "AgentChat/FoamAgentChatPanel.h"

#include <QVBoxLayout>

namespace GUI
{
    AgentPanelWidget::AgentPanelWidget(QWidget* parent)
        : QWidget(parent)
    {
        this->setObjectName("AgentPanelWidget");

        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        ::FoamAgentChatPanel* chatPanel = new ::FoamAgentChatPanel(this);
        mainLayout->addWidget(chatPanel);
    }
}
