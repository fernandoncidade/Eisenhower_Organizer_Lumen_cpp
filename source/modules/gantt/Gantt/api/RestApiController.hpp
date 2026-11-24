#pragma once

#include <QObject>
#include "HttpServer.hpp"
#include "../application/services/Bim4DService.hpp"
#include "../application/services/CsvExchangeService.hpp"
#include "../application/services/FieldProgressService.hpp"
#include "../application/services/PlanningEngineService.hpp"
#include "../application/services/ScheduleService.hpp"
#include "../application/services/SecurityService.hpp"
#include "../application/services/WebhookDispatcher.hpp"

namespace source::gantt {

class RestApiController : public QObject {
    Q_OBJECT

public:
    RestApiController(ScheduleService *schedule_service,
                      PlanningEngineService *planning_service,
                      FieldProgressService *progress_service,
                      Bim4DService *bim_service,
                      CsvExchangeService *csv_service,
                      SecurityService *security_service,
                      WebhookDispatcher *webhook_dispatcher,
                      HttpServer *server,
                      QObject *parent = nullptr);

    void register_endpoints();

private:
    ScheduleService *schedule_svc_;
    PlanningEngineService *planning_svc_;
    FieldProgressService *progress_svc_;
    Bim4DService *bim_svc_;
    CsvExchangeService *csv_svc_;
    SecurityService *security_svc_;
    WebhookDispatcher *webhook_dispatcher_;
    HttpServer *server_;

    // Auth & Rate limit guard
    bool authorize_request(const HttpRequest &req, UserRole min_role, HttpResponse &out_error);

    // Handlers
    HttpResponse handle_create_schedule(const HttpRequest &req);
    HttpResponse handle_get_schedule(const HttpRequest &req);
    HttpResponse handle_create_item(const HttpRequest &req);
    HttpResponse handle_patch_item(const HttpRequest &req);
    HttpResponse handle_create_dependency(const HttpRequest &req);
    HttpResponse handle_recalculate(const HttpRequest &req);
    HttpResponse handle_update_progress(const HttpRequest &req);
    HttpResponse handle_create_baseline(const HttpRequest &req);
    HttpResponse handle_import_csv(const HttpRequest &req);
    HttpResponse handle_export_csv(const HttpRequest &req);
    HttpResponse handle_bim_simulation(const HttpRequest &req);
    HttpResponse handle_create_bim_link(const HttpRequest &req);
    HttpResponse handle_openapi_spec(const HttpRequest &req);
};

} // namespace source::gantt
