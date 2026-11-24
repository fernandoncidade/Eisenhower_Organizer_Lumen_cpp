#include "HttpServer.hpp"
#include <QHostAddress>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QUrl>
#include <QUrlQuery>

namespace source::gantt {

HttpResponse HttpResponse::json(const QJsonObject &obj, int status)
{
    HttpResponse res;
    res.status_code = status;
    switch (status) {
        case 200: res.status_text = QStringLiteral("OK"); break;
        case 201: res.status_text = QStringLiteral("Created"); break;
        case 204: res.status_text = QStringLiteral("No Content"); break;
        case 400: res.status_text = QStringLiteral("Bad Request"); break;
        case 401: res.status_text = QStringLiteral("Unauthorized"); break;
        case 403: res.status_text = QStringLiteral("Forbidden"); break;
        case 404: res.status_text = QStringLiteral("Not Found"); break;
        case 422: res.status_text = QStringLiteral("Unprocessable Entity"); break;
        case 429: res.status_text = QStringLiteral("Too Many Requests"); break;
        default:  res.status_text = QStringLiteral("Internal Server Error"); break;
    }
    res.headers[QStringLiteral("Content-Type")] = QStringLiteral("application/json; charset=utf-8");
    res.body = QJsonDocument(obj).toJson(QJsonDocument::Compact);
    return res;
}

HttpResponse HttpResponse::error(const QString &code, const QString &message, int status, const QJsonObject &details)
{
    QJsonObject root;
    QJsonObject err;
    err[QStringLiteral("code")] = code;
    err[QStringLiteral("message")] = message;
    if (!details.isEmpty()) {
        err[QStringLiteral("details")] = details;
    }
    root[QStringLiteral("error")] = err;
    return json(root, status);
}

HttpServer::HttpServer(QObject *parent)
    : QObject(parent),
      server_(new QTcpServer(this))
{
    connect(server_, &QTcpServer::newConnection, this, &HttpServer::on_new_connection);
}

HttpServer::~HttpServer()
{
    stop();
}

bool HttpServer::start(quint16 port, const QString &address)
{
    stop();
    QHostAddress host = (address == QStringLiteral("0.0.0.0")) ? QHostAddress::Any : QHostAddress(address);
    return server_->listen(host, port);
}

void HttpServer::stop()
{
    if (server_->isListening()) {
        server_->close();
    }
}

bool HttpServer::is_listening() const
{
    return server_->isListening();
}

quint16 HttpServer::server_port() const
{
    return server_->serverPort();
}

void HttpServer::add_route(const QString &method, const QString &path_pattern, HttpRouteHandler handler)
{
    routes_.append({method.toUpper(), path_pattern, handler});
}

void HttpServer::on_new_connection()
{
    while (server_->hasPendingConnections()) {
        QTcpSocket *socket = server_->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &HttpServer::on_ready_read);
        connect(socket, &QTcpSocket::disconnected, this, &HttpServer::on_disconnected);
    }
}

void HttpServer::on_ready_read()
{
    auto *socket = qobject_cast<QTcpSocket *>(sender());
    if (!socket) return;

    handle_client(socket);
}

void HttpServer::on_disconnected()
{
    auto *socket = qobject_cast<QTcpSocket *>(sender());
    if (socket) {
        socket->deleteLater();
    }
}

bool HttpServer::match_route(const QString &pattern, const QString &actual_path, QHash<QString, QString> &out_params) const
{
    QStringList pat_parts = pattern.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    QStringList act_parts = actual_path.split(QLatin1Char('/'), Qt::SkipEmptyParts);

    if (pat_parts.size() != act_parts.size()) {
        return false;
    }

    out_params.clear();
    for (int i = 0; i < pat_parts.size(); ++i) {
        const QString &p = pat_parts[i];
        const QString &a = act_parts[i];
        if (p.startsWith(QLatin1Char('{')) && p.endsWith(QLatin1Char('}'))) {
            QString param_name = p.mid(1, p.size() - 2);
            out_params.insert(param_name, a);
        } else if (p.compare(a, Qt::CaseInsensitive) != 0) {
            return false;
        }
    }

    return true;
}

void HttpServer::handle_client(QTcpSocket *socket)
{
    QByteArray raw_data = socket->readAll();
    if (raw_data.isEmpty()) return;

    int header_end = raw_data.indexOf("\r\n\r\n");
    if (header_end == -1) {
        header_end = raw_data.indexOf("\n\n");
        if (header_end == -1) return;
    }

    QByteArray header_part = raw_data.left(header_end);
    QByteArray body_part = raw_data.mid(header_end + ((raw_data.contains("\r\n\r\n")) ? 4 : 2));

    QString header_text = QString::fromUtf8(header_part);
    QStringList header_lines = header_text.split(QRegularExpression(QStringLiteral("[\r\n]+")), Qt::SkipEmptyParts);
    if (header_lines.isEmpty()) return;

    // Request line: METHOD PATH HTTP/1.1
    QString request_line = header_lines.takeFirst();
    QStringList req_tokens = request_line.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    if (req_tokens.size() < 2) return;

    HttpRequest req;
    req.method = req_tokens[0].toUpper();
    req.client_ip = socket->peerAddress().toString();

    // Parse URL & Query params
    QUrl url(req_tokens[1]);
    req.path = url.path();
    QUrlQuery query(url.query());
    for (const auto &item : query.queryItems()) {
        req.query_params.insert(item.first, item.second);
    }

    // Parse headers
    for (const QString &line : header_lines) {
        int colon = line.indexOf(QLatin1Char(':'));
        if (colon != -1) {
            QString name = line.left(colon).trimmed().toLower();
            QString value = line.mid(colon + 1).trimmed();
            req.headers.insert(name, value);
        }
    }

    // Extract Bearer token
    if (req.headers.contains(QStringLiteral("authorization"))) {
        QString auth = req.headers[QStringLiteral("authorization")];
        if (auth.startsWith(QStringLiteral("Bearer "), Qt::CaseInsensitive)) {
            req.auth_token = auth.mid(7).trimmed();
        }
    }

    // Body
    req.body = body_part;
    if (!req.body.isEmpty()) {
        QJsonParseError parse_err;
        QJsonDocument doc = QJsonDocument::fromJson(req.body, &parse_err);
        if (parse_err.error == QJsonParseError::NoError && doc.isObject()) {
            req.json_body = doc.object();
        }
    }

    // Handle CORS Pre-flight
    HttpResponse res;
    if (req.method == QStringLiteral("OPTIONS")) {
        res.status_code = 204;
        res.status_text = QStringLiteral("No Content");
    } else {
        res = dispatch(req);
    }

    emit request_received(req.method, req.path, res.status_code);

    // Build HTTP Response message
    QByteArray response_bytes;
    response_bytes.append(QStringLiteral("HTTP/1.1 %1 %2\r\n").arg(res.status_code).arg(res.status_text).toUtf8());
    response_bytes.append("Access-Control-Allow-Origin: *\r\n");
    response_bytes.append("Access-Control-Allow-Methods: GET, POST, PATCH, PUT, DELETE, OPTIONS\r\n");
    response_bytes.append("Access-Control-Allow-Headers: Authorization, Content-Type, X-Idempotency-Key\r\n");
    response_bytes.append("Connection: close\r\n");

    for (auto it = res.headers.begin(); it != res.headers.end(); ++it) {
        response_bytes.append(QStringLiteral("%1: %2\r\n").arg(it.key(), it.value()).toUtf8());
    }

    response_bytes.append(QStringLiteral("Content-Length: %1\r\n\r\n").arg(res.body.size()).toUtf8());
    response_bytes.append(res.body);

    socket->write(response_bytes);
    socket->flush();
    socket->disconnectFromHost();
}

HttpResponse HttpServer::dispatch(const HttpRequest &req)
{
    for (const auto &route : routes_) {
        if (route.method != req.method) continue;

        QHash<QString, QString> path_params;
        if (match_route(route.pattern, req.path, path_params)) {
            // Merge path params into query params for uniform handler access
            HttpRequest enriched = req;
            for (auto it = path_params.begin(); it != path_params.end(); ++it) {
                enriched.query_params.insert(it.key(), it.value());
            }
            return route.handler(enriched);
        }
    }

    return HttpResponse::error(
        QStringLiteral("NOT_FOUND"),
        QStringLiteral("Endpoint não encontrado: %1 %2").arg(req.method, req.path),
        404
    );
}

} // namespace source::gantt
