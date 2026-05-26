#include "AgentController.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDir>
#include <QStringList>
#include <QUrl>
#include <QVariant>

namespace {

QString errorMessageFromResponseBody(const QByteArray &body)
{
    const QJsonDocument document = QJsonDocument::fromJson(body);
    if (!document.isObject()) {
        return QString::fromUtf8(body).trimmed();
    }

    const QJsonObject root = document.object();
    const QString errorText = root.value(QStringLiteral("error")).toString().trimmed();
    if (!errorText.isEmpty()) {
        return errorText;
    }

    return QString::fromUtf8(body).trimmed();
}

QString workflowMessageFromTask(const QJsonObject &task, const QString &appflowManifestPath)
{
    if (task.isEmpty()) {
        return {};
    }

    const QString taskId = task.value(QStringLiteral("task_id")).toString();
    const QString manifestPath = task.value(QStringLiteral("manifest_path")).toString();

    QStringList lines;
    lines << QString::fromUtf8("\xE5\xB7\xA5\xE4\xBD\x9C\xE6\xB5\x81");
    if (!taskId.isEmpty()) {
        lines << QString::fromUtf8("\xE4\xBB\xBB\xE5\x8A\xA1\x20\x49\x44\xEF\xBC\x9A\x25\x31").arg(taskId);
    }
    if (!manifestPath.isEmpty()) {
        if (!appflowManifestPath.isEmpty() && appflowManifestPath != manifestPath) {
            lines << QString::fromUtf8("\x41\x50\x50\x46\x6C\x6F\x77\x20\xE6\xB8\x85\xE5\x8D\x95\xEF\xBC\x9A\x25\x31").arg(appflowManifestPath);
        } else {
            lines << QString::fromUtf8("\x41\x50\x50\x46\x6C\x6F\x77\x20\xE6\xB8\x85\xE5\x8D\x95\xEF\xBC\x9A\x25\x31").arg(manifestPath);
        }
    }

    return lines.join(QStringLiteral("\n"));
}

QString nextActionMessageFromObject(const QJsonObject &nextAction)
{
    if (nextAction.isEmpty()) {
        return {};
    }

    const QString label = nextAction.value(QStringLiteral("label")).toString().trimmed();
    const QString id = nextAction.value(QStringLiteral("id")).toString().trimmed();
    const QString endpoint = nextAction.value(QStringLiteral("endpoint")).toString().trimmed();

    QString title = label;
    if (title.isEmpty()) {
        title = id;
    }
    if (title.isEmpty()) {
        title = endpoint;
    }
    if (title.isEmpty()) {
        return {};
    }

    QStringList lines;
    lines << QString::fromUtf8("\xE5\xBB\xBA\xE8\xAE\xAE\xE4\xB8\x8B\xE4\xB8\x80\xE6\xAD\xA5\xEF\xBC\x9A\x25\x31").arg(title);
    if (!id.isEmpty()) {
        lines << QString::fromUtf8("\xE5\x8A\xA8\xE4\xBD\x9C\x20\x49\x44\xEF\xBC\x9A\x25\x31").arg(id);
    }
    if (!endpoint.isEmpty()) {
        lines << QString::fromUtf8("\xE6\x8E\xA5\xE5\x8F\xA3\xEF\xBC\x9A\x25\x31").arg(endpoint);
    }

    return lines.join(QStringLiteral("\n"));
}

QString valueFromObject(const QJsonObject &object, const QStringList &keys)
{
    for (const QString &key : keys) {
        const QString value = object.value(key).toVariant().toString().trimmed();
        if (!value.isEmpty()) {
            return value;
        }
    }

    return {};
}

void appendSetting(QStringList *items, const QString &name, const QString &value)
{
    if (items == nullptr || value.trimmed().isEmpty()) {
        return;
    }

    items->append(QStringLiteral("%1=%2").arg(name, value.trimmed()));
}

QString solverSettingsMessageFromRoot(const QJsonObject &root)
{
    QJsonObject plan = root.value(QStringLiteral("plan")).toObject();
    if (plan.isEmpty()) {
        const QJsonObject task = root.value(QStringLiteral("task")).toObject();
        const QString taskStatus = task.value(QStringLiteral("status")).toString().trimmed();
        if (taskStatus != QStringLiteral("planned")) {
            return {};
        }
        plan = task.value(QStringLiteral("plan")).toObject();
    }
    if (plan.isEmpty()) {
        return {};
    }

    const QString solverName = plan.value(QStringLiteral("solver_name")).toString().trimmed();
    QJsonObject changes = plan.value(QStringLiteral("requested_changes")).toObject();
    const QJsonObject parameters = plan.value(QStringLiteral("parameters")).toObject();
    if (changes.isEmpty()) {
        changes = parameters.value(QStringLiteral("requested_changes")).toObject();
    }
    if (changes.isEmpty() && solverName.isEmpty()) {
        return {};
    }

    QStringList lines;
    lines << QString::fromUtf8("\xE6\xB1\x82\xE8\xA7\xA3\xE5\x8F\x82\xE6\x95\xB0");
    if (!solverName.isEmpty()) {
        lines << QString::fromUtf8("\xE6\xB1\x82\xE8\xA7\xA3\xE5\x99\xA8\xEF\xBC\x9A\x25\x31").arg(solverName);
    }

    QStringList timeItems;
    appendSetting(&timeItems, QStringLiteral("start_time"), valueFromObject(changes, {QStringLiteral("start_time"), QStringLiteral("startTime")}));
    appendSetting(&timeItems, QStringLiteral("end_time"), valueFromObject(changes, {QStringLiteral("end_time"), QStringLiteral("endTime"), QStringLiteral("final_time"), QStringLiteral("finalTime")}));
    appendSetting(&timeItems, QStringLiteral("delta_t"), valueFromObject(changes, {QStringLiteral("delta_t"), QStringLiteral("deltaT"), QStringLiteral("time_step"), QStringLiteral("timestep")}));
    appendSetting(&timeItems, QStringLiteral("write_interval"), valueFromObject(changes, {QStringLiteral("write_interval"), QStringLiteral("writeInterval")}));
    if (!timeItems.isEmpty()) {
        lines << QString::fromUtf8("\xE6\x97\xB6\xE9\x97\xB4\xEF\xBC\x9A\x25\x31").arg(timeItems.join(QStringLiteral(", ")));
    }

    QStringList physicsItems;
    appendSetting(&physicsItems, QStringLiteral("nu"), valueFromObject(changes, {QStringLiteral("kinematic_viscosity"), QStringLiteral("viscosity"), QStringLiteral("nu")}));
    appendSetting(&physicsItems, QStringLiteral("rho"), valueFromObject(changes, {QStringLiteral("density"), QStringLiteral("rho")}));
    appendSetting(&physicsItems, QStringLiteral("lid_velocity"), valueFromObject(changes, {QStringLiteral("lid_velocity"), QStringLiteral("top_velocity"), QStringLiteral("velocity")}));
    appendSetting(&physicsItems, QStringLiteral("inlet_velocity"), valueFromObject(changes, {QStringLiteral("inlet_velocity"), QStringLiteral("inletVelocity"), QStringLiteral("inflow_velocity"), QStringLiteral("inflowVelocity")}));
    appendSetting(&physicsItems, QStringLiteral("outlet_pressure"), valueFromObject(changes, {QStringLiteral("outlet_pressure"), QStringLiteral("outletPressure"), QStringLiteral("pressure_outlet"), QStringLiteral("pressureOutlet")}));
    if (!physicsItems.isEmpty()) {
        lines << QString::fromUtf8("\xE7\x89\xA9\xE7\x90\x86\xEF\xBC\x9A\x25\x31").arg(physicsItems.join(QStringLiteral(", ")));
    }

    QStringList numericsItems;
    appendSetting(&numericsItems, QStringLiteral("max_co"), valueFromObject(changes, {QStringLiteral("max_co"), QStringLiteral("maxCo"), QStringLiteral("max_courant")}));
    appendSetting(&numericsItems, QStringLiteral("max_delta_t"), valueFromObject(changes, {QStringLiteral("max_delta_t"), QStringLiteral("maxDeltaT"), QStringLiteral("max_time_step")}));
    appendSetting(&numericsItems, QStringLiteral("adjust_time_step"), valueFromObject(changes, {QStringLiteral("adjust_time_step"), QStringLiteral("adjustTimeStep"), QStringLiteral("adaptive_time_step")}));
    if (!numericsItems.isEmpty()) {
        lines << QString::fromUtf8("\xE6\x95\xB0\xE5\x80\xBC\xEF\xBC\x9A\x25\x31").arg(numericsItems.join(QStringLiteral(", ")));
    }

    return lines.size() > 1 ? lines.join(QStringLiteral("\n")) : QString();
}

}

AgentController::AgentController(QObject *parent)
    : QObject(parent)
{
    m_network = new QNetworkAccessManager(this);
}

bool AgentController::isRunning() const
{
    return m_currentReply != nullptr;
}

void AgentController::setOutputDirectory(const QString &path)
{
    m_outputDirectory = path.trimmed();
}

void AgentController::setHostOutputDirectory(const QString &path)
{
    m_hostOutputDirectory = path.trimmed();
}

void AgentController::submitUserMessage(const QString &text)
{
    if (m_currentReply != nullptr) {
        cancelCurrentTask();
    }

    QJsonObject payload;
    payload.insert(QStringLiteral("message"), text);
    if (!m_outputDirectory.isEmpty()) {
        payload.insert(QStringLiteral("output_dir"), m_outputDirectory);
    }
    if (!m_hostOutputDirectory.isEmpty()) {
        payload.insert(QStringLiteral("host_output_dir"), m_hostOutputDirectory);
    }

    postJson(QStringLiteral("/foam/plan"), payload);
}

void AgentController::generateCaseForTask(const QString &taskDir, const QJsonObject &repairAction)
{
    if (m_currentReply != nullptr) {
        cancelCurrentTask();
    }

    QJsonObject payload;
    payload.insert(QStringLiteral("task_dir"), taskDir);
    if (!repairAction.isEmpty()) {
        payload.insert(QStringLiteral("repair_action"), repairAction);
    }

    postJson(QStringLiteral("/foam/generate"), payload);
}

void AgentController::validateCaseForTask(const QString &taskDir)
{
    if (m_currentReply != nullptr) {
        cancelCurrentTask();
    }

    QJsonObject payload;
    payload.insert(QStringLiteral("task_dir"), taskDir);

    postJson(QStringLiteral("/foam/validate"), payload);
}

void AgentController::runCaseForTask(const QString &taskDir)
{
    if (m_currentReply != nullptr) {
        cancelCurrentTask();
    }

    QJsonObject payload;
    payload.insert(QStringLiteral("task_dir"), taskDir);

    postJson(QStringLiteral("/foam/run"), payload);
}

void AgentController::replanTaskForMessage(const QString &taskDir, const QString &message)
{
    if (m_currentReply != nullptr) {
        cancelCurrentTask();
    }

    QJsonObject payload;
    payload.insert(QStringLiteral("task_dir"), taskDir);
    payload.insert(QStringLiteral("message"), message);
    payload.insert(QStringLiteral("mode"), QStringLiteral("replan"));
    if (!m_outputDirectory.isEmpty()) {
        payload.insert(QStringLiteral("output_dir"), m_outputDirectory);
    }
    if (!m_hostOutputDirectory.isEmpty()) {
        payload.insert(QStringLiteral("host_output_dir"), m_hostOutputDirectory);
    }

    postJson(QStringLiteral("/foam/replan"), payload);
}

void AgentController::recordRepairActionForTask(const QString &taskDir, const QJsonObject &repairAction)
{
    QJsonObject payload;
    payload.insert(QStringLiteral("task_dir"), taskDir);
    payload.insert(QStringLiteral("repair_action"), repairAction);

    QNetworkRequest request(QUrl(QStringLiteral("http://127.0.0.1:8765/foam/repair-action")));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    const QByteArray body = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    QNetworkReply *reply = m_network->post(request, body);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const QByteArray body = reply->readAll();
        const QJsonObject root = QJsonDocument::fromJson(body).object();
        const QJsonObject task = root.value(QStringLiteral("task")).toObject();
        const QJsonArray repairHistory = task.value(QStringLiteral("repair_history")).toArray();
        if (!repairHistory.isEmpty()) {
            emit repairHistoryReceived(repairHistory);
        }
        reply->deleteLater();
    });
}

QString AgentController::hostPathForBackendPath(const QString &backendPath) const
{
    const QString normalizedOutput = m_outputDirectory.trimmed().replace('\\', '/');
    const QString normalizedBackendPath = backendPath.trimmed().replace('\\', '/');
    if (m_hostOutputDirectory.isEmpty()
        || normalizedOutput.isEmpty()
        || !normalizedBackendPath.startsWith(normalizedOutput)) {
        return {};
    }

    QString relativePath = normalizedBackendPath.mid(normalizedOutput.size());
    while (relativePath.startsWith('/')) {
        relativePath.remove(0, 1);
    }
    return QDir(m_hostOutputDirectory).filePath(relativePath);
}

void AgentController::postJson(const QString &path, const QJsonObject &payload)
{
    QNetworkRequest request(QUrl(QStringLiteral("http://127.0.0.1:8765%1").arg(path)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    const QByteArray body = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    m_currentReply = m_network->post(request, body);
    m_currentRequestPath = path;

    emit taskRunningChanged(true);

    connect(m_currentReply, &QNetworkReply::finished, this, [this, reply = m_currentReply]() {
        handleReplyFinished(reply);
    });
}

void AgentController::cancelCurrentTask()
{
    if (m_currentReply == nullptr) {
        return;
    }

    QNetworkReply *reply = m_currentReply;
    m_currentReply = nullptr;
    m_currentRequestPath.clear();
    reply->abort();
    reply->deleteLater();

    emit taskRunningChanged(false);
}

void AgentController::handleReplyFinished(QNetworkReply *reply)
{
    if (reply != m_currentReply) {
        return;
    }

    m_currentReply = nullptr;
    const QString requestPath = m_currentRequestPath;
    m_currentRequestPath.clear();

    const QByteArray body = reply->readAll();
    if (reply->error() != QNetworkReply::NoError) {
        const QString responseError = errorMessageFromResponseBody(body);
        const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QString message;
        if (requestPath == QStringLiteral("/foam/replan") && httpStatus == 404) {
            message = QString::fromUtf8("\xE5\x90\x8E\xE7\xAB\xAF\xE6\x9A\x82\xE6\x9C\xAA\xE6\x94\xAF\xE6\x8C\x81\xE5\x8F\x82\xE6\x95\xB0\xE9\x87\x8D\xE6\x96\xB0\xE8\xA7\x84\xE5\x88\x92\xE6\x8E\xA5\xE5\x8F\xA3\xE3\x80\x82\xE5\xBD\x93\xE5\x89\x8D\xE4\xBF\xAE\xE6\x94\xB9\xE8\xAF\xB7\xE6\xB1\x82\xE5\xB7\xB2\xE8\xAF\x86\xE5\x88\xAB\xEF\xBC\x8C\xE4\xBD\x86\xE6\x9C\xAA\xE6\x89\xA7\xE8\xA1\x8C\xE3\x80\x82");
        } else {
            message = responseError.isEmpty()
                ? QString::fromUtf8("\xE6\xA8\xA1\xE5\x9E\x8B\xE6\x9C\x8D\xE5\x8A\xA1\xE8\xAF\xB7\xE6\xB1\x82\xE5\xA4\xB1\xE8\xB4\xA5\xEF\xBC\x9A\x25\x31").arg(reply->errorString())
                : QString::fromUtf8("\xE6\xA8\xA1\xE5\x9E\x8B\xE6\x9C\x8D\xE5\x8A\xA1\xE8\xAF\xB7\xE6\xB1\x82\xE5\xA4\xB1\xE8\xB4\xA5\xEF\xBC\x9A\x25\x31").arg(responseError);
        }

        emit errorMessageReceived(message);
        emit taskRunningChanged(false);
        reply->deleteLater();
        return;
    }

    const QJsonDocument document = QJsonDocument::fromJson(body);
    const QJsonObject root = document.object();
    const QString replyText = root.value(QStringLiteral("reply")).toString().trimmed();
    const QString errorText = root.value(QStringLiteral("error")).toString().trimmed();
    const QJsonObject task = root.value(QStringLiteral("task")).toObject();
    const QJsonArray gateReviews = task.value(QStringLiteral("gate_reviews")).toArray();
    const QJsonArray repairHistory = task.value(QStringLiteral("repair_history")).toArray();
    const QJsonObject repairAction = root.value(QStringLiteral("repair_action")).toObject();
    const QString taskDir = task.value(QStringLiteral("task_dir")).toString().trimmed();
    const QString taskStatus = task.value(QStringLiteral("status")).toString().trimmed();
    const QString manifestPath = task.value(QStringLiteral("manifest_path")).toString().trimmed();
    const QString appflowManifestPath = hostPathForBackendPath(manifestPath);
    const QString effectiveManifestPath = appflowManifestPath.isEmpty() ? manifestPath : appflowManifestPath;
    const QJsonObject nextAction = root.value(QStringLiteral("next_action")).toObject();
    const QString nextActionId = nextAction.value(QStringLiteral("id")).toString().trimmed();
    const QString nextActionLabel = nextAction.value(QStringLiteral("label")).toString().trimmed();
    const QString nextActionEndpoint = nextAction.value(QStringLiteral("endpoint")).toString().trimmed();
    const QString nextActionMessage = nextActionMessageFromObject(nextAction);

    if (!errorText.isEmpty()) {
        emit errorMessageReceived(errorText);
    } else if (!replyText.isEmpty()) {
        const QString workflowMessage = workflowMessageFromTask(task, appflowManifestPath);
        if (!workflowMessage.isEmpty()) {
            emit workflowMessageReceived(workflowMessage);
        }
        const QString solverSettingsMessage = solverSettingsMessageFromRoot(root);
        if (!solverSettingsMessage.isEmpty()) {
            emit workflowMessageReceived(solverSettingsMessage);
        }
        if (!taskDir.isEmpty()) {
            emit taskContextReceived(taskDir);
            emit gateReviewsReceived(gateReviews);
            emit repairHistoryReceived(repairHistory);
            if (taskStatus == QStringLiteral("generated")) {
                emit caseGenerated(taskDir);
            }
            if (taskStatus == QStringLiteral("validated")) {
                emit caseValidated(taskDir);
            }
        }
        if (!effectiveManifestPath.isEmpty()) {
            emit manifestPathReceived(effectiveManifestPath);
        }
        if (!nextActionMessage.isEmpty()) {
            emit nextActionReceived(nextActionId, nextActionLabel, nextActionEndpoint, nextActionMessage);
        }
        if (!taskDir.isEmpty()) {
            emit repairActionReceived(repairAction);
        }
        emit agentMessageReceived(replyText);
    } else {
        emit errorMessageReceived(QString::fromUtf8("\xE6\xA8\xA1\xE5\x9E\x8B\xE6\x9C\x8D\xE5\x8A\xA1\xE8\xBF\x94\xE5\x9B\x9E\xE7\xA9\xBA\xE5\x9B\x9E\xE5\xA4\x8D\xE3\x80\x82"));
    }

    emit taskRunningChanged(false);
    reply->deleteLater();
}
