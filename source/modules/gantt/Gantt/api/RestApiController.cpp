#include "RestApiController.hpp"
#include <QJsonArray>
#include <QJsonDocument>

namespace source::gantt {

RestApiController::RestApiController(ScheduleService *schedule_service,
                                     PlanningEngineService *planning_service,
                                     FieldProgressService *progress_service,
                                     Bim4DService *bim_service,
                                     CsvExchangeService *csv_service,
                                     SecurityService *security_service,
                                     WebhookDispatcher *webhook_dispatcher,
                                     HttpServer *server,
                                     QObject *parent)
    : QObject(parent),
      schedule_svc_(schedule_service),
      planning_svc_(planning_service),
      progress_svc_(progress_service),
      bim_svc_(bim_service),
      csv_svc_(csv_service),
      security_svc_(security_service),
      webhook_dispatcher_(webhook_dispatcher),
      server_(server)
{
    register_endpoints();
}

bool RestApiController::authorize_request(const HttpRequest &req, UserRole min_role, HttpResponse &out_error)
{
    if (security_svc_ && !security_svc_->check_rate_limit(req.client_ip)) {
        out_error = HttpResponse::error(QStringLiteral("RATE_LIMIT_EXCEEDED"), QStringLiteral("Limite de requisições excedido. Tente novamente em alguns segundos."), 429);
        return false;
    }

    if (!security_svc_) return true;

    UserRole user_role = UserRole::Viewer;
    QString user_id;

    if (!req.auth_token.isEmpty()) {
        if (!security_svc_->validate_token(req.auth_token, user_role, user_id)) {
            out_error = HttpResponse::error(QStringLiteral("UNAUTHORIZED"), QStringLiteral("Token de autenticação inválido ou expirado."), 401);
            return false;
        }
    } else {
        // Allow unauthenticated local requests default viewer access
        user_role = UserRole::Viewer;
    }

    // Role hierarchy check: Admin > Engineer > Manager > Viewer
    if (min_role == UserRole::Admin && user_role != UserRole::Admin) {
        out_error = HttpResponse::error(QStringLiteral("FORBIDDEN"), QStringLiteral("Permissão insuficiente. Requer perfil de Administrador."), 403);
        return false;
    }
    if (min_role == UserRole::Engineer && user_role != UserRole::Admin && user_role != UserRole::Engineer) {
        out_error = HttpResponse::error(QStringLiteral("FORBIDDEN"), QStringLiteral("Permissão insuficiente. Requer perfil de Engenheiro."), 403);
        return false;
    }

    return true;
}

void RestApiController::register_endpoints()
{
    if (!server_) return;

    // 1. Projects & Schedules
    server_->add_route(QStringLiteral("POST"), QStringLiteral("/api/v1/projects/{projectId}/schedules"),
                       [this](const HttpRequest &req) { return handle_create_schedule(req); });

    server_->add_route(QStringLiteral("GET"), QStringLiteral("/api/v1/projects/{projectId}/schedules/{scheduleId}"),
                       [this](const HttpRequest &req) { return handle_get_schedule(req); });

    server_->add_route(QStringLiteral("GET"), QStringLiteral("/api/v1/schedules/{scheduleId}"),
                       [this](const HttpRequest &req) { return handle_get_schedule(req); });

    // 2. Schedule Items
    server_->add_route(QStringLiteral("POST"), QStringLiteral("/api/v1/schedules/{scheduleId}/items"),
                       [this](const HttpRequest &req) { return handle_create_item(req); });

    server_->add_route(QStringLiteral("PATCH"), QStringLiteral("/api/v1/schedule-items/{itemId}"),
                       [this](const HttpRequest &req) { return handle_patch_item(req); });

    // 3. Dependencies
    server_->add_route(QStringLiteral("POST"), QStringLiteral("/api/v1/schedules/{scheduleId}/dependencies"),
                       [this](const HttpRequest &req) { return handle_create_dependency(req); });

    // 4. Recalculate
    server_->add_route(QStringLiteral("POST"), QStringLiteral("/api/v1/schedules/{scheduleId}/recalculate"),
                       [this](const HttpRequest &req) { return handle_recalculate(req); });

    // 5. Progress
    server_->add_route(QStringLiteral("POST"), QStringLiteral("/api/v1/schedule-items/{itemId}/progress"),
                       [this](const HttpRequest &req) { return handle_update_progress(req); });

    // 6. Baseline
    server_->add_route(QStringLiteral("POST"), QStringLiteral("/api/v1/schedules/{scheduleId}/baseline"),
                       [this](const HttpRequest &req) { return handle_create_baseline(req); });

    // 7. CSV Integrations
    server_->add_route(QStringLiteral("POST"), QStringLiteral("/api/v1/integrations/import/csv"),
                       [this](const HttpRequest &req) { return handle_import_csv(req); });

    server_->add_route(QStringLiteral("GET"), QStringLiteral("/api/v1/integrations/export/csv"),
                       [this](const HttpRequest &req) { return handle_export_csv(req); });

    // 8. BIM 4D
    server_->add_route(QStringLiteral("GET"), QStringLiteral("/api/v1/schedules/{scheduleId}/bim-simulation"),
                       [this](const HttpRequest &req) { return handle_bim_simulation(req); });

    server_->add_route(QStringLiteral("POST"), QStringLiteral("/api/v1/schedules/{scheduleId}/bim-links"),
                       [this](const HttpRequest &req) { return handle_create_bim_link(req); });

    // 9. OpenAPI spec
    server_->add_route(QStringLiteral("GET"), QStringLiteral("/api/v1/openapi.json"),
                       [this](const HttpRequest &req) { return handle_openapi_spec(req); });
}

HttpResponse RestApiController::handle_create_schedule(const HttpRequest &req)
{
    HttpResponse err;
    if (!authorize_request(req, UserRole::Engineer, err)) return err;

    QString project_id = req.query_params.value(QStringLiteral("projectId"));
    QString name = req.json_body.value(QStringLiteral("name")).toString();
    QString desc = req.json_body.value(QStringLiteral("description")).toString();

    if (name.trimmed().isEmpty()) {
        return HttpResponse::error(QStringLiteral("VALIDATION_ERROR"), QStringLiteral("Nome do cronograma é obrigatório."), 422);
    }

    Schedule s = schedule_svc_->create_schedule(name, project_id, desc);
    if (webhook_dispatcher_) {
        webhook_dispatcher_->dispatch_event(QStringLiteral("schedule.created"), s.to_json());
    }

    return HttpResponse::json(s.to_json(), 201);
}

HttpResponse RestApiController::handle_get_schedule(const HttpRequest &req)
{
    HttpResponse err;
    if (!authorize_request(req, UserRole::Viewer, err)) return err;

    QString sched_id = req.query_params.value(QStringLiteral("scheduleId"));
    Schedule *s = schedule_svc_->get_schedule(sched_id);
    if (!s) {
        return HttpResponse::error(QStringLiteral("NOT_FOUND"), QStringLiteral("Cronograma não encontrado."), 404);
    }

    return HttpResponse::json(s->to_json());
}

HttpResponse RestApiController::handle_create_item(const HttpRequest &req)
{
    HttpResponse err;
    if (!authorize_request(req, UserRole::Engineer, err)) return err;

    QString sched_id = req.query_params.value(QStringLiteral("scheduleId"));
    ScheduleItem item = ScheduleItem::from_json(req.json_body);

    QString add_err;
    if (!schedule_svc_->add_activity(sched_id, item, &add_err)) {
        return HttpResponse::error(QStringLiteral("VALIDATION_ERROR"), add_err, 422);
    }

    if (webhook_dispatcher_) {
        webhook_dispatcher_->dispatch_event(QStringLiteral("activity.created"), item.to_json());
    }

    return HttpResponse::json(item.to_json(), 201);
}

HttpResponse RestApiController::handle_patch_item(const HttpRequest &req)
{
    HttpResponse err;
    if (!authorize_request(req, UserRole::Engineer, err)) return err;

    QString item_id = req.query_params.value(QStringLiteral("itemId"));

    // Locate activity across schedules
    for (auto &sched : schedule_svc_->list_schedules()) {
        Schedule *s = schedule_svc_->get_schedule(sched.id);
        ScheduleItem *item = s->find_item(item_id);
        if (item) {
            if (req.json_body.contains(QStringLiteral("name"))) item->name = req.json_body[QStringLiteral("name")].toString();
            if (req.json_body.contains(QStringLiteral("description"))) item->description = req.json_body[QStringLiteral("description")].toString();
            if (req.json_body.contains(QStringLiteral("duration_hours"))) item->duration_hours = req.json_body[QStringLiteral("duration_hours")].toDouble();
            if (req.json_body.contains(QStringLiteral("percent_complete"))) item->percent_complete = req.json_body[QStringLiteral("percent_complete")].toDouble();
            if (req.json_body.contains(QStringLiteral("is_milestone"))) item->is_milestone = req.json_body[QStringLiteral("is_milestone")].toBool();
            if (req.json_body.contains(QStringLiteral("task_id"))) item->task_id = req.json_body[QStringLiteral("task_id")].toString();

            item->sanitize();
            item->updated_at = QDateTime::currentDateTime();

            if (webhook_dispatcher_) {
                webhook_dispatcher_->dispatch_event(QStringLiteral("activity.updated"), item->to_json());
            }
            return HttpResponse::json(item->to_json(), 200);
        }
    }

    return HttpResponse::error(QStringLiteral("NOT_FOUND"), QStringLiteral("Atividade não encontrada."), 404);
}

HttpResponse RestApiController::handle_create_dependency(const HttpRequest &req)
{
    HttpResponse err;
    if (!authorize_request(req, UserRole::Engineer, err)) return err;

    QString sched_id = req.query_params.value(QStringLiteral("scheduleId"));
    Dependency dep = Dependency::from_json(req.json_body);

    QString add_err;
    if (!schedule_svc_->add_dependency(sched_id, dep, &add_err)) {
        return HttpResponse::error(QStringLiteral("DEPENDENCY_ERROR"), add_err, 422);
    }

    // Automatically recalculate CPM after adding dependency
    Schedule *s = schedule_svc_->get_schedule(sched_id);
    if (s && planning_svc_) {
        planning_svc_->recalculate(*s);
    }

    if (webhook_dispatcher_) {
        webhook_dispatcher_->dispatch_event(QStringLiteral("dependency.created"), dep.to_json());
    }

    return HttpResponse::json(dep.to_json(), 201);
}

HttpResponse RestApiController::handle_recalculate(const HttpRequest &req)
{
    HttpResponse err;
    if (!authorize_request(req, UserRole::Engineer, err)) return err;

    QString sched_id = req.query_params.value(QStringLiteral("scheduleId"));
    Schedule *s = schedule_svc_->get_schedule(sched_id);
    if (!s) {
        return HttpResponse::error(QStringLiteral("NOT_FOUND"), QStringLiteral("Cronograma não encontrado."), 404);
    }

    CpmEngine::CpmResult cpm = planning_svc_->recalculate(*s);
    if (!cpm.success) {
        return HttpResponse::error(QStringLiteral("CYCLE_DETECTED"), cpm.error_message, 422);
    }

    QJsonObject res_body;
    res_body[QStringLiteral("success")] = true;
    res_body[QStringLiteral("total_duration_hours")] = cpm.total_duration_hours;
    res_body[QStringLiteral("critical_activities_count")] = cpm.critical_activities_count;
    res_body[QStringLiteral("project_early_start")] = cpm.project_early_start.toString(Qt::ISODate);
    res_body[QStringLiteral("project_early_finish")] = cpm.project_early_finish.toString(Qt::ISODate);

    QJsonArray crit_arr;
    for (const QString &id : cpm.critical_path_ids) {
        crit_arr.append(id);
    }
    res_body[QStringLiteral("critical_path_ids")] = crit_arr;

    if (webhook_dispatcher_) {
        webhook_dispatcher_->dispatch_event(QStringLiteral("schedule.recalculated"), res_body);
    }

    return HttpResponse::json(res_body, 200);
}

HttpResponse RestApiController::handle_update_progress(const HttpRequest &req)
{
    HttpResponse err;
    if (!authorize_request(req, UserRole::Engineer, err)) return err;

    QString item_id = req.query_params.value(QStringLiteral("itemId"));
    double percent = req.json_body.value(QStringLiteral("percent_complete")).toDouble(-1.0);
    QString notes = req.json_body.value(QStringLiteral("notes")).toString();
    QString reporter = req.json_body.value(QStringLiteral("reported_by")).toString(QStringLiteral("api"));

    if (percent < 0.0 || percent > 100.0) {
        return HttpResponse::error(QStringLiteral("VALIDATION_ERROR"), QStringLiteral("Percentual de conclusão inválido (0-100)."), 422);
    }

    for (auto &sched : schedule_svc_->list_schedules()) {
        Schedule *s = schedule_svc_->get_schedule(sched.id);
        if (s->find_item(item_id)) {
            QString prog_err;
            if (!progress_svc_->record_progress(*s, item_id, percent, notes, reporter, &prog_err)) {
                return HttpResponse::error(QStringLiteral("PROGRESS_ERROR"), prog_err, 422);
            }

            if (webhook_dispatcher_) {
                QJsonObject event_data;
                event_data[QStringLiteral("item_id")] = item_id;
                event_data[QStringLiteral("percent_complete")] = percent;
                webhook_dispatcher_->dispatch_event(QStringLiteral("activity.progress_updated"), event_data);
            }

            return HttpResponse::json(s->find_item(item_id)->to_json(), 200);
        }
    }

    return HttpResponse::error(QStringLiteral("NOT_FOUND"), QStringLiteral("Atividade não encontrada."), 404);
}

HttpResponse RestApiController::handle_create_baseline(const HttpRequest &req)
{
    HttpResponse err;
    if (!authorize_request(req, UserRole::Engineer, err)) return err;

    QString sched_id = req.query_params.value(QStringLiteral("scheduleId"));
    QString name = req.json_body.value(QStringLiteral("name")).toString(QStringLiteral("Baseline Inicial"));
    QString author = req.json_body.value(QStringLiteral("created_by")).toString(QStringLiteral("engenheiro"));

    Schedule *s = schedule_svc_->get_schedule(sched_id);
    if (!s) {
        return HttpResponse::error(QStringLiteral("NOT_FOUND"), QStringLiteral("Cronograma não encontrado."), 404);
    }

    Baseline b = planning_svc_->create_baseline(*s, name, author);

    if (webhook_dispatcher_) {
        webhook_dispatcher_->dispatch_event(QStringLiteral("baseline.created"), b.to_json());
    }

    return HttpResponse::json(b.to_json(), 201);
}

HttpResponse RestApiController::handle_import_csv(const HttpRequest &req)
{
    HttpResponse err;
    if (!authorize_request(req, UserRole::Engineer, err)) return err;

    QString sched_id = req.query_params.value(QStringLiteral("scheduleId"));
    if (sched_id.isEmpty()) {
        sched_id = req.json_body.value(QStringLiteral("schedule_id")).toString();
    }

    Schedule *s = schedule_svc_->get_schedule(sched_id);
    if (!s) {
        return HttpResponse::error(QStringLiteral("NOT_FOUND"), QStringLiteral("Cronograma não encontrado para importação."), 404);
    }

    QString csv_content = req.json_body.value(QStringLiteral("csv_content")).toString();
    if (csv_content.isEmpty()) {
        csv_content = QString::fromUtf8(req.body);
    }

    CsvImportResult result = csv_svc_->import_from_csv_string(*s, csv_content);

    QJsonObject res_body;
    res_body[QStringLiteral("success")] = result.success;
    res_body[QStringLiteral("items_imported")] = result.items_imported;
    res_body[QStringLiteral("dependencies_imported")] = result.dependencies_imported;
    res_body[QStringLiteral("bim_links_imported")] = result.bim_links_imported;

    QJsonArray err_arr;
    for (const auto &m : result.error_messages) err_arr.append(m);
    res_body[QStringLiteral("errors")] = err_arr;

    QJsonArray warn_arr;
    for (const auto &w : result.warning_messages) warn_arr.append(w);
    res_body[QStringLiteral("warnings")] = warn_arr;

    return HttpResponse::json(res_body, result.success ? 200 : 422);
}

HttpResponse RestApiController::handle_export_csv(const HttpRequest &req)
{
    HttpResponse err;
    if (!authorize_request(req, UserRole::Viewer, err)) return err;

    QString sched_id = req.query_params.value(QStringLiteral("scheduleId"));
    Schedule *s = schedule_svc_->get_schedule(sched_id);
    if (!s) {
        return HttpResponse::error(QStringLiteral("NOT_FOUND"), QStringLiteral("Cronograma não encontrado."), 404);
    }

    QString csv = csv_svc_->export_to_csv_string(*s);
    HttpResponse res;
    res.status_code = 200;
    res.status_text = QStringLiteral("OK");
    res.headers[QStringLiteral("Content-Type")] = QStringLiteral("text/csv; charset=utf-8");
    res.headers[QStringLiteral("Content-Disposition")] = QStringLiteral("attachment; filename=\"cronograma_%1.csv\"").arg(s->name);
    res.body = csv.toUtf8();
    return res;
}

HttpResponse RestApiController::handle_bim_simulation(const HttpRequest &req)
{
    HttpResponse err;
    if (!authorize_request(req, UserRole::Viewer, err)) return err;

    QString sched_id = req.query_params.value(QStringLiteral("scheduleId"));
    Schedule *s = schedule_svc_->get_schedule(sched_id);
    if (!s) {
        return HttpResponse::error(QStringLiteral("NOT_FOUND"), QStringLiteral("Cronograma não encontrado."), 404);
    }

    QString date_str = req.query_params.value(QStringLiteral("referenceDate"));
    QDateTime ref_date = date_str.isEmpty() ? QDateTime::currentDateTime() : QDateTime::fromString(date_str, Qt::ISODate);

    Bim4DSimulationReport rep = bim_svc_->simulate(*s, ref_date);
    return HttpResponse::json(rep.to_json(), 200);
}

HttpResponse RestApiController::handle_create_bim_link(const HttpRequest &req)
{
    HttpResponse err;
    if (!authorize_request(req, UserRole::Engineer, err)) return err;

    QString sched_id = req.query_params.value(QStringLiteral("scheduleId"));
    Schedule *s = schedule_svc_->get_schedule(sched_id);
    if (!s) {
        return HttpResponse::error(QStringLiteral("NOT_FOUND"), QStringLiteral("Cronograma não encontrado."), 404);
    }

    BimLink link = BimLink::from_json(req.json_body);
    QString link_err;
    if (!bim_svc_->add_link(*s, link, &link_err)) {
        return HttpResponse::error(QStringLiteral("VALIDATION_ERROR"), link_err, 422);
    }

    return HttpResponse::json(link.to_json(), 201);
}

HttpResponse RestApiController::handle_openapi_spec(const HttpRequest &req)
{
    Q_UNUSED(req);
    QJsonObject openapi;
    openapi[QStringLiteral("openapi")] = QStringLiteral("3.0.3");

    QJsonObject info;
    info[QStringLiteral("title")] = QStringLiteral("EisenPo Construction Scheduling & 4D BIM API");
    info[QStringLiteral("version")] = QStringLiteral("1.0.0");
    info[QStringLiteral("description")] = QStringLiteral("API REST para gestão de cronogramas de obras, cálculo de caminho crítico (CPM), baselines e integração 4D BIM.");
    openapi[QStringLiteral("info")] = info;

    QJsonObject paths;

    // Add path summaries
    paths[QStringLiteral("/api/v1/projects/{projectId}/schedules")] = QJsonObject{
        {QStringLiteral("post"), QJsonObject{{QStringLiteral("summary"), QStringLiteral("Cria um novo cronograma de obra")}}}
    };
    paths[QStringLiteral("/api/v1/schedules/{scheduleId}/recalculate")] = QJsonObject{
        {QStringLiteral("post"), QJsonObject{{QStringLiteral("summary"), QStringLiteral("Recalcula datas CPM, folgas e caminho crítico")}}}
    };
    paths[QStringLiteral("/api/v1/schedules/{scheduleId}/bim-simulation")] = QJsonObject{
        {QStringLiteral("get"), QJsonObject{{QStringLiteral("summary"), QStringLiteral("Simulação temporal 4D por data de referência")}}}
    };

    openapi[QStringLiteral("paths")] = paths;
    return HttpResponse::json(openapi, 200);
}

} // namespace source::gantt
