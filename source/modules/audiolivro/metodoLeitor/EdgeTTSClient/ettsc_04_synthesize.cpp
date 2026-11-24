#include "source/modules/audiolivro/metodoLeitor/EdgeTTSClient/ettsc_04_synthesize.hpp"

#include "source/modules/audiolivro/metodoLeitor/EdgeTTSClient/ettsc_internal.hpp"

#include <QCoreApplication>
#include <QEventLoop>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QTimer>
#include <QWebSocket>

namespace source::plugin::Audiolivro {
using namespace EdgeTTSClientInternal;

EdgeTTSResult EdgeTTSClient::synthesize(const QString &text,
                                        const QString &voice_id,
                                        int rate_pct,
                                        int volume_pct,
                                        int timeout_ms)
{
    EdgeTTSResult result;
    if (text.trimmed().isEmpty()) {
        result.error = QCoreApplication::translate("EdgeTTSClient", "Texto vazio.");
        return result;
    }
    const QString synthesis_text = normalize_text_for_edge_tts(text);
    if (synthesis_text.trimmed().isEmpty()) {
        result.error = QCoreApplication::translate("EdgeTTSClient", "Texto vazio.");
        return result;
    }
    if (!is_edge_voice_id(voice_id)) {
        result.error = QCoreApplication::translate("EdgeTTSClient",
                                                   "A voz selecionada nao e uma voz Edge TTS.");
        return result;
    }

    QWebSocket socket;
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    bool turn_end = false;
    QString error_text;
    QByteArray audio;
    QVariantList timestamps;

    QObject::connect(&timer, &QTimer::timeout, &loop, [&] {
        error_text = QCoreApplication::translate("EdgeTTSClient",
                                                 "Tempo esgotado ao conectar ao Edge TTS.");
        socket.abort();
        loop.quit();
    });
    QObject::connect(&socket, &QWebSocket::connected, &loop, [&] {
        socket.sendTextMessage(speech_config_message());
        socket.sendTextMessage(ssml_message(synthesis_text, voice_id, qBound(-50, rate_pct, 50), qBound(-100, volume_pct, 100)));
    });
    QObject::connect(&socket, &QWebSocket::textMessageReceived, &loop, [&](const QString &message) {
        const QByteArray data = message.toUtf8();
        const int header_end = data.indexOf("\r\n\r\n");
        const QMap<QByteArray, QByteArray> headers = parse_headers(header_end >= 0 ? data.left(header_end) : data);
        const QByteArray path = headers.value("Path");
        if (path == "audio.metadata" && header_end >= 0) {
            append_metadata_timestamps(data.mid(header_end + 4), synthesis_text, &timestamps);
            return;
        }
        if (path == "turn.end") {
            turn_end = true;
            socket.close();
            loop.quit();
        }
    });
    QObject::connect(&socket, &QWebSocket::binaryMessageReceived, &loop, [&](const QByteArray &message) {
        const QByteArray payload = binary_audio_payload(message);
        if (!payload.isEmpty()) {
            audio.append(payload);
        }
    });
    QObject::connect(&socket, &QWebSocket::errorOccurred, &loop, [&](QAbstractSocket::SocketError) {
        error_text = socket.errorString();
        loop.quit();
    });
    QObject::connect(&socket, &QWebSocket::disconnected, &loop, [&] {
        if (!turn_end && error_text.isEmpty()) {
            error_text = socket.errorString().isEmpty() ? QCoreApplication::translate("EdgeTTSClient",
                                                                                      "Conexao com Edge TTS encerrada antes do fim.")
                                                       : socket.errorString();
        }
        loop.quit();
    });

    timer.start(timeout_ms);
    socket.open(websocket_request(locale_for_voice_id(voice_id)));
    loop.exec();
    timer.stop();

    if (!turn_end && !error_text.isEmpty()) {
        result.error = error_text;
        return result;
    }
    if (audio.isEmpty()) {
        result.error = QCoreApplication::translate("EdgeTTSClient", "O Edge TTS nao retornou audio.");
        return result;
    }

    result.ok = true;
    result.audio = audio;
    result.timestamps = timestamps;
    return result;
}

}  // namespace source::plugin::Audiolivro

namespace source::plugin::Audiolivro::EdgeTTSClientFns {

EdgeTTSResult synthesize(const QString &text, const QString &voice_id, int rate_pct, int volume_pct, int timeout_ms) { return EdgeTTSClient::synthesize(text, voice_id, rate_pct, volume_pct, timeout_ms); }

}  // namespace source::plugin::Audiolivro::EdgeTTSClientFns
