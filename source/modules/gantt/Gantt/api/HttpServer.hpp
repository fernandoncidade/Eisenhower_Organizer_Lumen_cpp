#pragma once

#include <QByteArray>
#include <QHash>
#include <QJsonObject>
#include <QObject>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>
#include <functional>

namespace source::gantt {

struct HttpRequest {
    QString method;
    QString path;
    QHash<QString, QString> query_params;
    QHash<QString, QString> headers;
    QByteArray body;
    QJsonObject json_body;
    QString client_ip;
    QString auth_token;
};

struct HttpResponse {
    int status_code = 200;
    QString status_text = QStringLiteral("OK");
    QHash<QString, QString> headers;
    QByteArray body;

    static HttpResponse json(const QJsonObject &obj, int status = 200);
    static HttpResponse error(const QString &code, const QString &message, int status = 400, const QJsonObject &details = QJsonObject());
};

using HttpRouteHandler = std::function<HttpResponse(const HttpRequest &req)>;

class HttpServer : public QObject {
    Q_OBJECT

public:
    explicit HttpServer(QObject *parent = nullptr);
    ~HttpServer() override;

    bool start(quint16 port = 8085, const QString &address = QStringLiteral("0.0.0.0"));
    void stop();
    bool is_listening() const;
    quint16 server_port() const;

    // Routing registration
    void add_route(const QString &method, const QString &path_pattern, HttpRouteHandler handler);

signals:
    void request_received(const QString &method, const QString &path, int status_code);

private slots:
    void on_new_connection();
    void on_ready_read();
    void on_disconnected();

private:
    QTcpServer *server_ = nullptr;
    struct Route {
        QString method;
        QString pattern;
        HttpRouteHandler handler;
    };
    QList<Route> routes_;

    void handle_client(QTcpSocket *socket);
    HttpResponse dispatch(const HttpRequest &req);
    bool match_route(const QString &pattern, const QString &actual_path, QHash<QString, QString> &out_params) const;
};

} // namespace source::gantt
