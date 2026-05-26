/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "OperatorsAgentManifest.h"
#include "AgentBridgeMessage.h"
#include "AgentLogPreview.h"
#include "AgentManifestData.h"
#include "AgentSolverSettingsApplier.h"
#include "AgentSolverSettingsPreview.h"
#include "OpenFoamAgentAdapter.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKAppFramework/FITKAppSettings.h"
#include "FITK_Kernel/FITKAppFramework/FITKMessage.h"
#include "FITK_Kernel/FITKCore/FITKActionOperator.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"

#include <QEventLoop>
#include <QFileDialog>
#include <QFileInfo>

namespace ModelOper
{
    OperatorsAgentManifest::OperatorsAgentManifest()
    {

    }

    OperatorsAgentManifest::~OperatorsAgentManifest()
    {

    }

    bool OperatorsAgentManifest::execGUI()
    {
        QWidget* mw = FITKAPP->getGlobalData()->getMainWindow();

        QString wk;
        AppFrame::FITKAppSettings* settings = FITKAPP->getAppSettings();
        if (settings)
            wk = settings->getWorkingDir();

        QFileDialog dlg(mw, QObject::tr("Import Agent Result"), wk, "JSON(*.json)");
        dlg.setAcceptMode(QFileDialog::AcceptOpen);
        dlg.show();

        bool accept = false;
        QEventLoop loop;
        connect(&dlg, &QFileDialog::accepted, [&] {loop.quit(); accept = true; });
        connect(&dlg, &QFileDialog::rejected, [&] {loop.quit(); accept = false; });
        loop.exec();
        if (!accept) return false;

        QStringList files = dlg.selectedFiles();
        if (files.isEmpty()) return false;
        QString fileName = files.at(0);
        if (fileName.isEmpty()) return false;

        this->setArgs("FileName", fileName);
        return true;
    }

    bool OperatorsAgentManifest::execProfession()
    {
        QString fileName;
        bool ok = this->argValue<QString>("FileName", fileName);
        this->clearArgs();
        if (!ok || fileName.isEmpty()) return false;

        AgentManifestData manifest;
        QString errorMessage;
        if (!loadAgentManifest(fileName, manifest, errorMessage)) {
            AppFrame::FITKMessageError(errorMessage);
            return false;
        }

        QString solverFamily = manifest.solver.value("family").toString();

        QStringList messages;
        messages << tr("Agent manifest loaded: %1").arg(fileName);
        messages << tr("Workflow: %1").arg(manifest.workflow.value("id").toString());
        messages << tr("Status: %1").arg(manifest.workflow.value("status").toString());
        messages << tr("Solver family: %1").arg(solverFamily);
        messages << tr("Solver command: %1").arg(agentValueOrNotProvided(manifest.solver.value("command").toString()));
        messages << tr("Case dir: %1").arg(manifest.artifacts.value("case_dir").toString());
        messages << tr("Mesh dir: %1").arg(agentValueOrNotProvided(manifest.artifacts.value("mesh_dir").toString()));
        messages << tr("Result dir: %1").arg(agentValueOrNotProvided(manifest.artifacts.value("result_dir").toString()));
        messages << tr("Case name: %1").arg(agentValueOrNotProvided(manifest.caseSummary.value("case_name").toString()));
        messages << tr("Case dir exists: %1").arg(QFileInfo(manifest.artifacts.value("case_dir").toString()).isDir() ? "yes" : "no");
        messages << tr("Mesh dir exists: %1").arg(QFileInfo(manifest.artifacts.value("mesh_dir").toString()).isDir() ? "yes" : "no");

        bool isOpenFoam = solverFamily.compare("openfoam", Qt::CaseInsensitive) == 0;
        OpenFoamAgentResult result;
        if (isOpenFoam) {
            result = OpenFoamAgentAdapter::processResults(manifest.artifacts, manifest.appflowHints, messages);
        }
        else {
            messages << tr("APPFlow only supports openfoam Agent manifests.");
            messages << tr("Solver family %1 is not handled by this application.").arg(solverFamily);
        }

        appendAgentSolverSettingsPreviewMessages(manifest.solverSettings, messages);
        applyAgentSolverSettingsToAppFlowModel(manifest.solver, manifest.solverSettings, messages);
        appendAgentLogPreviewMessages(manifest.logs, messages);

        QString caseDir = manifest.artifacts.value("case_dir").toString();
        bool importMesh = manifest.appflowHints.value("import_mesh").toBool(false);
        if (importMesh && isOpenFoam && QFileInfo(caseDir).isDir()) {
            Core::FITKActionOperator* oper = FITKOPERREPO->getOperatorT<Core::FITKActionOperator>("actionImportOpenFoamMesh");
            if (oper) {
                oper->setArgs("FileName", caseDir);
                oper->setArgs("SenderName", "actionImportOpenFoamMesh");
                oper->execProfession();
                messages << tr("OpenFOAM mesh import requested from: %1").arg(caseDir);
            }
            else {
                messages << tr("OpenFOAM mesh import operator not found.");
            }
        }
        else if (importMesh) {
            messages << tr("OpenFOAM mesh import skipped. Solver family or case directory is not ready.");
        }

        if (isOpenFoam) {
            OpenFoamAgentAdapter::openParaViewIfRequested(result, manifest.appflowHints, messages);
        }

        outputAgentNormalMessages(messages);
        return true;
    }
}
