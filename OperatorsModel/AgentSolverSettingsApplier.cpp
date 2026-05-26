/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "AgentSolverSettingsApplier.h"

#include "OperatorsInterface/TreeEventOperator.h"

#include "FITK_Kernel/FITKAppFramework/FITKAppFramework.h"
#include "FITK_Kernel/FITKAppFramework/FITKComponents.h"
#include "FITK_Kernel/FITKAppFramework/FITKGlobalData.h"
#include "FITK_Kernel/FITKCore/FITKOperatorRepo.h"
#include "FITK_Kernel/FITKEasyParam/FITKAbstractEasyParam.h"
#include "FITK_Kernel/FITKEasyParam/FITKParameter.h"
#include "FITK_Kernel/FITKEasyParam/FITKParamBoolGroup.h"
#include "FITK_Kernel/FITKEasyParam/FITKParamDouble.h"
#include "FITK_Kernel/FITKEasyParam/FITKParamInt.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKAbstractOFSolver.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKFlowPhysicsHandlerFactory.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFPhysicsData.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFRunControl.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFTransportModel.h"
#include "FITK_Interface/FITKInterfaceFlowOF/FITKOFTransportProp.h"

#include <QJsonValue>
#include <QObject>
#include <QtMath>

namespace ModelOper
{
    namespace
    {
        QString appFlowSolverNameFromCommand(const QString& command)
        {
            QString normalized = command.trimmed().toLower();
            if (normalized == "simplefoam") return "SIMPLE";
            if (normalized == "interfoam") return "Inter";
            if (normalized == "foammultirun") return "CHT Multi Region";
            return QString();
        }

        bool readNumber(const QJsonObject& object, const QStringList& keys, double& value)
        {
            for (const QString& key : keys) {
                QJsonValue jsonValue = object.value(key);
                if (jsonValue.isUndefined() || jsonValue.isNull()) continue;

                if (jsonValue.isDouble()) {
                    value = jsonValue.toDouble();
                    return true;
                }

                if (jsonValue.isString()) {
                    bool ok = false;
                    double number = jsonValue.toString().toDouble(&ok);
                    if (ok) {
                        value = number;
                        return true;
                    }
                }
            }

            return false;
        }

        bool setParameterNumber(Core::FITKParameter* parameter, const QStringList& names, double value, const QString& createName)
        {
            if (parameter == nullptr) return false;

            for (const QString& name : names) {
                Core::FITKAbstractEasyParam* data = parameter->getDataByName(name);
                if (data == nullptr) continue;

                Core::FITKParamDouble* doubleData = dynamic_cast<Core::FITKParamDouble*>(data);
                if (doubleData) {
                    doubleData->setValue(value);
                    return true;
                }

                Core::FITKParamInt* intData = dynamic_cast<Core::FITKParamInt*>(data);
                if (intData) {
                    intData->setValue(qRound(value));
                    return true;
                }
            }

            if (!createName.isEmpty()) {
                parameter->createDoubleParameter(createName, value);
                return true;
            }

            return false;
        }

        bool setAdjustTimeStepNumber(Core::FITKParameter* timeControl, const QStringList& names, double value)
        {
            if (timeControl == nullptr) return false;

            Core::FITKParamBoolGroup* adjustTimeStep =
                dynamic_cast<Core::FITKParamBoolGroup*>(timeControl->getDataByName("Adjust Time Stepping"));
            if (adjustTimeStep == nullptr) return false;

            return setParameterNumber(adjustTimeStep->getValueGroup(), names, value, QString());
        }

        void appendAppliedMessage(QStringList& messages, const QString& name, double value)
        {
            messages << QObject::tr("Agent setting applied to APPFlow RunControl: %1 = %2")
                .arg(name)
                .arg(QString::number(value, 'g', 15));
        }

        void appendTransportAppliedMessage(QStringList& messages, const QString& name, double value)
        {
            messages << QObject::tr("Agent setting applied to APPFlow Transport Properties: %1 = %2")
                .arg(name)
                .arg(QString::number(value, 'g', 15));
        }

        void refreshModelTree()
        {
            EventOper::TreeEventOperator* treeOper =
                Core::FITKOperatorRepo::getInstance()->getOperatorT<EventOper::TreeEventOperator>("ModelTreeEvent");
            if (treeOper) treeOper->updateTree();
        }

        bool ensureSolverSelected(const QJsonObject& solver, QStringList& messages)
        {
            QString command = solver.value("command").toString();
            QString appFlowSolverName = appFlowSolverNameFromCommand(command);
            if (appFlowSolverName.isEmpty()) {
                if (!command.isEmpty()) {
                    messages << QObject::tr("No native APPFlow solver mapping for Agent solver command: %1").arg(command);
                    messages << QObject::tr("RunControl settings can be displayed, but solver-specific Setup nodes require a native APPFlow solver.");
                }
                return false;
            }

            Interface::FITKOFPhysicsData* physicsData =
                FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
            if (physicsData == nullptr) return false;

            Interface::FITKAbstractOFSolver* currentSolver = physicsData->getSolver();
            if (currentSolver && currentSolver->getDataObjectName() == appFlowSolverName)
                return true;

            if (currentSolver) {
                messages << QObject::tr("APPFlow solver is already selected: %1").arg(currentSolver->getDataObjectName());
                messages << QObject::tr("Agent solver mapping was not applied automatically: %1 -> %2")
                    .arg(command)
                    .arg(appFlowSolverName);
                return false;
            }

            AppFrame::FITKCmponents* components = FITKAPP->getComponents();
            if (components == nullptr) {
                messages << QObject::tr("Agent solver mapping was not applied: APPFlow components are not ready.");
                return false;
            }

            Interface::FITKFlowPhysicsHandlerFactory* factory =
                components->getComponentTByName<Interface::FITKFlowPhysicsHandlerFactory>("FITKFlowPhysicsHandlerFactory");
            if (factory == nullptr) {
                messages << QObject::tr("Agent solver mapping was not applied: FITKFlowPhysicsHandlerFactory is not ready.");
                return false;
            }

            factory->setSolver(appFlowSolverName);
            messages << QObject::tr("Agent solver mapped to APPFlow solver: %1 -> %2")
                .arg(command)
                .arg(appFlowSolverName);
            return true;
        }

        bool applyPhysicsSettings(Interface::FITKOFPhysicsData* physicsData, const QJsonObject& physicsSettings, QStringList& messages)
        {
            if (physicsData == nullptr || physicsSettings.isEmpty()) return false;

            double value = 0.0;
            if (!readNumber(physicsSettings, QStringList() << "nu", value)) return false;

            Interface::FITKOFTransportProp* transportProp = physicsData->getTransportProp();
            if (transportProp == nullptr) {
                messages << QObject::tr("Agent physics setting was not applied: APPFlow Transport Properties is not ready.");
                return false;
            }

            Interface::FITKOFTransportPhase* phase = transportProp->getPhase(0);
            if (phase == nullptr) {
                messages << QObject::tr("Agent physics setting was not applied: APPFlow Transport phase is not ready.");
                return false;
            }

            Interface::FITKAbsOFTransportModel* transportModel = phase->getTransportModel();
            if (transportModel == nullptr) {
                messages << QObject::tr("Agent physics setting was not applied: APPFlow Transport Model is not ready.");
                return false;
            }

            bool applied = setParameterNumber(
                transportModel->getTransportModelPara(),
                QStringList() << "v [m2/s]" << "nu",
                value,
                QString());
            if (applied) {
                appendTransportAppliedMessage(messages, "nu", value);
            }
            else {
                messages << QObject::tr("Agent physics setting was not applied: APPFlow nu parameter was not found.");
            }

            return applied;
        }
    }

    bool applyAgentSolverSettingsToAppFlowModel(const QJsonObject& solver, const QJsonObject& solverSettings, QStringList& messages)
    {
        applyAgentSolverToAppFlowModel(solver, messages);

        if (solverSettings.isEmpty()) return false;
        QJsonObject timeSettings = solverSettings.value("time").toObject();
        QJsonObject physicsSettings = solverSettings.value("physics").toObject();
        if (timeSettings.isEmpty() && physicsSettings.isEmpty()) return false;

        Interface::FITKOFPhysicsData* physicsData =
            FITKAPP->getGlobalData()->getPhysicsData<Interface::FITKOFPhysicsData>();
        if (physicsData == nullptr) {
            messages << QObject::tr("Agent settings were not applied: APPFlow physics data is not ready.");
            return false;
        }

        Interface::FITKOFRunControl* runControl = physicsData->getRunControl();
        if (runControl == nullptr) {
            runControl = new Interface::FITKOFRunControl;
            physicsData->setRunControl(runControl);
            messages << QObject::tr("APPFlow RunControl was created for Agent settings.");
        }

        bool applied = false;

        if (!timeSettings.isEmpty()) {
            Core::FITKParameter* timeControl = runControl->getTimeControl();
            Core::FITKParameter* outputControl = runControl->getOutputControl();

            double value = 0.0;
            if (readNumber(timeSettings, QStringList() << "delta_t" << "deltaT" << "time_step", value)) {
                bool currentApplied = setParameterNumber(
                    timeControl,
                    QStringList() << "Intial Time [s]" << "Initial Time [s]",
                    value,
                    "Intial Time [s]");
                setAdjustTimeStepNumber(timeControl, QStringList() << "max Delta Time", value);
                if (currentApplied) {
                    applied = true;
                    appendAppliedMessage(messages, "delta_t", value);
                }
            }

            if (readNumber(timeSettings, QStringList() << "end_time" << "endTime", value)) {
                bool currentApplied = setParameterNumber(
                    timeControl,
                    QStringList() << "Simulation Time [s]",
                    value,
                    "Simulation Time [s]");
                if (currentApplied) {
                    applied = true;
                    appendAppliedMessage(messages, "end_time", value);
                }
            }

            if (readNumber(timeSettings, QStringList() << "write_interval" << "writeInterval", value)) {
                bool currentApplied = setParameterNumber(
                    outputControl,
                    QStringList() << "Write Interval",
                    value,
                    "Write Interval");
                if (currentApplied) {
                    applied = true;
                    appendAppliedMessage(messages, "write_interval", value);
                }
            }
        }

        if (applyPhysicsSettings(physicsData, physicsSettings, messages))
            applied = true;

        if (applied) refreshModelTree();
        return applied;
    }

    bool applyAgentSolverToAppFlowModel(const QJsonObject& solver, QStringList& messages)
    {
        bool applied = ensureSolverSelected(solver, messages);
        if (applied) refreshModelTree();
        return applied;
    }
}
