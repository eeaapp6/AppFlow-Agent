/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "AgentSolverSettingsPreview.h"

#include <QJsonArray>
#include <QJsonValue>
#include <QObject>

namespace ModelOper
{
    namespace
    {
        QString settingValueToString(const QJsonValue& value)
        {
            if (value.isString()) return value.toString();
            if (value.isBool()) return value.toBool() ? "true" : "false";
            if (value.isDouble()) return QString::number(value.toDouble(), 'g', 15);
            if (value.isNull()) return "null";
            if (value.isUndefined()) return QString();
            return QString();
        }

        QString settingTopLevelLabel(const QString& key)
        {
            if (key == "source_files") return QObject::tr("Source file");
            if (key.isEmpty()) return key;

            QString label = key;
            label[0] = label.at(0).toUpper();
            return label;
        }

        void appendSettingValue(const QString& label, const QJsonValue& value, QStringList& messages)
        {
            if (value.isObject()) {
                QJsonObject object = value.toObject();
                for (QJsonObject::const_iterator iter = object.constBegin(); iter != object.constEnd(); ++iter) {
                    appendSettingValue(QString("%1 %2").arg(label).arg(iter.key()), iter.value(), messages);
                }
                return;
            }

            if (value.isArray()) {
                QJsonArray array = value.toArray();
                for (int i = 0; i < array.size(); ++i) {
                    QString arrayLabel = QString("%1[%2]").arg(label).arg(i);
                    appendSettingValue(arrayLabel, array.at(i), messages);
                }
                return;
            }

            QString valueText = settingValueToString(value);
            if (!valueText.isEmpty())
                messages << QObject::tr("%1: %2").arg(label).arg(valueText);
        }

        void appendSettingSection(const QJsonObject& solverSettings, const QString& key, QStringList& messages)
        {
            if (!solverSettings.contains(key)) return;
            appendSettingValue(settingTopLevelLabel(key), solverSettings.value(key), messages);
        }
    }

    void appendAgentSolverSettingsPreviewMessages(const QJsonObject& solverSettings, QStringList& messages)
    {
        if (solverSettings.isEmpty()) return;

        messages << QObject::tr("Solver settings:");

        QStringList handledKeys;
        handledKeys << "time" << "physics" << "numerics" << "source_files";
        for (const QString& key : handledKeys)
            appendSettingSection(solverSettings, key, messages);

        for (QJsonObject::const_iterator iter = solverSettings.constBegin(); iter != solverSettings.constEnd(); ++iter) {
            if (handledKeys.contains(iter.key())) continue;
            appendSettingValue(settingTopLevelLabel(iter.key()), iter.value(), messages);
        }
    }
}
