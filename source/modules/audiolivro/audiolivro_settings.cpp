#include "source/modules/audiolivro/audiolivro_settings.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSettings>

#include "source/utils/CaminhoPersistenteUtils.hpp"

namespace source::plugin::Audiolivro {
namespace {

constexpr auto kConfigFileName = "audiolivro_config.json";

QString legacy_settings_key(const QString &key)
{
    if (key == QStringLiteral("voice_id")) {
        return QStringLiteral("audiolivro/voice_id");
    }
    if (key == QStringLiteral("font_size")) {
        return QStringLiteral("audiolivro/font_size");
    }
    return {};
}

QJsonObject read_config()
{
    QFile file(audiolivro_config_path());
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    return document.isObject() ? document.object() : QJsonObject{};
}

void write_config(const QJsonObject &config)
{
    const QString path = audiolivro_config_path();
    QDir().mkpath(QFileInfo(path).absolutePath());

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }

    const QByteArray payload = QJsonDocument(config).toJson(QJsonDocument::Indented);
    if (file.write(payload) == payload.size()) {
        file.commit();
    }
}

QString read_legacy_setting(const QString &key)
{
    const QString legacy_key = legacy_settings_key(key);
    if (legacy_key.isEmpty()) {
        return {};
    }
    return QSettings().value(legacy_key).toString().trimmed();
}

int point_size_from_text(const QString &value, int fallback)
{
    const QRegularExpressionMatch match = QRegularExpression(QStringLiteral("(\\d+)")).match(value);
    return qBound(6, match.hasMatch() ? match.captured(1).toInt() : fallback, 72);
}

bool bool_from_config(const QString &value, bool fallback = false)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == QStringLiteral("true") || normalized == QStringLiteral("1")
        || normalized == QStringLiteral("yes")) {
        return true;
    }
    if (normalized == QStringLiteral("false") || normalized == QStringLiteral("0")
        || normalized == QStringLiteral("no")) {
        return false;
    }
    return fallback;
}

QString bool_to_config(bool value)
{
    return value ? QStringLiteral("true") : QStringLiteral("false");
}

}  // namespace

QString audiolivro_persistent_dir()
{
    return source::utils::obter_caminho_persistente();
}

QString audiolivro_config_path()
{
    return QDir(audiolivro_persistent_dir()).filePath(QString::fromLatin1(kConfigFileName));
}

QString carregar_configuracao_texto(const QString &key, const QString &fallback)
{
    const QJsonObject config = read_config();
    const QString stored = config.value(key).toString().trimmed();
    if (!stored.isEmpty()) {
        return stored;
    }

    const QString legacy = read_legacy_setting(key);
    if (!legacy.isEmpty()) {
        salvar_configuracao_texto(key, legacy);
        return legacy;
    }

    return fallback;
}

void salvar_configuracao_texto(const QString &key, const QString &value)
{
    QJsonObject config = read_config();
    config.insert(key, value);
    write_config(config);
}

QFont fonte_audiolivro_padrao()
{
    QFont font = QGuiApplication::font();
    font.setPointSize(10);
    font.setBold(false);
    font.setItalic(false);
    font.setUnderline(false);
    font.setStrikeOut(false);
    return font;
}

QFont carregar_fonte_configurada()
{
    QFont font = fonte_audiolivro_padrao();
    const QString family = carregar_configuracao_texto(QStringLiteral("font_family")).trimmed();
    if (!family.isEmpty()) {
        font.setFamily(family);
    }
    const QString style = carregar_configuracao_texto(QStringLiteral("font_style")).trimmed();
    if (!style.isEmpty()) {
        font.setStyleName(style);
    }

    font.setPointSize(point_size_from_text(
        carregar_configuracao_texto(QStringLiteral("font_size"), QStringLiteral("10pt")),
        font.pointSize() > 0 ? font.pointSize() : 10));
    font.setBold(bool_from_config(carregar_configuracao_texto(QStringLiteral("font_bold")), false));
    font.setItalic(bool_from_config(carregar_configuracao_texto(QStringLiteral("font_italic")), false));
    font.setUnderline(bool_from_config(carregar_configuracao_texto(QStringLiteral("font_underline")), false));
    font.setStrikeOut(bool_from_config(carregar_configuracao_texto(QStringLiteral("font_strikeout")), false));
    return font;
}

void salvar_fonte_configurada(const QFont &font)
{
    salvar_configuracao_texto(QStringLiteral("font_family"), font.family());
    salvar_configuracao_texto(QStringLiteral("font_style"), font.styleName());
    salvar_configuracao_texto(QStringLiteral("font_size"), QStringLiteral("%1pt").arg(qBound(6, font.pointSize(), 72)));
    salvar_configuracao_texto(QStringLiteral("font_bold"), bool_to_config(font.bold()));
    salvar_configuracao_texto(QStringLiteral("font_italic"), bool_to_config(font.italic()));
    salvar_configuracao_texto(QStringLiteral("font_underline"), bool_to_config(font.underline()));
    salvar_configuracao_texto(QStringLiteral("font_strikeout"), bool_to_config(font.strikeOut()));
}

QColor carregar_cor_configurada()
{
    const QString hex = carregar_configuracao_texto(QStringLiteral("text_color")).trimmed();
    if (!hex.isEmpty()) {
        const QColor color(hex);
        if (color.isValid()) {
            return color;
        }
    }
    return {};
}

void salvar_cor_configurada(const QColor &color)
{
    salvar_configuracao_texto(QStringLiteral("text_color"), color.isValid() ? color.name(QColor::HexArgb) : QString());
}

static QString temp_text_file_path()
{
    return QDir(audiolivro_persistent_dir()).filePath(QStringLiteral("audiolivro_temp_text.html"));
}

void salvar_pdf_carregado_temp(const QString &pdf_path)
{
    salvar_configuracao_texto(QStringLiteral("temp_pdf_path"), pdf_path);
}

QString carregar_pdf_carregado_temp()
{
    return carregar_configuracao_texto(QStringLiteral("temp_pdf_path"));
}

void salvar_texto_carregado_temp(const QString &content)
{
    const QString path = temp_text_file_path();
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        const QByteArray payload = content.toUtf8();
        if (file.write(payload) == payload.size()) {
            file.commit();
        }
    }
}

QString carregar_texto_carregado_temp()
{
    QFile file(temp_text_file_path());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    return QString::fromUtf8(file.readAll());
}

void limpar_texto_carregado_temp()
{
    QFile::remove(temp_text_file_path());
}

void salvar_aba_ativa_temp(int index)
{
    salvar_configuracao_texto(QStringLiteral("temp_active_tab"), QString::number(index));
}

int carregar_aba_ativa_temp()
{
    return carregar_configuracao_texto(QStringLiteral("temp_active_tab"), QStringLiteral("0")).toInt();
}

}  // namespace source::plugin::Audiolivro
