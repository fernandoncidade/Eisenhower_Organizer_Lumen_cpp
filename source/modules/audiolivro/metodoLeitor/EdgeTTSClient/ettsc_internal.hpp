#pragma once

#include <utility>

#include <QAbstractSocket>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLocale>
#include <QMap>
#include <QNetworkRequest>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariantList>
#include <QVariantMap>
#include <QtGlobal>

#include "source/modules/audiolivro/metodoLeitor/lt_07_EdgeTTSClient.hpp"

namespace source::plugin::Audiolivro::EdgeTTSClientInternal {


constexpr auto kBaseUrl = "speech.platform.bing.com/consumer/speech/synthesize/readaloud";
constexpr auto kChromiumFullVersion = "143.0.3650.75";
constexpr auto kSecMsGecVersion = "1-143.0.3650.75";
constexpr qint64 kWindowsEpochSeconds = 11644473600LL;
constexpr qint64 kTicksPerSecond = 10000000LL;
constexpr qint64 kTicksPerMillisecond = 10000LL;

inline QByteArray trusted_client_token()
{
    static const QByteArray token = [] {
        const QByteArray override_token = qgetenv("AUDIOLIVRO_EDGE_TTS_CLIENT_TOKEN").trimmed();
        if (!override_token.isEmpty()) {
            return override_token;
        }

        // Public Edge Read Aloud protocol token. It is not an application credential.
        return QByteArrayLiteral("6A5AA1D4EAFF4E9FB37E23D68491D6F4");
    }();
    return token;
}

inline QString trusted_client_token_text()
{
    return QString::fromLatin1(trusted_client_token());
}

inline QString chromium_major_version()
{
    return QString::fromLatin1(kChromiumFullVersion).section(QLatin1Char('.'), 0, 0);
}

inline QByteArray user_agent()
{
    const QString major = chromium_major_version();
    return QStringLiteral("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
                          "(KHTML, like Gecko) Chrome/%1.0.0.0 Safari/537.36 Edg/%1.0.0.0")
        .arg(major)
        .toUtf8();
}

inline QString random_hex_128()
{
    const quint64 high = QRandomGenerator::global()->generate64();
    const quint64 low = QRandomGenerator::global()->generate64();
    return QStringLiteral("%1%2")
        .arg(high, 16, 16, QLatin1Char('0'))
        .arg(low, 16, 16, QLatin1Char('0'))
        .toUpper();
}

inline QString connection_id()
{
    return random_hex_128().toLower();
}

inline QString sec_ms_gec()
{
    qint64 seconds = QDateTime::currentSecsSinceEpoch() + kWindowsEpochSeconds;
    seconds -= seconds % 300;
    const QString input = QString::number(seconds * kTicksPerSecond) + trusted_client_token_text();
    return QString::fromLatin1(QCryptographicHash::hash(input.toLatin1(), QCryptographicHash::Sha256).toHex()).toUpper();
}

inline QString edge_timestamp()
{
    return QLocale::c().toString(QDateTime::currentDateTimeUtc(),
                                 QStringLiteral("ddd MMM dd yyyy HH:mm:ss 'GMT+0000 (Coordinated Universal Time)'"));
}

inline QString percent_arg(int value)
{
    return QStringLiteral("%1%2%").arg(value >= 0 ? QStringLiteral("+") : QString(), QString::number(value));
}

inline bool is_letter_or_number_at(const QString &text, int index)
{
    return index >= 0 && index < text.size() && text.at(index).isLetterOrNumber();
}

inline bool is_dash_like(QChar ch)
{
    switch (ch.unicode()) {
    case 0x002D: // hyphen-minus
    case 0x2010: // hyphen
    case 0x2011: // non-breaking hyphen
    case 0x2012: // figure dash
    case 0x2013: // en dash
    case 0x2014: // em dash
    case 0x2015: // horizontal bar
    case 0x2212: // minus sign
    case 0xFE58: // small em dash
    case 0xFE63: // small hyphen-minus
    case 0xFF0D: // fullwidth hyphen-minus
        return true;
    default:
        return false;
    }
}

inline bool is_closing_punctuation(QChar ch)
{
    switch (ch.unicode()) {
    case 0x0029: // )
    case 0x005D: // ]
    case 0x007D: // }
    case 0x2019: // right single quotation mark
    case 0x201D: // right double quotation mark
        return true;
    default:
        return false;
    }
}

inline QChar alnum_before_separator(const QString &text, int index)
{
    while (index >= 0 && is_closing_punctuation(text.at(index))) {
        --index;
    }
    return is_letter_or_number_at(text, index) ? text.at(index) : QChar();
}

inline QChar alnum_after_separator(const QString &text, int index)
{
    return is_letter_or_number_at(text, index) ? text.at(index) : QChar();
}

inline bool is_internal_dot_separator(const QString &text, int index)
{
    if (index < 0 || index >= text.size() || text.at(index) != QLatin1Char('.')) {
        return false;
    }

    const QChar before = alnum_before_separator(text, index - 1);
    const QChar after = alnum_after_separator(text, index + 1);
    if (before.isNull() || after.isNull()) {
        return false;
    }
    return !(before.isDigit() && after.isDigit());
}

inline QString normalize_text_for_edge_tts(QString text)
{
    text = text.normalized(QString::NormalizationForm_KC);

    QString out;
    out.reserve(text.size());
    for (int i = 0; i < text.size(); ++i) {
        const QChar ch = text.at(i);
        switch (ch.unicode()) {
        case 0x00AD: // soft hyphen
        case 0x200B: // zero width space
        case 0x200C: // zero width non-joiner
        case 0x200D: // zero width joiner
        case 0x2060: // word joiner
        case 0xFEFF: // zero width no-break space
            continue;
        case 0x00A0: // no-break space
        case 0x202F: // narrow no-break space
            out += QLatin1Char(' ');
            continue;
        default:
            break;
        }

        if (ch == QLatin1Char('_')) {
            out += QLatin1Char(' ');
            continue;
        }

        if (is_internal_dot_separator(text, i)) {
            out += QLatin1Char(' ');
            continue;
        }

        if (is_dash_like(ch)) {
            const bool joins_word = is_letter_or_number_at(text, i - 1) && is_letter_or_number_at(text, i + 1);
            if (joins_word) {
                out += QLatin1Char(' ');
            } else {
                while (!out.isEmpty() && out.back().isSpace() && out.back() != QLatin1Char('\n')) {
                    out.chop(1);
                }
                out += QStringLiteral(", ");
            }
            continue;
        }

        if (ch.isLowSurrogate() || ch.isHighSurrogate()) {
            out += ch;
            continue;
        }

        if ((ch.category() == QChar::Other_Control || ch.category() == QChar::Other_Format)
            && ch != QLatin1Char('\n') && ch != QLatin1Char('\t')) {
            out += QLatin1Char(' ');
            continue;
        }

        out += ch;
    }

    out.replace(QRegularExpression(QStringLiteral("[ \\t]{2,}")), QStringLiteral(" "));
    out.replace(QRegularExpression(QStringLiteral("\\n{3,}")), QStringLiteral("\n\n"));
    return out.trimmed();
}

inline QString xml_escape_text(QString text)
{
    text.replace(QChar(0x000B), QLatin1Char(' '));
    text.replace(QChar(0x000C), QLatin1Char(' '));
    text.replace(QLatin1Char('&'), QStringLiteral("&amp;"));
    text.replace(QLatin1Char('<'), QStringLiteral("&lt;"));
    text.replace(QLatin1Char('>'), QStringLiteral("&gt;"));
    return text;
}

inline QString xml_escape_attr(QString text)
{
    text = xml_escape_text(std::move(text));
    text.replace(QLatin1Char('"'), QStringLiteral("&quot;"));
    text.replace(QLatin1Char('\''), QStringLiteral("&apos;"));
    return text;
}

inline QString language_for_locale(const QString &locale)
{
    const QLocale loc(locale);
    if (loc.language() != QLocale::C && loc.language() != QLocale::AnyLanguage) {
        const QString native = loc.nativeLanguageName();
        if (!native.trimmed().isEmpty()) {
            QString cap = native.trimmed();
            cap[0] = cap[0].toUpper();
            return cap;
        }
        const QString lang_name = QLocale::languageToString(loc.language());
        if (!lang_name.trimmed().isEmpty()) {
            return lang_name;
        }
    }

    const QString prefix = locale.left(2).toLower();
    if (prefix == QStringLiteral("pt")) return QStringLiteral("Português");
    if (prefix == QStringLiteral("en")) return QStringLiteral("English");
    if (prefix == QStringLiteral("es")) return QStringLiteral("Español");
    if (prefix == QStringLiteral("fr")) return QStringLiteral("Français");
    if (prefix == QStringLiteral("de")) return QStringLiteral("Deutsch");
    if (prefix == QStringLiteral("it")) return QStringLiteral("Italiano");
    if (prefix == QStringLiteral("ja")) return QStringLiteral("日本語");
    if (prefix == QStringLiteral("ko")) return QStringLiteral("한국어");
    if (prefix == QStringLiteral("zh")) return QStringLiteral("中文");
    if (prefix == QStringLiteral("ru")) return QStringLiteral("Русский");
    if (prefix == QStringLiteral("ar")) return QStringLiteral("العربية");
    if (prefix == QStringLiteral("hi")) return QStringLiteral("हिन्दी");
    if (prefix == QStringLiteral("nl")) return QStringLiteral("Nederlands");
    if (prefix == QStringLiteral("pl")) return QStringLiteral("Polski");
    if (prefix == QStringLiteral("tr")) return QStringLiteral("Türkçe");
    return locale;
}

inline QString friendly_name_from_id(const QString &id)
{
    const QStringList parts = id.split(QLatin1Char('-'), Qt::SkipEmptyParts);
    QString name = parts.size() >= 3 ? parts.at(2) : id;
    name.remove(QStringLiteral("Neural"), Qt::CaseInsensitive);
    return name;
}

inline QString accept_language_header(const QString &locale)
{
    const QString normalized = locale.trimmed().isEmpty() ? QStringLiteral("pt-BR") : locale.trimmed();
    const QString language = normalized.left(2).toLower();
    if (language.isEmpty()) {
        return QStringLiteral("pt-BR,pt;q=0.9,en;q=0.3");
    }
    return QStringLiteral("%1,%2;q=0.9,en;q=0.3").arg(normalized, language);
}

inline QString speech_config_message()
{
    return QStringLiteral(
               "X-Timestamp:%1\r\n"
               "Content-Type:application/json; charset=utf-8\r\n"
               "Path:speech.config\r\n\r\n"
               "{\"context\":{\"synthesis\":{\"audio\":{\"metadataoptions\":{"
               "\"sentenceBoundaryEnabled\":\"false\",\"wordBoundaryEnabled\":\"true\"},"
               "\"outputFormat\":\"audio-24khz-48kbitrate-mono-mp3\"}}}}\r\n")
        .arg(edge_timestamp());
}

inline QString ssml_message(const QString &text, const QString &voice_id, int rate_pct, int volume_pct)
{
    const QString voice = EdgeTTSClient::normalize_edge_voice_name(voice_id);
    const QString locale = EdgeTTSClient::locale_for_voice_id(voice_id);
    const QString ssml = QStringLiteral(
                             "<speak version='1.0' xmlns='http://www.w3.org/2001/10/synthesis' xml:lang='%2'>"
                             "<voice name=\"%1\" xml:lang=\"%2\"><prosody xml:lang=\"%2\" pitch=\"+0Hz\" rate=\"%3\" volume=\"%4\">%5</prosody></voice>"
                             "</speak>")
                             .arg(xml_escape_attr(voice),
                                  xml_escape_attr(locale),
                                  percent_arg(rate_pct),
                                  percent_arg(volume_pct),
                                  xml_escape_text(text));

    return QStringLiteral(
               "X-RequestId:%1\r\n"
               "Content-Type:application/ssml+xml\r\n"
               "X-Timestamp:%2Z\r\n"
               "Path:ssml\r\n\r\n%3")
        .arg(connection_id(), edge_timestamp(), ssml);
}

inline QUrl websocket_url()
{
    return QUrl(QStringLiteral("wss://%1/edge/v1?TrustedClientToken=%2&ConnectionId=%3&Sec-MS-GEC=%4&Sec-MS-GEC-Version=%5")
                    .arg(QString::fromLatin1(kBaseUrl),
                         trusted_client_token_text(),
                         connection_id(),
                         sec_ms_gec(),
                         QString::fromLatin1(kSecMsGecVersion)));
}

inline void apply_common_headers(QNetworkRequest *request, const QString &locale = QStringLiteral("pt-BR"))
{
    request->setRawHeader("User-Agent", user_agent());
    request->setRawHeader("Accept-Encoding", "gzip, deflate, br, zstd");
    request->setRawHeader("Accept-Language", accept_language_header(locale).toUtf8());
}

inline QNetworkRequest websocket_request(const QString &locale)
{
    QNetworkRequest request(websocket_url());
    apply_common_headers(&request, locale);
    request.setRawHeader("Pragma", "no-cache");
    request.setRawHeader("Cache-Control", "no-cache");
    request.setRawHeader("Origin", "chrome-extension://jdiccldimpdaibmpdkjnbmckianbfold");
    request.setRawHeader("Cookie", QStringLiteral("muid=%1;").arg(random_hex_128()).toUtf8());
    return request;
}

inline QNetworkRequest voice_list_request()
{
    QNetworkRequest request(QUrl(QStringLiteral("https://%1/voices/list?trustedclienttoken=%2")
                                     .arg(QString::fromLatin1(kBaseUrl), trusted_client_token_text())));
    apply_common_headers(&request, QStringLiteral("pt-BR"));
    request.setRawHeader("Authority", "speech.platform.bing.com");
    request.setRawHeader("Accept", "*/*");
    request.setRawHeader("Sec-Fetch-Site", "none");
    request.setRawHeader("Sec-Fetch-Mode", "cors");
    request.setRawHeader("Sec-Fetch-Dest", "empty");
    return request;
}

inline QMap<QByteArray, QByteArray> parse_headers(const QByteArray &header_bytes)
{
    QMap<QByteArray, QByteArray> headers;
    for (const QByteArray &line : header_bytes.split('\n')) {
        const QByteArray trimmed = line.trimmed();
        const int colon = trimmed.indexOf(':');
        if (colon <= 0) {
            continue;
        }
        headers.insert(trimmed.left(colon), trimmed.mid(colon + 1).trimmed());
    }
    return headers;
}

inline QByteArray binary_audio_payload(const QByteArray &message)
{
    if (message.size() < 2) {
        return {};
    }

    const int header_length = (quint8(message.at(0)) << 8) | quint8(message.at(1));
    int header_start = 2;
    int payload_start = 2 + header_length;
    if (payload_start > message.size()) {
        header_start = 0;
        payload_start = header_length + 2;
    }
    if (payload_start > message.size()) {
        return {};
    }

    const QByteArray headers_raw = message.mid(header_start, payload_start - header_start);
    const QMap<QByteArray, QByteArray> headers = parse_headers(headers_raw);
    if (headers.value("Path") != "audio") {
        return {};
    }

    QByteArray payload = message.mid(payload_start);
    while (payload.startsWith("\r\n")) {
        payload.remove(0, 2);
    }
    return payload;
}

inline qint64 json_int64(const QJsonValue &value, qint64 fallback = 0)
{
    if (value.isDouble()) {
        return qRound64(value.toDouble());
    }
    if (value.isString()) {
        bool ok = false;
        const qint64 parsed = value.toString().toLongLong(&ok);
        if (ok) {
            return parsed;
        }
    }
    return fallback;
}

inline qint64 ticks_to_ms(qint64 ticks)
{
    if (ticks <= 0) {
        return 0;
    }
    return qMax<qint64>(1, qRound64(double(ticks) / double(kTicksPerMillisecond)));
}

inline int resolve_text_offset(const QString &source, const QString &word, int suggested_offset, int last_offset)
{
    Q_UNUSED(word)
    Q_UNUSED(last_offset)
    return qBound(0, suggested_offset, source.size());
}

inline void append_metadata_timestamps(const QByteArray &body, const QString &source_text, QVariantList *timestamps)
{
    if (timestamps == nullptr || body.trimmed().isEmpty()) {
        return;
    }

    const QJsonDocument document = QJsonDocument::fromJson(body);
    if (!document.isObject()) {
        return;
    }

    const QJsonArray metadata = document.object().value(QStringLiteral("Metadata")).toArray();
    int last_text_offset = timestamps->isEmpty()
                               ? 0
                               : timestamps->constLast().toMap().value(QStringLiteral("text_offset")).toInt();

    for (const QJsonValue &entry_value : metadata) {
        const QJsonObject entry = entry_value.toObject();
        if (entry.value(QStringLiteral("Type")).toString().compare(QStringLiteral("WordBoundary"), Qt::CaseInsensitive) != 0) {
            continue;
        }

        const QJsonObject data = entry.value(QStringLiteral("Data")).toObject();
        QJsonObject text_info = data.value(QStringLiteral("text")).toObject();
        if (text_info.isEmpty()) {
            text_info = data.value(QStringLiteral("Text")).toObject();
        }

        QString word = text_info.value(QStringLiteral("Text")).toString();
        if (word.isEmpty()) {
            word = data.value(QStringLiteral("Text")).toString();
        }
        if (word.trimmed().isEmpty()) {
            continue;
        }

        const int suggested_offset = int(json_int64(text_info.value(QStringLiteral("Offset")), last_text_offset));
        const int text_offset = resolve_text_offset(source_text, word, suggested_offset, last_text_offset);
        const qint64 offset_ms = ticks_to_ms(json_int64(data.value(QStringLiteral("Offset"))));
        qint64 duration_ms = ticks_to_ms(json_int64(data.value(QStringLiteral("Duration"))));
        if (duration_ms <= 0) {
            duration_ms = 120;
        }

        QVariantMap timestamp;
        timestamp.insert(QStringLiteral("text"), word);
        timestamp.insert(QStringLiteral("offset_ms"), offset_ms);
        timestamp.insert(QStringLiteral("duration_ms"), duration_ms);
        timestamp.insert(QStringLiteral("raw_text_offset"), qBound(0, suggested_offset, source_text.size()));
        timestamp.insert(QStringLiteral("text_offset"), text_offset);
        timestamps->append(timestamp);
        last_text_offset = text_offset + word.size();
    }
}

}  // namespace source::plugin::Audiolivro::EdgeTTSClientInternal
