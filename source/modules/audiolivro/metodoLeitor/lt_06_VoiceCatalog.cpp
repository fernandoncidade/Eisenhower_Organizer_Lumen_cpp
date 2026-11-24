#include "source/modules/audiolivro/metodoLeitor/lt_06_VoiceCatalog.hpp"

#include "source/modules/audiolivro/metodoLeitor/lt_07_EdgeTTSClient.hpp"

#include <algorithm>
#include <QLocale>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>
#include <QVariantMap>
#include <utility>

namespace source::plugin::Audiolivro {
namespace {

QVariantMap voice(QString id, QString language, QString locale, QString gender, QString name)
{
    QVariantMap item;
    item.insert(QStringLiteral("id"), std::move(id));
    item.insert(QStringLiteral("language"), std::move(language));
    item.insert(QStringLiteral("locale"), std::move(locale));
    item.insert(QStringLiteral("gender"), std::move(gender));
    item.insert(QStringLiteral("name"), std::move(name));
    return item;
}

QVariantMap voice(QString id, QString language, QString locale, QString gender, QString name, QString engine)
{
    QVariantMap item = voice(std::move(id), std::move(language), std::move(locale), std::move(gender), std::move(name));
    item.insert(QStringLiteral("engine"), std::move(engine));
    return item;
}

QString language_for_locale(const QString &locale)
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

QString friendly_name_from_id(const QString &id)
{
    const QStringList parts = id.split(QLatin1Char('-'), Qt::SkipEmptyParts);
    QString name = parts.size() >= 3 ? parts.at(2) : id;
    name.remove(QStringLiteral("Neural"), Qt::CaseInsensitive);
    return name;
}

QVariantList builtin_voice_options()
{
    return {
        voice(QStringLiteral("pt-BR-AntonioNeural"), QStringLiteral("Portuguese"), QStringLiteral("pt-BR"), QStringLiteral("Masculina"), QStringLiteral("Antonio"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("pt-BR-FranciscaNeural"), QStringLiteral("Portuguese"), QStringLiteral("pt-BR"), QStringLiteral("Feminina"), QStringLiteral("Francisca"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("pt-PT-DuarteNeural"), QStringLiteral("Portuguese"), QStringLiteral("pt-PT"), QStringLiteral("Masculina"), QStringLiteral("Duarte"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("pt-PT-RaquelNeural"), QStringLiteral("Portuguese"), QStringLiteral("pt-PT"), QStringLiteral("Feminina"), QStringLiteral("Raquel"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("en-US-AriaNeural"), QStringLiteral("English"), QStringLiteral("en-US"), QStringLiteral("Female"), QStringLiteral("Aria"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("en-US-JennyNeural"), QStringLiteral("English"), QStringLiteral("en-US"), QStringLiteral("Female"), QStringLiteral("Jenny"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("en-US-GuyNeural"), QStringLiteral("English"), QStringLiteral("en-US"), QStringLiteral("Male"), QStringLiteral("Guy"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("en-US-DavisNeural"), QStringLiteral("English"), QStringLiteral("en-US"), QStringLiteral("Male"), QStringLiteral("Davis"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("en-GB-SoniaNeural"), QStringLiteral("English"), QStringLiteral("en-GB"), QStringLiteral("Female"), QStringLiteral("Sonia"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("en-GB-RyanNeural"), QStringLiteral("English"), QStringLiteral("en-GB"), QStringLiteral("Male"), QStringLiteral("Ryan"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("en-AU-NatashaNeural"), QStringLiteral("English"), QStringLiteral("en-AU"), QStringLiteral("Female"), QStringLiteral("Natasha"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("en-AU-WilliamNeural"), QStringLiteral("English"), QStringLiteral("en-AU"), QStringLiteral("Male"), QStringLiteral("William"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("es-ES-ElviraNeural"), QStringLiteral("Espanol"), QStringLiteral("es-ES"), QStringLiteral("Femenina"), QStringLiteral("Elvira"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("es-ES-AlvaroNeural"), QStringLiteral("Espanol"), QStringLiteral("es-ES"), QStringLiteral("Masculina"), QStringLiteral("Alvaro"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("es-MX-DaliaNeural"), QStringLiteral("Espanol"), QStringLiteral("es-MX"), QStringLiteral("Femenina"), QStringLiteral("Dalia"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("es-MX-JorgeNeural"), QStringLiteral("Espanol"), QStringLiteral("es-MX"), QStringLiteral("Masculina"), QStringLiteral("Jorge"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("fr-FR-DeniseNeural"), QStringLiteral("Francais"), QStringLiteral("fr-FR"), QStringLiteral("Feminine"), QStringLiteral("Denise"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("fr-FR-HenriNeural"), QStringLiteral("Francais"), QStringLiteral("fr-FR"), QStringLiteral("Masculine"), QStringLiteral("Henri"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("fr-CA-SylvieNeural"), QStringLiteral("Francais"), QStringLiteral("fr-CA"), QStringLiteral("Feminine"), QStringLiteral("Sylvie"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("fr-CA-JeanNeural"), QStringLiteral("Francais"), QStringLiteral("fr-CA"), QStringLiteral("Masculine"), QStringLiteral("Jean"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("de-DE-KatjaNeural"), QStringLiteral("Deutsch"), QStringLiteral("de-DE"), QStringLiteral("Weiblich"), QStringLiteral("Katja"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("de-DE-ConradNeural"), QStringLiteral("Deutsch"), QStringLiteral("de-DE"), QStringLiteral("Mannlich"), QStringLiteral("Conrad"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("it-IT-ElsaNeural"), QStringLiteral("Italiano"), QStringLiteral("it-IT"), QStringLiteral("Femminile"), QStringLiteral("Elsa"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("it-IT-DiegoNeural"), QStringLiteral("Italiano"), QStringLiteral("it-IT"), QStringLiteral("Maschile"), QStringLiteral("Diego"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("ja-JP-NanamiNeural"), QStringLiteral("Japanese"), QStringLiteral("ja-JP"), QStringLiteral("Female"), QStringLiteral("Nanami"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("ja-JP-KeitaNeural"), QStringLiteral("Japanese"), QStringLiteral("ja-JP"), QStringLiteral("Male"), QStringLiteral("Keita"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("ko-KR-SunHiNeural"), QStringLiteral("Korean"), QStringLiteral("ko-KR"), QStringLiteral("Female"), QStringLiteral("SunHi"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("ko-KR-InJoonNeural"), QStringLiteral("Korean"), QStringLiteral("ko-KR"), QStringLiteral("Male"), QStringLiteral("InJoon"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("zh-CN-XiaoxiaoNeural"), QStringLiteral("Chinese"), QStringLiteral("zh-CN"), QStringLiteral("Female"), QStringLiteral("Xiaoxiao"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("zh-CN-YunxiNeural"), QStringLiteral("Chinese"), QStringLiteral("zh-CN"), QStringLiteral("Male"), QStringLiteral("Yunxi"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("ru-RU-SvetlanaNeural"), QStringLiteral("Russian"), QStringLiteral("ru-RU"), QStringLiteral("Female"), QStringLiteral("Svetlana"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("ru-RU-DmitryNeural"), QStringLiteral("Russian"), QStringLiteral("ru-RU"), QStringLiteral("Male"), QStringLiteral("Dmitry"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("ar-SA-ZariyahNeural"), QStringLiteral("Arabic"), QStringLiteral("ar-SA"), QStringLiteral("Female"), QStringLiteral("Zariyah"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("ar-SA-HamedNeural"), QStringLiteral("Arabic"), QStringLiteral("ar-SA"), QStringLiteral("Male"), QStringLiteral("Hamed"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("hi-IN-SwaraNeural"), QStringLiteral("Hindi"), QStringLiteral("hi-IN"), QStringLiteral("Female"), QStringLiteral("Swara"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("hi-IN-MadhurNeural"), QStringLiteral("Hindi"), QStringLiteral("hi-IN"), QStringLiteral("Male"), QStringLiteral("Madhur"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("nl-NL-ColetteNeural"), QStringLiteral("Nederlands"), QStringLiteral("nl-NL"), QStringLiteral("Female"), QStringLiteral("Colette"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("nl-NL-MaartenNeural"), QStringLiteral("Nederlands"), QStringLiteral("nl-NL"), QStringLiteral("Male"), QStringLiteral("Maarten"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("pl-PL-ZofiaNeural"), QStringLiteral("Polski"), QStringLiteral("pl-PL"), QStringLiteral("Female"), QStringLiteral("Zofia"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("pl-PL-MarekNeural"), QStringLiteral("Polski"), QStringLiteral("pl-PL"), QStringLiteral("Male"), QStringLiteral("Marek"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("tr-TR-EmelNeural"), QStringLiteral("Turkce"), QStringLiteral("tr-TR"), QStringLiteral("Female"), QStringLiteral("Emel"), QStringLiteral("Edge TTS")),
        voice(QStringLiteral("tr-TR-AhmetNeural"), QStringLiteral("Turkce"), QStringLiteral("tr-TR"), QStringLiteral("Male"), QStringLiteral("Ahmet"), QStringLiteral("Edge TTS")),
    };
}

QVariantList query_edge_tts_voices()
{
    return EdgeTTSClient::available_voice_options();
}

QVariantList parse_sapi_voices(const QString &output)
{
    QVariantList voices;
    QSet<QString> seen;
    const QStringList lines = output.split(QRegularExpression(QStringLiteral("[\\r\\n]+")), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        const QStringList parts = line.split(QLatin1Char('\t'));
        if (parts.isEmpty()) {
            continue;
        }

        const QString id = parts.value(0).trimmed();
        if (id.isEmpty() || seen.contains(id)) {
            continue;
        }

        const QString locale = parts.value(1).trimmed();
        const QString gender = parts.value(2).trimmed();
        voices.append(voice(id, language_for_locale(locale), locale, gender, id, QStringLiteral("Windows SAPI")));
        seen.insert(id);
    }
    return voices;
}

QVariantList query_sapi_voices()
{
    QProcess process;
    QString script;
    script += QStringLiteral("Add-Type -AssemblyName System.Speech;");
    script += QStringLiteral("$s=New-Object System.Speech.Synthesis.SpeechSynthesizer;");
    script += QStringLiteral("$s.GetInstalledVoices()|ForEach-Object{$i=$_.VoiceInfo;Write-Output ($i.Name + \"`t\" + $i.Culture.Name + \"`t\" + $i.Gender)};");
    script += QStringLiteral("$s.Dispose();");

    process.start(QStringLiteral("powershell"),
                  {QStringLiteral("-NoProfile"),
                   QStringLiteral("-ExecutionPolicy"),
                   QStringLiteral("Bypass"),
                   QStringLiteral("-Command"),
                   script});
    if (!process.waitForStarted(1000)) {
        return {};
    }
    if (!process.waitForFinished(6000)) {
        process.kill();
        process.waitForFinished(1000);
        return {};
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        return {};
    }
    return parse_sapi_voices(QString::fromUtf8(process.readAllStandardOutput()));
}

void append_unique_voices(QVariantList &target, const QVariantList &source)
{
    QSet<QString> seen;
    for (const QVariant &item : target) {
        const QVariantMap voice = item.toMap();
        seen.insert(voice.value(QStringLiteral("engine")).toString() + QLatin1Char('|')
                    + voice.value(QStringLiteral("id")).toString());
    }

    for (const QVariant &item : source) {
        const QVariantMap voice = item.toMap();
        const QString id = voice.value(QStringLiteral("id")).toString();
        const QString key = voice.value(QStringLiteral("engine")).toString() + QLatin1Char('|') + id;
        if (id.trimmed().isEmpty() || seen.contains(key)) {
            continue;
        }
        target.append(item);
        seen.insert(key);
    }
}

QString key_for_voice(const QVariantMap &voice)
{
    return voice.value(QStringLiteral("language")).toString() + QLatin1Char('|')
           + voice.value(QStringLiteral("locale")).toString() + QLatin1Char('|')
           + voice.value(QStringLiteral("engine")).toString() + QLatin1Char('|')
           + voice.value(QStringLiteral("name")).toString() + QLatin1Char('|')
           + voice.value(QStringLiteral("id")).toString();
}

}  // namespace

QString default_voice_id()
{
    return QStringLiteral("pt-BR-AntonioNeural");
}

QVariantList available_voice_options()
{
    static QVariantList cached;
    static bool querying_edge = false;
    if (cached.size() > 100) {
        return cached;
    }

    if (cached.isEmpty()) {
        cached = builtin_voice_options();
        append_unique_voices(cached, query_sapi_voices());
        std::sort(cached.begin(), cached.end(), [](const QVariant &left, const QVariant &right) {
            return key_for_voice(left.toMap()).localeAwareCompare(key_for_voice(right.toMap())) < 0;
        });
    }

    if (querying_edge) {
        return cached;
    }

    querying_edge = true;
    QVariantList edge_voices = query_edge_tts_voices();
    querying_edge = false;

    if (!edge_voices.isEmpty()) {
        QVariantList updated = edge_voices;
        append_unique_voices(updated, query_sapi_voices());
        std::sort(updated.begin(), updated.end(), [](const QVariant &left, const QVariant &right) {
            return key_for_voice(left.toMap()).localeAwareCompare(key_for_voice(right.toMap())) < 0;
        });
        cached = updated;
    }

    return cached;
}

QString voice_display_text(const QString &voice_id)
{
    for (const QVariant &item : available_voice_options()) {
        const QVariantMap voice = item.toMap();
        if (voice.value(QStringLiteral("id")).toString() == voice_id) {
            const QString name = voice.value(QStringLiteral("name")).toString();
            const QString locale = voice.value(QStringLiteral("locale")).toString();
            const QString gender = voice.value(QStringLiteral("gender")).toString();
            const QString engine = voice.value(QStringLiteral("engine")).toString();
            return QStringLiteral("%1 (%2%3%4%5%6)")
                .arg(name.isEmpty() ? voice_id : name,
                     locale,
                     gender.isEmpty() ? QString() : QStringLiteral(", "),
                     gender,
                     engine.isEmpty() ? QString() : QStringLiteral(", "),
                     engine);
        }
    }
    return voice_id;
}

}  // namespace source::plugin::Audiolivro
