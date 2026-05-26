/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

#include "AgentLogPreview.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QObject>
#include <QTextStream>

namespace ModelOper
{
    namespace
    {
        QString readLogPreview(const QString& fileName, int maxLines, int maxChars)
        {
            QFile file(fileName);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();

            QTextStream in(&file);
            QStringList lines;
            int chars = 0;
            while (!in.atEnd() && lines.size() < maxLines && chars < maxChars) {
                QString line = in.readLine();
                chars += line.size() + 1;
                lines << line;
            }
            file.close();
            return lines.join("\n");
        }
    }

    void appendAgentLogPreviewMessages(const QJsonArray& logs, QStringList& messages)
    {
        for (int i = 0; i < logs.size(); ++i) {
            QJsonObject logObj = logs.at(i).toObject();
            QString logPath = logObj.value("path").toString();
            messages << QObject::tr("Log[%1] %2: %3")
                .arg(i)
                .arg(logObj.value("type").toString())
                .arg(logPath);
            bool logExists = QFileInfo(logPath).isFile();
            messages << QObject::tr("Log[%1] exists: %2").arg(i).arg(logExists ? "yes" : "no");
            if (logExists) {
                QString preview = readLogPreview(logPath, 80, 8000);
                if (!preview.isEmpty()) {
                    messages << QObject::tr("Log[%1] preview:").arg(i);
                    messages << "----------------------------------------";
                    messages << preview;
                    messages << "----------------------------------------";
                }
            }
        }
    }
}
