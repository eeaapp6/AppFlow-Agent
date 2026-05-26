/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "OperatorsAgentPanel.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKMessage.h"
#include "GUIFrame/MainWindow.h"

namespace ModelOper
{
    bool OperatorsAgentPanel::execGUI()
    {
        GUI::MainWindow* mainWindow = FITKAPP->getGlobalData()->getMainWindowT<GUI::MainWindow>();
        if (mainWindow == nullptr) {
            AppFrame::FITKMessageError(tr("Main window is not ready."));
            return false;
        }

        mainWindow->showAgentPanel();
        return true;
    }
}
