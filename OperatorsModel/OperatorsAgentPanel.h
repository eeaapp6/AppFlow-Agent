/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#ifndef _OperatorsAgentPanel_H
#define _OperatorsAgentPanel_H

#include "FITK_Kernel/FITKCore/FITKActionOperator.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"

namespace ModelOper
{
    class OperatorsAgentPanel : public Core::FITKActionOperator
    {
    public:
        explicit OperatorsAgentPanel() = default;
        ~OperatorsAgentPanel() = default;

    private:
        bool execGUI() override;
    };

    Register2FITKOperatorRepo(actionFoamAgentPanel, OperatorsAgentPanel);
}

#endif
