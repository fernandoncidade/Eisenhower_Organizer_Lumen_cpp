#pragma once

#include "source/modules/audiolivro/metodoLeitor/lt_07_EdgeTTSClient.hpp"

#include <QString>
#include <QStringList>

namespace source::plugin::Audiolivro::EdgeVoiceCandidates {

inline bool is_multilingual_voice(const QString &voice)
{
    return voice.contains(QStringLiteral("Multilingual"), Qt::CaseInsensitive);
}

inline void append_voice_candidate(QStringList &voices, const QString &voice)
{
    const QString normalized = voice.trimmed();
    if (normalized.isEmpty() || !EdgeTTSClient::is_edge_voice_id(normalized)) {
        return;
    }

    for (const QString &existing : voices) {
        if (existing.compare(normalized, Qt::CaseInsensitive) == 0) {
            return;
        }
    }
    voices.append(normalized);
}

inline void append_locale_fallbacks(QStringList &voices, const QString &locale)
{
    const QString normalized = locale.trimmed();
    if (normalized.compare(QStringLiteral("pt-BR"), Qt::CaseInsensitive) == 0) {
        append_voice_candidate(voices, QStringLiteral("pt-BR-AntonioNeural"));
        append_voice_candidate(voices, QStringLiteral("pt-BR-FranciscaNeural"));
    } else if (normalized.compare(QStringLiteral("pt-PT"), Qt::CaseInsensitive) == 0) {
        append_voice_candidate(voices, QStringLiteral("pt-PT-DuarteNeural"));
        append_voice_candidate(voices, QStringLiteral("pt-PT-RaquelNeural"));
    } else if (normalized.compare(QStringLiteral("en-US"), Qt::CaseInsensitive) == 0) {
        append_voice_candidate(voices, QStringLiteral("en-US-AriaNeural"));
        append_voice_candidate(voices, QStringLiteral("en-US-JennyNeural"));
        append_voice_candidate(voices, QStringLiteral("en-US-GuyNeural"));
        append_voice_candidate(voices, QStringLiteral("en-US-DavisNeural"));
    } else if (normalized.compare(QStringLiteral("en-GB"), Qt::CaseInsensitive) == 0) {
        append_voice_candidate(voices, QStringLiteral("en-GB-SoniaNeural"));
        append_voice_candidate(voices, QStringLiteral("en-GB-RyanNeural"));
    } else if (normalized.compare(QStringLiteral("en-AU"), Qt::CaseInsensitive) == 0) {
        append_voice_candidate(voices, QStringLiteral("en-AU-NatashaNeural"));
        append_voice_candidate(voices, QStringLiteral("en-AU-WilliamNeural"));
    } else if (normalized.compare(QStringLiteral("es-ES"), Qt::CaseInsensitive) == 0) {
        append_voice_candidate(voices, QStringLiteral("es-ES-ElviraNeural"));
        append_voice_candidate(voices, QStringLiteral("es-ES-AlvaroNeural"));
    } else if (normalized.compare(QStringLiteral("es-MX"), Qt::CaseInsensitive) == 0) {
        append_voice_candidate(voices, QStringLiteral("es-MX-DaliaNeural"));
        append_voice_candidate(voices, QStringLiteral("es-MX-JorgeNeural"));
    } else if (normalized.compare(QStringLiteral("fr-FR"), Qt::CaseInsensitive) == 0) {
        append_voice_candidate(voices, QStringLiteral("fr-FR-DeniseNeural"));
        append_voice_candidate(voices, QStringLiteral("fr-FR-HenriNeural"));
    } else if (normalized.compare(QStringLiteral("fr-CA"), Qt::CaseInsensitive) == 0) {
        append_voice_candidate(voices, QStringLiteral("fr-CA-SylvieNeural"));
        append_voice_candidate(voices, QStringLiteral("fr-CA-JeanNeural"));
    } else if (normalized.compare(QStringLiteral("de-DE"), Qt::CaseInsensitive) == 0) {
        append_voice_candidate(voices, QStringLiteral("de-DE-KatjaNeural"));
        append_voice_candidate(voices, QStringLiteral("de-DE-ConradNeural"));
    } else if (normalized.compare(QStringLiteral("it-IT"), Qt::CaseInsensitive) == 0) {
        append_voice_candidate(voices, QStringLiteral("it-IT-ElsaNeural"));
        append_voice_candidate(voices, QStringLiteral("it-IT-DiegoNeural"));
    } else if (normalized.compare(QStringLiteral("ja-JP"), Qt::CaseInsensitive) == 0) {
        append_voice_candidate(voices, QStringLiteral("ja-JP-NanamiNeural"));
        append_voice_candidate(voices, QStringLiteral("ja-JP-KeitaNeural"));
    } else if (normalized.compare(QStringLiteral("ko-KR"), Qt::CaseInsensitive) == 0) {
        append_voice_candidate(voices, QStringLiteral("ko-KR-SunHiNeural"));
        append_voice_candidate(voices, QStringLiteral("ko-KR-InJoonNeural"));
    } else if (normalized.compare(QStringLiteral("zh-CN"), Qt::CaseInsensitive) == 0) {
        append_voice_candidate(voices, QStringLiteral("zh-CN-XiaoxiaoNeural"));
        append_voice_candidate(voices, QStringLiteral("zh-CN-YunxiNeural"));
    } else if (normalized.compare(QStringLiteral("ru-RU"), Qt::CaseInsensitive) == 0) {
        append_voice_candidate(voices, QStringLiteral("ru-RU-SvetlanaNeural"));
        append_voice_candidate(voices, QStringLiteral("ru-RU-DmitryNeural"));
    } else if (normalized.compare(QStringLiteral("ar-SA"), Qt::CaseInsensitive) == 0) {
        append_voice_candidate(voices, QStringLiteral("ar-SA-ZariyahNeural"));
        append_voice_candidate(voices, QStringLiteral("ar-SA-HamedNeural"));
    } else if (normalized.compare(QStringLiteral("hi-IN"), Qt::CaseInsensitive) == 0) {
        append_voice_candidate(voices, QStringLiteral("hi-IN-SwaraNeural"));
        append_voice_candidate(voices, QStringLiteral("hi-IN-MadhurNeural"));
    } else if (normalized.compare(QStringLiteral("nl-NL"), Qt::CaseInsensitive) == 0) {
        append_voice_candidate(voices, QStringLiteral("nl-NL-ColetteNeural"));
        append_voice_candidate(voices, QStringLiteral("nl-NL-MaartenNeural"));
    } else if (normalized.compare(QStringLiteral("pl-PL"), Qt::CaseInsensitive) == 0) {
        append_voice_candidate(voices, QStringLiteral("pl-PL-ZofiaNeural"));
        append_voice_candidate(voices, QStringLiteral("pl-PL-MarekNeural"));
    } else if (normalized.compare(QStringLiteral("tr-TR"), Qt::CaseInsensitive) == 0) {
        append_voice_candidate(voices, QStringLiteral("tr-TR-EmelNeural"));
        append_voice_candidate(voices, QStringLiteral("tr-TR-AhmetNeural"));
    }
}

inline void append_same_locale_fallbacks(QStringList &voices, const QString &selected_voice)
{
    append_locale_fallbacks(voices, EdgeTTSClient::locale_for_voice_id(selected_voice));
}

inline QStringList build_voice_candidates(const QString &selected_voice)
{
    QStringList voices;
    if (!EdgeTTSClient::is_edge_voice_id(selected_voice)) {
        return voices;
    }

    if (is_multilingual_voice(selected_voice)) {
        append_same_locale_fallbacks(voices, selected_voice);
        append_voice_candidate(voices, selected_voice);
    } else {
        append_voice_candidate(voices, selected_voice);
        append_same_locale_fallbacks(voices, selected_voice);
    }
    return voices;
}

}  // namespace source::plugin::Audiolivro::EdgeVoiceCandidates
