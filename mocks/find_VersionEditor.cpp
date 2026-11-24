#include "find_VersionEditor.hpp"

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMap>
#include <QPair>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QTabWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QtGlobal>

#include <optional>
#include <cstdio>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace mocks::version_editor {
namespace {

struct FileSpec {
    QString key;
    QString relative_path;
    QString content_key;
    QString group;
};

struct FileStatus {
    QString key;
    QString group;
    QString relative_path;
    QString path;
    bool exists = false;
    QStringList found;
    QStringList missing;
};

struct UpdateResult {
    QString key;
    QString group;
    QString relative_path;
    bool ok = false;
    QString message;
};

struct UpdateContext {
    QString pt_version;
    QString pt_date;
    QString en_version;
    QString en_date;
    QString pt_readme_version;
    QString en_readme_version;
};

const QStringList kPtMonths = {
    QStringLiteral("Janeiro"),
    QStringLiteral("Fevereiro"),
    QStringLiteral("Março"),
    QStringLiteral("Abril"),
    QStringLiteral("Maio"),
    QStringLiteral("Junho"),
    QStringLiteral("Julho"),
    QStringLiteral("Agosto"),
    QStringLiteral("Setembro"),
    QStringLiteral("Outubro"),
    QStringLiteral("Novembro"),
    QStringLiteral("Dezembro"),
};

const QStringList kPtMonthsLower = {
    QStringLiteral("janeiro"),
    QStringLiteral("fevereiro"),
    QStringLiteral("março"),
    QStringLiteral("abril"),
    QStringLiteral("maio"),
    QStringLiteral("junho"),
    QStringLiteral("julho"),
    QStringLiteral("agosto"),
    QStringLiteral("setembro"),
    QStringLiteral("outubro"),
    QStringLiteral("novembro"),
    QStringLiteral("dezembro"),
};

const QStringList kEnMonths = {
    QStringLiteral("January"),
    QStringLiteral("February"),
    QStringLiteral("March"),
    QStringLiteral("April"),
    QStringLiteral("May"),
    QStringLiteral("June"),
    QStringLiteral("July"),
    QStringLiteral("August"),
    QStringLiteral("September"),
    QStringLiteral("October"),
    QStringLiteral("November"),
    QStringLiteral("December"),
};

const QString &group_about()
{
    static const QString value = QStringLiteral("Sobre");
    return value;
}

const QString &group_clc()
{
    static const QString value = QStringLiteral("CLC");
    return value;
}

const QString &group_copyright()
{
    static const QString value = QStringLiteral("Direitos Autorais");
    return value;
}

const QString &group_eula()
{
    static const QString value = QStringLiteral("EULA");
    return value;
}

const QString &group_notices()
{
    static const QString value = QStringLiteral("Notices");
    return value;
}

const QString &group_privacy()
{
    static const QString value = QStringLiteral("Política de Privacidade");
    return value;
}

const QString &group_modules()
{
    static const QString value = QStringLiteral("Módulos");
    return value;
}

const QString &group_docs()
{
    static const QString value = QStringLiteral("Documentação");
    return value;
}

const QString &group_plugins()
{
    return group_modules();
}

const QString &group_misc()
{
    static const QString value = QStringLiteral("Miscelânia");
    return value;
}

const QStringList &group_order()
{
    static const QStringList order = {
        group_about(),
        group_clc(),
        group_copyright(),
        group_eula(),
        group_notices(),
        group_privacy(),
        group_modules(),
        group_docs(),
        group_misc(),
    };
    return order;
}

QString normalize_dir(const QString &path)
{
    if (path.trimmed().isEmpty()) {
        return QString();
    }

    return QDir::cleanPath(QFileInfo(path).absoluteFilePath());
}

QString detect_project_root()
{
    const QString app_dir = QCoreApplication::applicationDirPath();
    const QString current_dir = QDir::currentPath();
    const QString source_file_dir = QFileInfo(QString::fromUtf8(__FILE__)).absolutePath();

    QStringList seeds = {app_dir, current_dir, source_file_dir};
    QStringList checked;

    auto is_project_root = [](const QString &candidate) {
        const QDir dir(candidate);
        return dir.exists(QStringLiteral("mocks")) && dir.exists(QStringLiteral("source"))
               && QFileInfo(dir.filePath(QStringLiteral("README.md"))).exists();
    };

    for (const QString &seed : seeds) {
        QDir dir(seed);
        for (int depth = 0; depth < 8; ++depth) {
            const QString candidate = normalize_dir(dir.absolutePath());
            if (!checked.contains(candidate)) {
                checked.append(candidate);
                if (is_project_root(candidate)) {
                    return candidate;
                }
            }

            if (!dir.cdUp()) {
                break;
            }
        }
    }

    return normalize_dir(QDir(source_file_dir).filePath(QStringLiteral("..")));
}

QString build_version_from_date(const QDate &date = QDate::currentDate())
{
    const QDate safe_date = date.isValid() ? date : QDate::currentDate();
    if (!safe_date.isValid()) {
        return QStringLiteral("2025.11.26.0");
    }

    return QStringLiteral("%1.%2.%3.0")
        .arg(safe_date.year())
        .arg(safe_date.month())
        .arg(safe_date.day());
}

QString readme_version_token(const QString &version)
{
    const QString trimmed = version.trimmed();
    if (trimmed.isEmpty()) {
        return QStringLiteral("v0.0.0.0");
    }

    return trimmed.startsWith(QLatin1Char('v'), Qt::CaseInsensitive) ? trimmed : QStringLiteral("v%1").arg(trimmed);
}

QString spec_content_key(const FileSpec &spec)
{
    return spec.content_key.isEmpty() ? spec.key : spec.content_key;
}

QString detect_release_markdown_relpath(const QString &base_dir)
{
    const QDir docs_dir(QDir(base_dir).filePath(QStringLiteral("docs")));
    if (docs_dir.exists()) {
        const QStringList candidates = docs_dir.entryList(
            QStringList{QStringLiteral("release-v*.md")},
            QDir::Files,
            QDir::Name | QDir::Reversed);
        if (!candidates.isEmpty()) {
            return QStringLiteral("docs/") + candidates.first();
        }
    }

    return QStringLiteral("docs/release-v2026.10.8.0.md");
}

QStringList discover_docs_markdown_relpaths(const QString &base_dir, const QString &release_relpath)
{
    QStringList relpaths;
    const QDir docs_dir(QDir(base_dir).filePath(QStringLiteral("docs")));
    if (!docs_dir.exists()) {
        return relpaths;
    }

    const QStringList entries = docs_dir.entryList(
        QStringList{QStringLiteral("*.md")},
        QDir::Files,
        QDir::Name | QDir::IgnoreCase);
    for (const QString &entry : entries) {
        const QString rel = QStringLiteral("docs/") + entry;
        if (rel.compare(release_relpath, Qt::CaseInsensitive) != 0) {
            relpaths.append(rel);
        }
    }
    return relpaths;
}

QList<FileSpec> file_specs(const QString &base_dir)
{
    QList<FileSpec> data;

    auto make_key = [](const QString &relative_path) {
        QString key;
        bool last_was_separator = false;
        for (const QChar ch : relative_path.toCaseFolded()) {
            if (ch.isLetterOrNumber()) {
                key.append(ch);
                last_was_separator = false;
            } else if (!last_was_separator) {
                key.append(QLatin1Char('_'));
                last_was_separator = true;
            }
        }

        while (key.endsWith(QLatin1Char('_'))) {
            key.chop(1);
        }
        return key;
    };

    auto language_suffix = [](const QString &filename) {
        if (filename.contains(QStringLiteral("_pt_BR")) ||
            filename.startsWith(QStringLiteral("AVISO DE COPYRIGHT"), Qt::CaseInsensitive)) {
            return QStringLiteral("_pt");
        }
        return QStringLiteral("_en");
    };

    auto append = [&](const QString &group,
                      const QString &relative_path,
                      const QString &content_key) {
        data.append({make_key(relative_path), relative_path, content_key, group});
    };

    auto append_asset = [&](const QString &group,
                            const QString &product,
                            const QString &folder,
                            const QStringList &filenames,
                            const QString &content_key_prefix) {
        for (const QString &filename : filenames) {
            const QString relative_path =
                QStringLiteral("source/assets/%1/%2/%3").arg(product, folder, filename);
            append(group, relative_path, content_key_prefix + language_suffix(filename));
        }
    };

    const QStringList products = {QStringLiteral("eisenhower"), QStringLiteral("lumen")};

    const QStringList about_files = {
        QStringLiteral("ABOUT_pt_BR.txt"),
        QStringLiteral("ABOUT_en_US.txt"),
        QStringLiteral("ABOUT_Eisenhower_Organizer_pt_BR.txt"),
        QStringLiteral("ABOUT_Eisenhower_Organizer_en_US.txt"),
        QStringLiteral("ABOUT_plugin_pomodoro_pt_BR.txt"),
        QStringLiteral("ABOUT_plugin_pomodoro_en_US.txt"),
        QStringLiteral("ABOUT_plugin_feynman_pt_BR.txt"),
        QStringLiteral("ABOUT_plugin_feynman_en_US.txt"),
        QStringLiteral("ABOUT_plugin_mapamental_pt_BR.txt"),
        QStringLiteral("ABOUT_plugin_mapamental_en_US.txt"),
        QStringLiteral("ABOUT_plugin_audiolivro_pt_BR.txt"),
        QStringLiteral("ABOUT_plugin_audiolivro_en_US.txt"),
        QStringLiteral("ABOUT_plugin_agenda_pt_BR.txt"),
        QStringLiteral("ABOUT_plugin_agenda_en_US.txt"),
        QStringLiteral("ABOUT_plugin_modelo_pt_BR.txt"),
        QStringLiteral("ABOUT_plugin_modelo_en_US.txt"),
        QStringLiteral("ABOUT_plugin_gantt_pt_BR.txt"),
        QStringLiteral("ABOUT_plugin_gantt_en_US.txt"),
    };

    const QStringList clc_files = {
        QStringLiteral("CLC_pt_BR.txt"),
        QStringLiteral("CLC_en_US.txt"),
        QStringLiteral("CLC_Eisenhower_Organizer_pt_BR.txt"),
        QStringLiteral("CLC_Eisenhower_Organizer_en_US.txt"),
        QStringLiteral("CLC_plugin_pomodoro_pt_BR.txt"),
        QStringLiteral("CLC_plugin_pomodoro_en_US.txt"),
        QStringLiteral("CLC_plugin_feynman_pt_BR.txt"),
        QStringLiteral("CLC_plugin_feynman_en_US.txt"),
        QStringLiteral("CLC_plugin_mapamental_pt_BR.txt"),
        QStringLiteral("CLC_plugin_mapamental_en_US.txt"),
        QStringLiteral("CLC_plugin_audiolivro_pt_BR.txt"),
        QStringLiteral("CLC_plugin_audiolivro_en_US.txt"),
        QStringLiteral("CLC_plugin_agenda_pt_BR.txt"),
        QStringLiteral("CLC_plugin_agenda_en_US.txt"),
        QStringLiteral("CLC_plugin_modelo_pt_BR.txt"),
        QStringLiteral("CLC_plugin_modelo_en_US.txt"),
        QStringLiteral("CLC_plugin_gantt_pt_BR.txt"),
        QStringLiteral("CLC_plugin_gantt_en_US.txt"),
    };

    const QStringList copyright_files = {
        QStringLiteral("AVISO DE COPYRIGHT E MARCA REGISTRA_pt_BR.txt"),
        QStringLiteral("COPYRIGHT AND TRADEMARK NOTICE_en_US.txt"),
        QStringLiteral("Copyright_Notice_Eisenhower_Organizer_pt_BR.txt"),
        QStringLiteral("Copyright_Notice_Eisenhower_Organizer_en_US.txt"),
        QStringLiteral("Copyright_Notice_plugin_pomodoro_pt_BR.txt"),
        QStringLiteral("Copyright_Notice_plugin_pomodoro_en_US.txt"),
        QStringLiteral("Copyright_Notice_plugin_feynman_pt_BR.txt"),
        QStringLiteral("Copyright_Notice_plugin_feynman_en_US.txt"),
        QStringLiteral("Copyright_Notice_plugin_mapamental_pt_BR.txt"),
        QStringLiteral("Copyright_Notice_plugin_mapamental_en_US.txt"),
        QStringLiteral("Copyright_Notice_plugin_audiolivro_pt_BR.txt"),
        QStringLiteral("Copyright_Notice_plugin_audiolivro_en_US.txt"),
        QStringLiteral("Copyright_Notice_plugin_agenda_pt_BR.txt"),
        QStringLiteral("Copyright_Notice_plugin_agenda_en_US.txt"),
        QStringLiteral("Copyright_Notice_plugin_modelo_pt_BR.txt"),
        QStringLiteral("Copyright_Notice_plugin_modelo_en_US.txt"),
        QStringLiteral("Copyright_Notice_plugin_gantt_pt_BR.txt"),
        QStringLiteral("Copyright_Notice_plugin_gantt_en_US.txt"),
    };

    const QStringList eula_files = {
        QStringLiteral("EULA_pt_BR.txt"),
        QStringLiteral("EULA_en_US.txt"),
        QStringLiteral("EULA_Eisenhower_Organizer_pt_BR.txt"),
        QStringLiteral("EULA_Eisenhower_Organizer_en_US.txt"),
        QStringLiteral("EULA_plugin_pomodoro_pt_BR.txt"),
        QStringLiteral("EULA_plugin_pomodoro_en_US.txt"),
        QStringLiteral("EULA_plugin_feynman_pt_BR.txt"),
        QStringLiteral("EULA_plugin_feynman_en_US.txt"),
        QStringLiteral("EULA_plugin_mapamental_pt_BR.txt"),
        QStringLiteral("EULA_plugin_mapamental_en_US.txt"),
        QStringLiteral("EULA_plugin_audiolivro_pt_BR.txt"),
        QStringLiteral("EULA_plugin_audiolivro_en_US.txt"),
        QStringLiteral("EULA_plugin_agenda_pt_BR.txt"),
        QStringLiteral("EULA_plugin_agenda_en_US.txt"),
        QStringLiteral("EULA_plugin_modelo_pt_BR.txt"),
        QStringLiteral("EULA_plugin_modelo_en_US.txt"),
        QStringLiteral("EULA_plugin_gantt_pt_BR.txt"),
        QStringLiteral("EULA_plugin_gantt_en_US.txt"),
    };

    const QStringList notice_files = {
        QStringLiteral("NOTICE_pt_BR.txt"),
        QStringLiteral("NOTICE_en_US.txt"),
        QStringLiteral("NOTICE_Eisenhower_Organizer_pt_BR.txt"),
        QStringLiteral("NOTICE_Eisenhower_Organizer_en_US.txt"),
        QStringLiteral("NOTICE_plugin_pomodoro_pt_BR.txt"),
        QStringLiteral("NOTICE_plugin_pomodoro_en_US.txt"),
        QStringLiteral("NOTICE_plugin_feynman_pt_BR.txt"),
        QStringLiteral("NOTICE_plugin_feynman_en_US.txt"),
        QStringLiteral("NOTICE_plugin_mapamental_pt_BR.txt"),
        QStringLiteral("NOTICE_plugin_mapamental_en_US.txt"),
        QStringLiteral("NOTICE_plugin_audiolivro_pt_BR.txt"),
        QStringLiteral("NOTICE_plugin_audiolivro_en_US.txt"),
        QStringLiteral("NOTICE_plugin_agenda_pt_BR.txt"),
        QStringLiteral("NOTICE_plugin_agenda_en_US.txt"),
        QStringLiteral("NOTICE_plugin_modelo_pt_BR.txt"),
        QStringLiteral("NOTICE_plugin_modelo_en_US.txt"),
        QStringLiteral("NOTICE_plugin_gantt_pt_BR.txt"),
        QStringLiteral("NOTICE_plugin_gantt_en_US.txt"),
    };

    const QStringList privacy_files = {
        QStringLiteral("Privacy_Policy_pt_BR.txt"),
        QStringLiteral("Privacy_Policy_en_US.txt"),
        QStringLiteral("Privacy_Policy_Eisenhower_Organizer_pt_BR.txt"),
        QStringLiteral("Privacy_Policy_Eisenhower_Organizer_en_US.txt"),
        QStringLiteral("Privacy_Policy_plugin_pomodoro_pt_BR.txt"),
        QStringLiteral("Privacy_Policy_plugin_pomodoro_en_US.txt"),
        QStringLiteral("Privacy_Policy_plugin_feynman_pt_BR.txt"),
        QStringLiteral("Privacy_Policy_plugin_feynman_en_US.txt"),
        QStringLiteral("Privacy_Policy_plugin_mapamental_pt_BR.txt"),
        QStringLiteral("Privacy_Policy_plugin_mapamental_en_US.txt"),
        QStringLiteral("Privacy_Policy_plugin_audiolivro_pt_BR.txt"),
        QStringLiteral("Privacy_Policy_plugin_audiolivro_en_US.txt"),
        QStringLiteral("Privacy_Policy_plugin_agenda_pt_BR.txt"),
        QStringLiteral("Privacy_Policy_plugin_agenda_en_US.txt"),
        QStringLiteral("Privacy_Policy_plugin_modelo_pt_BR.txt"),
        QStringLiteral("Privacy_Policy_plugin_modelo_en_US.txt"),
        QStringLiteral("Privacy_Policy_plugin_gantt_pt_BR.txt"),
        QStringLiteral("Privacy_Policy_plugin_gantt_en_US.txt"),
    };

    for (const QString &product : products) {
        append_asset(group_about(), product, QStringLiteral("ABOUT"), about_files, QStringLiteral("about"));
        append_asset(group_clc(), product, QStringLiteral("CLC"), clc_files, QStringLiteral("clc"));
        append_asset(group_copyright(),
                     product,
                     QStringLiteral("COPYRIGHT"),
                     copyright_files,
                     QStringLiteral("copyright"));
        append_asset(group_eula(), product, QStringLiteral("EULA"), eula_files, QStringLiteral("eula"));
        append_asset(group_notices(), product, QStringLiteral("NOTICES"), notice_files, QStringLiteral("notice"));
        append_asset(group_privacy(),
                     product,
                     QStringLiteral("PRIVACY_POLICY"),
                     privacy_files,
                     QStringLiteral("privacy"));
    }

    append(group_misc(), QStringLiteral("README.md"), QStringLiteral("readme_pt"));
    append(group_misc(), QStringLiteral("README.md"), QStringLiteral("readme_en"));
    append(group_misc(), QStringLiteral("OBSERVACAO.md"), QStringLiteral("observacao_pt"));
    append(group_misc(), QStringLiteral("source/services/incore_08_exibir_sobre.cpp"), QStringLiteral("about_dialog"));
    append(group_misc(), QStringLiteral("library/Eisenhower_Organizer.iss"), QStringLiteral("installer_unified"));
    append(group_misc(), QStringLiteral("library/Eisenhower_Organizer_MinGW.iss"), QStringLiteral("installer_mingw"));
    append(group_misc(), QStringLiteral("library/Eisenhower_Organizer_MSVC.iss"), QStringLiteral("installer_msvc"));
    append(group_misc(), QStringLiteral("library/Lumen.iss"), QStringLiteral("installer_unified"));
    append(group_misc(), QStringLiteral("library/Lumen_MinGW.iss"), QStringLiteral("lumen_installer_mingw"));
    append(group_misc(), QStringLiteral("library/Lumen_MSVC.iss"), QStringLiteral("lumen_installer_msvc"));
    append(group_misc(), QStringLiteral("library/E-AppxManifest.xml"), QStringLiteral("appx_manifest"));
    append(group_misc(), QStringLiteral("library/L-AppxManifest.xml"), QStringLiteral("appx_manifest"));
    append(group_misc(), QStringLiteral("library/Eisenhower_Organizer_Lumen_cpp.txt"), QStringLiteral("library_notes"));

    append(group_docs(), QStringLiteral("CODE_OF_CONDUCT.md"), QStringLiteral("docs_bilingual_md"));
    append(group_docs(), QStringLiteral("CONTRIBUTING.md"), QStringLiteral("docs_bilingual_md"));
    append(group_docs(), QStringLiteral("MANUAL.md"), QStringLiteral("manual_md"));
    append(group_docs(), QStringLiteral("SECURITY.md"), QStringLiteral("docs_bilingual_md"));
    const QString release_relpath = detect_release_markdown_relpath(base_dir);
    for (const QString &doc_relpath : discover_docs_markdown_relpaths(base_dir, release_relpath)) {
        append(group_docs(), doc_relpath, QStringLiteral("docs_bilingual_md"));
    }
    append(group_docs(), release_relpath, QStringLiteral("docs_release_markdown"));

    append(group_modules(), QStringLiteral("source/modules/pomodoro/pomodoro_plugin.json"), QStringLiteral("plugin_manifest"));
    append(group_modules(), QStringLiteral("source/modules/mapamental/mapamental_plugin.json"), QStringLiteral("plugin_manifest"));
    append(group_modules(), QStringLiteral("source/modules/modelo/modelo_plugin.json"), QStringLiteral("plugin_manifest"));
    append(group_modules(), QStringLiteral("source/modules/feynman/feynman_plugin.json"), QStringLiteral("plugin_manifest"));
    append(group_modules(), QStringLiteral("source/modules/audiolivro/audiolivro_plugin.json"), QStringLiteral("plugin_manifest"));
    append(group_modules(), QStringLiteral("source/modules/agenda/agenda_plugin.json"), QStringLiteral("plugin_manifest"));
    append(group_modules(), QStringLiteral("source/modules/gantt/gantt_plugin.json"), QStringLiteral("plugin_manifest"));
    append(group_modules(),
           QStringLiteral("source/modules/pomodoro/pomodoro_plugin_interface.hpp"),
           QStringLiteral("plugin_interface_iid"));
    append(group_modules(),
           QStringLiteral("source/modules/mapamental/mapamental_plugin_interface.hpp"),
           QStringLiteral("plugin_interface_iid"));
    append(group_modules(),
           QStringLiteral("source/modules/modelo/modelo_plugin_interface.hpp"),
           QStringLiteral("plugin_interface_iid"));
    append(group_modules(),
           QStringLiteral("source/modules/feynman/feynman_plugin_interface.hpp"),
           QStringLiteral("plugin_interface_iid"));
    append(group_modules(),
           QStringLiteral("source/modules/audiolivro/audiolivro_plugin_interface.hpp"),
           QStringLiteral("plugin_interface_iid"));
    append(group_modules(),
           QStringLiteral("source/modules/agenda/agenda_plugin_interface.hpp"),
           QStringLiteral("plugin_interface_iid"));
    append(group_modules(),
           QStringLiteral("source/modules/gantt/gantt_plugin_interface.hpp"),
           QStringLiteral("plugin_interface_iid"));

    return data;
}

const QMap<QString, QStringList> &patterns()
{
    static const QMap<QString, QStringList> data = {
        {QStringLiteral("readme_pt"),
         {QStringLiteral(R"(^(> \*\*Observa(?:ção|cao):\*\*.*EISENHOWER ORGANIZER.*)$)"),
          QStringLiteral(R"(^(> \*\*Observa(?:ção|cao):\*\*.*L(?:ú|u)men.*)$)"),
          QStringLiteral(R"(^(Vers(?:ã|a)o:\s*v[\d\.]+<br>)\s*$)"),
          QStringLiteral(R"(^(Data t(?:é|e)cnica desta revis(?:ã|a)o:\s*.*<br>)\s*$)")}},
        {QStringLiteral("readme_en"),
         {QStringLiteral(R"(^(> \*\*Note:\*\*.*EISENHOWER ORGANIZER.*)$)"),
          QStringLiteral(R"(^(> \*\*Note:\*\*.*L(?:ú|u)men.*)$)"),
          QStringLiteral(R"(^(Version:\s*v[\d\.]+<br>)\s*$)"),
          QStringLiteral(R"(^(Technical revision date:\s*.*<br>)\s*$)")}},
        {QStringLiteral("observacao_pt"),
         {QStringLiteral(R"(^(Vers(?:ã|a)o documentada:\s*`[\d\.]+`)\s*$)"),
          QStringLiteral(R"(^(Data do snapshot t(?:é|e)cnico:\s*`.*`)\s*$)")}},
        {QStringLiteral("privacy_pt"),
         {QStringLiteral(R"(^(Versão:\s*.*)$)"), QStringLiteral(R"(^(Última atualização:\s*.*)$)")}},
        {QStringLiteral("privacy_en"),
         {QStringLiteral(R"(^(Version:\s*.*)$)"), QStringLiteral(R"(^(Last updated:\s*.*)$)")}},
        {QStringLiteral("notice_pt"), {QStringLiteral(R"(^(Versão.*)$)")}},
        {QStringLiteral("notice_en"), {QStringLiteral(R"(^(Version.*)$)")}},
        {QStringLiteral("eula_pt"), {QStringLiteral(R"(^(Versão.*)$)")}},
        {QStringLiteral("eula_en"), {QStringLiteral(R"(^(Version.*)$)")}},
        {QStringLiteral("copyright_pt"),
         {QStringLiteral(R"(^(Versão.*)$)"), QStringLiteral(R"(^(Copyright.*)$)")}},
        {QStringLiteral("copyright_en"),
         {QStringLiteral(R"(^(Version.*)$)"), QStringLiteral(R"(^(Copyright.*)$)")}},
        {QStringLiteral("clc_pt"),
         {QStringLiteral(R"(^(Versão:\s*.*)$)"), QStringLiteral(R"(^(Data:\s*.*)$)")}},
        {QStringLiteral("clc_en"),
         {QStringLiteral(R"(^(Version:\s*.*)$)"), QStringLiteral(R"(^(Date:\s*.*)$)")}},
        {QStringLiteral("about_pt"), {QStringLiteral(R"(^(Versão:\s*.*)$)")}},
        {QStringLiteral("about_en"), {QStringLiteral(R"(^(Version:\s*.*)$)")}},
        {QStringLiteral("about_dialog"), {QStringLiteral(R"(^(\s*"<p><b>%2:</b>\s*[\d\.]+</p>".*)$)")}},
        {QStringLiteral("installer_unified"), {QStringLiteral(R"(^(#define\s+MyAppVersion\s+"[\d\.]+";?)\r?$)")}},
        {QStringLiteral("installer_mingw"),
         {QStringLiteral(R"(^(#define\s+MyAppVersion\s+"[\d\.]+";?)\r?$)"),
          QStringLiteral(R"(^(OutputBaseFilename=Eisenhower_Organizer_mingw_v(?:[\d\.]+|\{#MyAppVersion\}))\r?$)")}},
        {QStringLiteral("installer_msvc"),
         {QStringLiteral(R"(^(#define\s+MyAppVersion\s+"[\d\.]+";?)\r?$)"),
          QStringLiteral(R"(^(\s*OutputBaseFilename=Eisenhower_Organizer_msvc_v(?:[\d\.]+|\{#MyAppVersion\}))\r?$)")}},
        {QStringLiteral("lumen_installer_mingw"),
         {QStringLiteral(R"(^(#define\s+MyAppVersion\s+"[\d\.]+";?)\r?$)"),
          QStringLiteral(R"(^(OutputBaseFilename=Lumen_mingw_v(?:[\d\.]+|\{#MyAppVersion\}))\r?$)")}},
        {QStringLiteral("lumen_installer_msvc"),
         {QStringLiteral(R"(^(#define\s+MyAppVersion\s+"[\d\.]+";?)\r?$)"),
          QStringLiteral(R"(^(\s*OutputBaseFilename=Lumen_msvc_v(?:[\d\.]+|\{#MyAppVersion\}))\r?$)")}},
        {QStringLiteral("appx_manifest"),
         {QStringLiteral(R"(^(\s*<Identity\b[^>]*\bVersion="[\d\.]+"[^>]*/>)\r?$)")}},
        {QStringLiteral("library_notes"),
         {QStringLiteral(R"(^(NOTAS DE LANÇAMENTO:\s*Versão\s+[\d\.]+)\s*$)"),
          QStringLiteral(R"(^(Data:\s*.+)\s*$)"),
          QStringLiteral(R"(^(RELEASE NOTES:\s*Version\s+[\d\.]+)\s*$)"),
          QStringLiteral(R"(^(Date:\s*.+)\s*$)"),
          QStringLiteral(R"(^(Versão:\s*[\d\.]+)\s*$)"),
          QStringLiteral(R"(^(Version:\s*[\d\.]+)\s*$)"),
          QStringLiteral(R"(^(.*docs[\\/]release-v[\d\.]+\.md.*)$)")}},
        {QStringLiteral("docs_bilingual_md"),
         {QStringLiteral(R"(^(\*\*Vers(?:ã|a)o:\*\*\s*`[\d\.]+`)\s*$)"),
          QStringLiteral(R"(^(\*\*Data:\*\*\s*`[^`]+`)\s*$)"),
          QStringLiteral(R"(^(\*\*Version:\*\*\s*`[\d\.]+`)\s*$)"),
          QStringLiteral(R"(^(\*\*Date:\*\*\s*`[^`]+`)\s*$)")}},
        {QStringLiteral("manual_md"),
         {QStringLiteral(R"(^(Vers(?:ã|a)o documentada:\s*`[\d\.]+`)\s*$)"),
          QStringLiteral(R"(^(Data:\s*`[^`]+`)\s*$)"),
          QStringLiteral(R"(^(Documented Version:\s*`[\d\.]+`)\s*$)"),
          QStringLiteral(R"(^(Date:\s*`[^`]+`)\s*$)")}},
        {QStringLiteral("docs_release_markdown"),
         {QStringLiteral(R"(^(# .+v[\d\.]+)\s*$)"),
          QStringLiteral(R"(^(# .+Vers(?:ã|a)o\s+[\d\.]+)\s*$)"),
          QStringLiteral(R"(^(\*\*Data de Lan(?:ç|c)amento:\*\*\s*`[^`]+`.*)$)"),
          QStringLiteral(R"(^(# .+Version\s+[\d\.]+)\s*$)"),
          QStringLiteral(R"(^(\*\*Release Date:\*\*\s*`[^`]+`.*)$)")}},
        {QStringLiteral("plugin_manifest"), {QStringLiteral(R"(^(\s*"version"\s*:\s*"[^"]+"\s*,?\s*)$)")}},
        {QStringLiteral("plugin_interface_iid"),
         {QStringLiteral(R"(^(\s*#define\s+SOURCE_[A-Z_]+_PLUGIN_INTERFACE_IID\s+"source\.plugin\.[^/"]+/[\d\.]+"\s*)$)")}},
        {QStringLiteral("app_plugin_status_fallback"),
         {QStringLiteral(R"(^(\s*return\s+QStringLiteral\("[\d\.]+"\);\s*)$)"),
          QStringLiteral(R"(^(\s*return\s+document\.isObject\(\)\s*\?\s*document\.object\(\)\.value\(QStringLiteral\("version"\)\)\.toString\(QStringLiteral\("[\d\.]+"\)\)\s*)$)"),
          QStringLiteral(R"(^(\s*:\s*QStringLiteral\("[\d\.]+"\);\s*)$)")}},
    };

    return data;
}

QString resolve_file_path(const QString &base_dir, const QString &relative_path)
{
    const QDir dir(base_dir);
    const QString primary = QFileInfo(dir.filePath(relative_path)).absoluteFilePath();
    if (QFileInfo::exists(primary)) {
        return primary;
    }

    if (relative_path.startsWith(QStringLiteral("source/modules/"))) {
        const QString plugin_alt = QStringLiteral("plugin/") + relative_path.mid(QStringLiteral("source/modules/").size());
        const QString alt_path = QFileInfo(dir.filePath(plugin_alt)).absoluteFilePath();
        if (QFileInfo::exists(alt_path)) {
            return alt_path;
        }
    }

    if (relative_path.startsWith(QStringLiteral("plugin/"))) {
        const QString mod_alt = QStringLiteral("source/modules/") + relative_path.mid(QStringLiteral("plugin/").size());
        const QString alt_path = QFileInfo(dir.filePath(mod_alt)).absoluteFilePath();
        if (QFileInfo::exists(alt_path)) {
            return alt_path;
        }
    }

    const QStringList rel_parts = QDir::fromNativeSeparators(relative_path).split(QLatin1Char('/'), Qt::SkipEmptyParts);
    if (!rel_parts.isEmpty() && rel_parts.first() == QStringLiteral("source")) {
        QString stripped;
        for (int i = 1; i < rel_parts.size(); ++i) {
            if (!stripped.isEmpty()) {
                stripped += QLatin1Char('/');
            }
            stripped += rel_parts.at(i);
        }

        const QString alternate = QFileInfo(dir.filePath(stripped)).absoluteFilePath();
        if (QFileInfo::exists(alternate)) {
            return alternate;
        }
    }

    return primary;
}

std::optional<QString> read_file_text(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return std::nullopt;
    }

    QByteArray data = file.readAll();
    if (data.startsWith("\xEF\xBB\xBF")) {
        data = data.mid(3);
    }

    QString utf8 = QString::fromUtf8(data);
    if (!utf8.contains(QChar::ReplacementCharacter)) {
        return utf8;
    }

    QString latin1 = QString::fromLatin1(data);
    return latin1;
}

bool write_file_text(const QString &path, const QString &text, QString *error_message)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        if (error_message != nullptr) {
            *error_message = file.errorString();
        }
        return false;
    }

    const qint64 written = file.write(text.toUtf8());
    if (written < 0) {
        if (error_message != nullptr) {
            *error_message = file.errorString();
        }
        return false;
    }

    return true;
}

QString detect_line_ending(const QString &text)
{
    return text.contains(QStringLiteral("\r\n")) ? QStringLiteral("\r\n") : QStringLiteral("\n");
}

QStringList split_lines_preserving_empty(const QString &text)
{
    return text.split(QRegularExpression(QStringLiteral(R"(\r\n|\n|\r)")), Qt::KeepEmptyParts);
}

QString join_lines(const QStringList &lines, const QString &separator)
{
    return lines.join(separator);
}

QStringList first_lines(const QString &text, int limit)
{
    QStringList lines = split_lines_preserving_empty(text);
    if (lines.size() > limit) {
        lines = lines.mid(0, limit);
    }
    return lines;
}

QList<FileStatus> check_expected_lines(const QString &base_dir)
{
    QList<FileStatus> statuses;

    for (const FileSpec &spec : file_specs(base_dir)) {
        FileStatus status;
        status.key = spec.key;
        status.group = spec.group.isEmpty() ? group_misc() : spec.group;
        status.relative_path = spec.relative_path;
        status.path = resolve_file_path(base_dir, spec.relative_path);

        const std::optional<QString> text = read_file_text(status.path);
        if (!text.has_value()) {
            status.exists = false;
            statuses.append(status);
            continue;
        }

        status.exists = true;
        const QStringList pattern_list = patterns().value(spec_content_key(spec));
        if (pattern_list.isEmpty()) {
            status.found = first_lines(text.value(), 3);
            statuses.append(status);
            continue;
        }

        for (const QString &pattern : pattern_list) {
            const QRegularExpression rx(pattern, QRegularExpression::MultilineOption);
            const QRegularExpressionMatch match = rx.match(text.value());
            if (match.hasMatch()) {
                status.found.append(match.captured(1).trimmed());
            } else {
                status.missing.append(pattern);
            }
        }

        statuses.append(status);
    }

    return statuses;
}

QString replace_readme_version_token(const QString &line, const QString &version)
{
    const QRegularExpression rx(QStringLiteral(R"(^(.*\*\*)v[\d\.]+(\*\*.*)$)"));
    const QRegularExpressionMatch match = rx.match(line);
    if (!match.hasMatch()) {
        return line;
    }

    return QStringLiteral("%1%2%3").arg(match.captured(1), version, match.captured(2));
}

bool all_patterns_matched(const QString &text, const QString &content_key)
{
    const QStringList pattern_list = patterns().value(content_key);
    for (const QString &pattern : pattern_list) {
        const QRegularExpression rx(pattern, QRegularExpression::MultilineOption);
        if (!rx.match(text).hasMatch()) {
            return false;
        }
    }
    return !pattern_list.isEmpty();
}

QString extract_series_token(const QString &version)
{
    const QStringList parts = version.trimmed().split(QLatin1Char('.'), Qt::SkipEmptyParts);
    if (parts.size() >= 2) {
        return QStringLiteral("%1.%2").arg(parts.at(0), parts.at(1));
    }
    return version.trimmed();
}

bool apply_line_updates(const QString &key, QStringList *lines, const UpdateContext &context)
{
    if (lines == nullptr) {
        return false;
    }

    const QString series_token = extract_series_token(context.pt_version);
    bool changed = false;
    for (QString &line : *lines) {
        const QString original = line;

        if (key == QStringLiteral("readme_pt")) {
            if (line.startsWith(QStringLiteral("> **Observação:**")) ||
                line.startsWith(QStringLiteral("> **Observacao:**"))) {
                line = replace_readme_version_token(line, context.pt_readme_version);
            } else if (line.startsWith(QStringLiteral("Versão:"))) {
                line = QStringLiteral("Versão: %1<br>").arg(context.pt_readme_version);
            } else if (line.startsWith(QStringLiteral("Versao:"))) {
                line = QStringLiteral("Versao: %1<br>").arg(context.pt_readme_version);
            } else if (line.startsWith(QStringLiteral("Data técnica desta revisão:"))) {
                line = QStringLiteral("Data técnica desta revisão: %1<br>").arg(context.pt_date);
            } else if (line.startsWith(QStringLiteral("Data tecnica desta revisao:"))) {
                line = QStringLiteral("Data tecnica desta revisao: %1<br>").arg(context.pt_date);
            } else {
                line.replace(QRegularExpression(QStringLiteral(R"(docs/release-v[\d\.]+\.md)")),
                             QStringLiteral("docs/release-v%1.md").arg(context.pt_version));
                line.replace(QRegularExpression(QStringLiteral(R"(Version-v[\d\.]+-blue\.svg)")),
                             QStringLiteral("Version-%1-blue.svg").arg(context.pt_readme_version));
                line.replace(QRegularExpression(QStringLiteral(R"(Notas de Lançamento v[\d\.]+)")),
                             QStringLiteral("Notas de Lançamento %1").arg(context.pt_readme_version));
            }
        } else if (key == QStringLiteral("readme_en")) {
            if (line.startsWith(QStringLiteral("> **Note:**"))) {
                line = replace_readme_version_token(line, context.en_readme_version);
            } else if (line.startsWith(QStringLiteral("Version:"))) {
                line = QStringLiteral("Version: %1<br>").arg(context.en_readme_version);
            } else if (line.startsWith(QStringLiteral("Technical revision date:"))) {
                line = QStringLiteral("Technical revision date: %1<br>").arg(context.en_date);
            } else {
                line.replace(QRegularExpression(QStringLiteral(R"(Release Notes v[\d\.]+)")),
                             QStringLiteral("Release Notes %1").arg(context.en_readme_version));
            }
        } else if (key == QStringLiteral("observacao_pt")) {
            if (line.startsWith(QStringLiteral("Versão documentada:"))) {
                line = QStringLiteral("Versão documentada: `%1`").arg(context.pt_version);
            } else if (line.startsWith(QStringLiteral("Versao documentada:"))) {
                line = QStringLiteral("Versao documentada: `%1`").arg(context.pt_version);
            } else if (line.startsWith(QStringLiteral("Data do snapshot técnico:"))) {
                line = QStringLiteral("Data do snapshot técnico: `%1`").arg(context.pt_date);
            } else if (line.startsWith(QStringLiteral("Data do snapshot tecnico:"))) {
                line = QStringLiteral("Data do snapshot tecnico: `%1`").arg(context.pt_date);
            } else if (line.startsWith(QStringLiteral("Documented Version:"))) {
                line = QStringLiteral("Documented Version: `%1`").arg(context.en_version);
            } else if (line.startsWith(QStringLiteral("Technical Snapshot Date:"))) {
                line = QStringLiteral("Technical Snapshot Date: `%1`").arg(context.en_date);
            }
        } else if (key == QStringLiteral("privacy_pt")) {
            if (line.startsWith(QStringLiteral("Versão:"))) {
                line = QStringLiteral("Versão: %1").arg(context.pt_version);
            } else if (line.startsWith(QStringLiteral("Última atualização:"))) {
                line = QStringLiteral("Última atualização: %1").arg(context.pt_date);
            }
        } else if (key == QStringLiteral("privacy_en")) {
            if (line.startsWith(QStringLiteral("Version:"))) {
                line = QStringLiteral("Version: %1").arg(context.en_version);
            } else if (line.startsWith(QStringLiteral("Last updated:"))) {
                line = QStringLiteral("Last updated: %1").arg(context.en_date);
            }
        } else if (key == QStringLiteral("notice_pt") || key == QStringLiteral("eula_pt")
                   || key == QStringLiteral("copyright_pt")) {
            if (line.startsWith(QStringLiteral("Versão:"))) {
                line = QStringLiteral("Versão: %1").arg(context.pt_version);
            } else if (line.startsWith(QStringLiteral("Data:"))) {
                line = QStringLiteral("Data: %1").arg(context.pt_date);
            } else if (line.startsWith(QStringLiteral("Versão "))) {
                line = QStringLiteral("Versão %1, %2").arg(context.pt_version, context.pt_date);
            }
        } else if (key == QStringLiteral("notice_en") || key == QStringLiteral("eula_en")
                   || key == QStringLiteral("copyright_en")) {
            if (line.startsWith(QStringLiteral("Version:"))) {
                line = QStringLiteral("Version: %1").arg(context.en_version);
            } else if (line.startsWith(QStringLiteral("Date:"))) {
                line = QStringLiteral("Date: %1").arg(context.en_date);
            } else if (line.startsWith(QStringLiteral("Version "))) {
                line = QStringLiteral("Version %1, %2").arg(context.en_version, context.en_date);
            }
        } else if (key == QStringLiteral("clc_pt")) {
            if (line.startsWith(QStringLiteral("Versão:"))) {
                line = QStringLiteral("Versão: %1").arg(context.pt_version);
            } else if (line.startsWith(QStringLiteral("Data:"))) {
                line = QStringLiteral("Data: %1").arg(context.pt_date);
            }
        } else if (key == QStringLiteral("clc_en")) {
            if (line.startsWith(QStringLiteral("Version:"))) {
                line = QStringLiteral("Version: %1").arg(context.en_version);
            } else if (line.startsWith(QStringLiteral("Date:"))) {
                line = QStringLiteral("Date: %1").arg(context.en_date);
            }
        } else if (key == QStringLiteral("about_pt")) {
            if (line.startsWith(QStringLiteral("Versão:"))) {
                line = QStringLiteral("Versão: %1").arg(context.pt_version);
            }
        } else if (key == QStringLiteral("about_en")) {
            if (line.startsWith(QStringLiteral("Version:"))) {
                line = QStringLiteral("Version: %1").arg(context.en_version);
            }
        } else if (key == QStringLiteral("about_dialog")) {
            const QRegularExpression rx(QStringLiteral(R"((<p><b>%2:</b>\s*)[\d\.]+(</p>))"));
            const QRegularExpressionMatch match = rx.match(line);
            if (match.hasMatch()) {
                line = line.left(match.capturedStart(0)) + match.captured(1) + context.pt_version
                       + match.captured(2) + line.mid(match.capturedEnd(0));
            }
        } else if (key == QStringLiteral("installer_unified") || key == QStringLiteral("installer_mingw")
                   || key == QStringLiteral("installer_msvc") || key == QStringLiteral("lumen_installer_mingw")
                   || key == QStringLiteral("lumen_installer_msvc")) {
            if (line.startsWith(QStringLiteral("#define MyAppVersion"))) {
                const QRegularExpression rx(QStringLiteral(R"((#define\s+MyAppVersion\s+")[\d\.]+(";?))"));
                const QRegularExpressionMatch match = rx.match(line);
                if (match.hasMatch()) {
                    line = match.captured(1) + context.pt_version + match.captured(2);
                }
            } else if (!line.contains(QStringLiteral("{#MyAppVersion}"))) {
                if (key == QStringLiteral("installer_mingw")
                    && line.startsWith(QStringLiteral("OutputBaseFilename=Eisenhower_Organizer_mingw_v"))) {
                    line = QStringLiteral("OutputBaseFilename=Eisenhower_Organizer_mingw_v%1").arg(context.pt_version);
                } else if (key == QStringLiteral("installer_msvc")
                           && line.trimmed().startsWith(QStringLiteral("OutputBaseFilename=Eisenhower_Organizer_msvc_v"))) {
                    const QString indentation = line.left(line.size() - line.trimmed().size());
                    line = QStringLiteral("%1OutputBaseFilename=Eisenhower_Organizer_msvc_v%2")
                               .arg(indentation, context.pt_version);
                } else if (key == QStringLiteral("lumen_installer_mingw")
                           && line.startsWith(QStringLiteral("OutputBaseFilename=Lumen_mingw_v"))) {
                    line = QStringLiteral("OutputBaseFilename=Lumen_mingw_v%1").arg(context.pt_version);
                } else if (key == QStringLiteral("lumen_installer_msvc")
                           && line.trimmed().startsWith(QStringLiteral("OutputBaseFilename=Lumen_msvc_v"))) {
                    const QString indentation = line.left(line.size() - line.trimmed().size());
                    line = QStringLiteral("%1OutputBaseFilename=Lumen_msvc_v%2").arg(indentation, context.pt_version);
                }
            }
        } else if (key == QStringLiteral("appx_manifest")) {
            const QRegularExpression rx(QStringLiteral(R"(^(\s*<Identity\b[^>]*\bVersion=")[\d\.]+("[^>]*/>)\s*$)"));
            const QRegularExpressionMatch match = rx.match(line);
            if (match.hasMatch()) {
                line = match.captured(1) + context.pt_version + match.captured(2);
            }
        } else if (key == QStringLiteral("library_notes")) {
            if (line.startsWith(QStringLiteral("NOTAS DE LANÇAMENTO: Versão "))) {
                line = QStringLiteral("NOTAS DE LANÇAMENTO: Versão %1").arg(context.pt_version);
            } else if (line.startsWith(QStringLiteral("RELEASE NOTES: Version "))) {
                line = QStringLiteral("RELEASE NOTES: Version %1").arg(context.en_version);
            } else if (line.startsWith(QStringLiteral("Versão:"))) {
                line = QStringLiteral("Versão: %1").arg(context.pt_version);
            } else if (line.startsWith(QStringLiteral("Version:"))) {
                line = QStringLiteral("Version: %1").arg(context.en_version);
            } else if (line.startsWith(QStringLiteral("Data:"))) {
                line = QStringLiteral("Data: %1").arg(context.pt_date);
            } else if (line.startsWith(QStringLiteral("Date:"))) {
                line = QStringLiteral("Date: %1").arg(context.en_date);
            } else {
                const QRegularExpression rx(QStringLiteral(R"((docs[\\/]release-v)[\d\.]+(\.md))"));
                const QRegularExpressionMatch match = rx.match(line);
                if (match.hasMatch()) {
                    line = line.left(match.capturedStart(0)) + match.captured(1) + context.pt_version
                           + match.captured(2) + line.mid(match.capturedEnd(0));
                }
            }
        } else if (key == QStringLiteral("docs_bilingual_md")) {
            const QRegularExpression rx_pt_ver(QStringLiteral(R"(^(\*\*Vers(?:ã|a)o:\*\*\s*`)[\d\.]+(`\s*)$)"));
            const QRegularExpression rx_pt_date(QStringLiteral(R"(^(\*\*Data:\*\*\s*`)[^`]+(`\s*)$)"));
            const QRegularExpression rx_en_ver(QStringLiteral(R"(^(\*\*Version:\*\*\s*`)[\d\.]+(`\s*)$)"));
            const QRegularExpression rx_en_date(QStringLiteral(R"(^(\*\*Date:\*\*\s*`)[^`]+(`\s*)$)"));
            if (const auto m = rx_pt_ver.match(line); m.hasMatch()) {
                line = m.captured(1) + context.pt_version + m.captured(2);
            } else if (const auto m = rx_pt_date.match(line); m.hasMatch()) {
                line = m.captured(1) + context.pt_date + m.captured(2);
            } else if (const auto m = rx_en_ver.match(line); m.hasMatch()) {
                line = m.captured(1) + context.en_version + m.captured(2);
            } else if (const auto m = rx_en_date.match(line); m.hasMatch()) {
                line = m.captured(1) + context.en_date + m.captured(2);
            } else if (line.startsWith(QStringLiteral("| `v")) || line.startsWith(QStringLiteral("| `< v"))) {
                line.replace(QRegularExpression(QStringLiteral(R"(\bv\d+\.\d+\b)")),
                             QStringLiteral("v%1").arg(series_token));
            }
        } else if (key == QStringLiteral("manual_md")) {
            const QRegularExpression rx_pt_ver(QStringLiteral(R"(^(Vers(?:ã|a)o documentada:\s*`)[\d\.]+(`\s*)$)"));
            const QRegularExpression rx_pt_date(QStringLiteral(R"(^(Data:\s*`)[^`]+(`\s*)$)"));
            const QRegularExpression rx_en_ver(QStringLiteral(R"(^(Documented Version:\s*`)[\d\.]+(`\s*)$)"));
            const QRegularExpression rx_en_date(QStringLiteral(R"(^(Date:\s*`)[^`]+(`\s*)$)"));
            if (const auto m = rx_pt_ver.match(line); m.hasMatch()) {
                line = m.captured(1) + context.pt_version + m.captured(2);
            } else if (const auto m = rx_pt_date.match(line); m.hasMatch()) {
                line = m.captured(1) + context.pt_date + m.captured(2);
            } else if (const auto m = rx_en_ver.match(line); m.hasMatch()) {
                line = m.captured(1) + context.en_version + m.captured(2);
            } else if (const auto m = rx_en_date.match(line); m.hasMatch()) {
                line = m.captured(1) + context.en_date + m.captured(2);
            }
        } else if (key == QStringLiteral("docs_release_markdown")) {
            if (line.startsWith(QStringLiteral("# "))) {
                const QRegularExpression rx_pt_heading(QStringLiteral(R"(^(.+Vers(?:ã|a)o\s+)[\d\.]+(\s*)$)"));
                const QRegularExpression rx_en_heading(QStringLiteral(R"(^(.+Version\s+)[\d\.]+(\s*)$)"));
                const QRegularExpression rx_title_v(QStringLiteral(R"(^(.+\b)v[\d\.]+(\s*)$)"));
                if (const auto m = rx_pt_heading.match(line); m.hasMatch()) {
                    line = m.captured(1) + context.pt_version + m.captured(2);
                } else if (const auto m = rx_en_heading.match(line); m.hasMatch()) {
                    line = m.captured(1) + context.en_version + m.captured(2);
                } else if (const auto m = rx_title_v.match(line); m.hasMatch()) {
                    line = m.captured(1) + context.pt_readme_version + m.captured(2);
                }
            } else if (line.startsWith(QStringLiteral("**Data de Lançamento:**"))
                       || line.startsWith(QStringLiteral("**Data de Lancamento:**"))) {
                const QRegularExpression rx(QStringLiteral(R"(^(\*\*Data de Lan(?:ç|c)amento:\*\*\s*`)[^`]+(`.*)$)"));
                if (const auto m = rx.match(line); m.hasMatch()) {
                    line = m.captured(1) + context.pt_date + m.captured(2);
                }
            } else if (line.startsWith(QStringLiteral("**Release Date:**"))) {
                const QRegularExpression rx(QStringLiteral(R"(^(\*\*Release Date:\*\*\s*`)[^`]+(`.*)$)"));
                if (const auto m = rx.match(line); m.hasMatch()) {
                    line = m.captured(1) + context.en_date + m.captured(2);
                }
            }
        } else if (key == QStringLiteral("plugin_manifest")) {
            const QRegularExpression rx(QStringLiteral(R"(^(\s*"version"\s*:\s*")[^"]+("\s*,?\s*)$)"));
            const QRegularExpressionMatch match = rx.match(line);
            if (match.hasMatch()) {
                line = match.captured(1) + context.pt_version + match.captured(2);
            }
        } else if (key == QStringLiteral("plugin_interface_iid")) {
            const QRegularExpression rx(QStringLiteral(
                R"((#define\s+SOURCE_[A-Z_]+_PLUGIN_INTERFACE_IID\s+"source\.plugin\.[^/"]+/)[\d\.]+("))"));
            const QRegularExpressionMatch match = rx.match(line);
            if (match.hasMatch()) {
                line = line.left(match.capturedStart(0)) + match.captured(1) + context.pt_version
                       + match.captured(2) + line.mid(match.capturedEnd(0));
            }
        } else if (key == QStringLiteral("app_plugin_status_fallback")) {
            const QRegularExpression rx(QStringLiteral(R"(QStringLiteral\("\d+(?:\.\d+){1,3}"\))"));
            line.replace(rx, QStringLiteral("QStringLiteral(\"%1\")").arg(context.pt_version));
        }

        if (line != original) {
            changed = true;
        }
    }

    return changed;
}

QList<UpdateResult> apply_updates(const UpdateContext &context, const QString &base_dir)
{
    QList<UpdateResult> results;

    for (const FileSpec &spec : file_specs(base_dir)) {
        UpdateResult result;
        result.key = spec.key;
        result.group = spec.group.isEmpty() ? group_misc() : spec.group;
        result.relative_path = spec.relative_path;

        const QString path = resolve_file_path(base_dir, spec.relative_path);
        const std::optional<QString> text = read_file_text(path);
        if (!text.has_value()) {
            result.ok = false;
            result.message = QStringLiteral("arquivo inexistente");
            results.append(result);
            continue;
        }

        const QString content_key = spec_content_key(spec);
        const QString line_ending = detect_line_ending(text.value());
        QStringList lines = split_lines_preserving_empty(text.value());
        const bool changed = apply_line_updates(content_key, &lines, context);
        const bool patterns_ok = all_patterns_matched(text.value(), content_key);

        if (changed) {
            QString error_message;
            if (!write_file_text(path, join_lines(lines, line_ending), &error_message)) {
                result.ok = false;
                result.message = QStringLiteral("erro escrita: %1").arg(error_message);
                results.append(result);
                continue;
            }
        }

        bool renamed = false;
        if (content_key == QStringLiteral("docs_release_markdown")) {
            const QString target_rel = QStringLiteral("docs/release-v%1.md").arg(context.pt_version);
            const QString target_abs = QFileInfo(QDir(base_dir).filePath(target_rel)).absoluteFilePath();
            const QString source_abs = QFileInfo(path).absoluteFilePath();
            if (source_abs.compare(target_abs, Qt::CaseInsensitive) != 0) {
                if (QFileInfo::exists(target_abs)) {
                    result.ok = false;
                    result.message = QStringLiteral("destino já existe: %1").arg(target_rel);
                    results.append(result);
                    continue;
                }
                if (!QFile::rename(source_abs, target_abs)) {
                    result.ok = false;
                    result.message = QStringLiteral("erro ao renomear para %1").arg(target_rel);
                    results.append(result);
                    continue;
                }
                renamed = true;
                result.relative_path = QStringLiteral("%1 -> %2").arg(spec.relative_path, target_rel);
            }
        }

        if (changed && renamed) {
            result.ok = true;
            result.message = QStringLiteral("atualizado e renomeado");
        } else if (renamed) {
            result.ok = true;
            result.message = QStringLiteral("renomeado");
        } else if (changed) {
            result.ok = true;
            result.message = QStringLiteral("atualizado");
        } else if (patterns_ok) {
            result.ok = true;
            result.message = QStringLiteral("já atualizado (sem alterações)");
        } else {
            result.ok = false;
            result.message = QStringLiteral("padrões não encontrados / sem alteração");
        }

        results.append(result);
    }

    return results;
}

QString mock_windows_icon_name()
{
#if defined(MOCKS_WINDOWS_ICON_NAME)
    return QStringLiteral(MOCKS_WINDOWS_ICON_NAME);
#elif defined(_MSC_VER)
    return QStringLiteral("autismo.ico");
#else
    return QStringLiteral("organizador.ico");
#endif
}

QString find_icon_path(const QString &project_root)
{
    const QString icon_name = mock_windows_icon_name();
    const QDir root(project_root);
    const QDir app_dir(QCoreApplication::applicationDirPath());
    const QDir current_dir(QDir::currentPath());
    const QDir source_dir(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath());

    const QStringList candidates = {
        app_dir.filePath(QStringLiteral("icones/%1").arg(icon_name)),
        root.filePath(QStringLiteral("mocks/icones/%1").arg(icon_name)),
        current_dir.filePath(QStringLiteral("icones/%1").arg(icon_name)),
        current_dir.filePath(QStringLiteral("mocks/icones/%1").arg(icon_name)),
        source_dir.filePath(QStringLiteral("icones/%1").arg(icon_name)),
    };

    for (const QString &candidate : candidates) {
        if (QFileInfo::exists(candidate)) {
            return QFileInfo(candidate).absoluteFilePath();
        }
    }

    return QString();
}

}  // namespace

bool VersionEditorWidget::updateAllRegisteredFiles(const QString &base_dir,
                                                   const QString &pt_version,
                                                   const QDate &pt_date,
                                                   const QString &en_version,
                                                   const QDate &en_date)
{
    const QString target_dir = base_dir.isEmpty() ? detect_project_root() : normalize_dir(base_dir);
    const int pt_day = pt_date.isValid() ? pt_date.day() : 1;
    const int pt_month_index = pt_date.isValid() ? qMax(0, pt_date.month() - 1) : 0;
    const int pt_year = pt_date.isValid() ? pt_date.year() : 2026;

    const int en_day = en_date.isValid() ? en_date.day() : 1;
    const int en_month_index = en_date.isValid() ? qMax(0, en_date.month() - 1) : 0;
    const int en_year = en_date.isValid() ? en_date.year() : 2026;

    const UpdateContext context = {
        pt_version,
        QStringLiteral("%1 de %2 de %3")
            .arg(QString::number(pt_day), kPtMonthsLower.value(pt_month_index), QString::number(pt_year)),
        en_version,
        QStringLiteral("%1 %2, %3")
            .arg(kEnMonths.value(en_month_index), QString::number(en_day), QString::number(en_year)),
        readme_version_token(pt_version),
        readme_version_token(en_version),
    };

    const QList<UpdateResult> results = apply_updates(context, target_dir);
    for (const auto &r : results) {
        if (!r.ok) {
            return false;
        }
    }
    return true;
}

VersionEditorWidget::VersionEditorWidget(QWidget *parent) : QWidget(parent), selected_base_dir_(detect_project_root())
{
    setWindowTitle(QStringLiteral("Editor de Versões / Datas - Lúmen (PT/EN)"));
    resize(980, 640);

    const QDate today = QDate::currentDate();
    const QString default_version = build_version_from_date(today);
    const int default_day = today.isValid() ? today.day() : 1;
    const int default_month_index = today.isValid() ? qMax(0, today.month() - 1) : 0;
    const int default_year = today.isValid() ? today.year() : 2025;

    const QString icon_path = find_icon_path(selected_base_dir_);
    if (!icon_path.isEmpty()) {
        const QIcon icon(icon_path);
        QApplication::setWindowIcon(icon);
        setWindowIcon(icon);
    }

    auto *layout = new QVBoxLayout(this);

    auto *folder_layout = new QHBoxLayout();
    folder_layout->addWidget(new QLabel(QStringLiteral("Pasta dos arquivos:"), this));

    folder_path_ = new QLineEdit(selected_base_dir_, this);
    folder_path_->setReadOnly(true);
    folder_layout->addWidget(folder_path_);

    select_folder_btn_ = new QPushButton(QStringLiteral("Selecionar..."), this);
    folder_layout->addWidget(select_folder_btn_);
    layout->addLayout(folder_layout);

    auto *pt_group = new QGroupBox(QStringLiteral("Português (pt-BR)"), this);
    auto *pt_layout = new QHBoxLayout(pt_group);
    pt_layout->addWidget(new QLabel(QStringLiteral("Versão:"), pt_group));
    pt_version_ = new QLineEdit(default_version, pt_group);
    pt_layout->addWidget(pt_version_);

    pt_layout->addWidget(new QLabel(QStringLiteral("Dia:"), pt_group));
    pt_day_ = new QSpinBox(pt_group);
    pt_day_->setRange(1, 31);
    pt_day_->setValue(default_day);
    pt_layout->addWidget(pt_day_);

    pt_layout->addWidget(new QLabel(QStringLiteral("Mês:"), pt_group));
    pt_month_ = new QComboBox(pt_group);
    pt_month_->addItems(kPtMonths);
    pt_month_->setCurrentIndex(default_month_index);
    pt_layout->addWidget(pt_month_);

    pt_layout->addWidget(new QLabel(QStringLiteral("Ano:"), pt_group));
    pt_year_ = new QSpinBox(pt_group);
    pt_year_->setRange(1900, 3000);
    pt_year_->setValue(default_year);
    pt_layout->addWidget(pt_year_);
    layout->addWidget(pt_group);

    auto *en_group = new QGroupBox(QStringLiteral("English (en-US)"), this);
    auto *en_layout = new QHBoxLayout(en_group);
    en_layout->addWidget(new QLabel(QStringLiteral("Version:"), en_group));
    en_version_ = new QLineEdit(default_version, en_group);
    en_layout->addWidget(en_version_);

    en_layout->addWidget(new QLabel(QStringLiteral("Day:"), en_group));
    en_day_ = new QSpinBox(en_group);
    en_day_->setRange(1, 31);
    en_day_->setValue(default_day);
    en_layout->addWidget(en_day_);

    en_layout->addWidget(new QLabel(QStringLiteral("Month:"), en_group));
    en_month_ = new QComboBox(en_group);
    en_month_->addItems(kEnMonths);
    en_month_->setCurrentIndex(default_month_index);
    en_layout->addWidget(en_month_);

    en_layout->addWidget(new QLabel(QStringLiteral("Year:"), en_group));
    en_year_ = new QSpinBox(en_group);
    en_year_->setRange(1900, 3000);
    en_year_->setValue(default_year);
    en_layout->addWidget(en_year_);
    layout->addWidget(en_group);

    auto *btn_layout = new QHBoxLayout();
    check_btn_ = new QPushButton(QStringLiteral("🔍 Verificar arquivos"), this);
    save_btn_ = new QPushButton(QStringLiteral("💾 Salvar alterações"), this);
    refresh_btn_ = new QPushButton(QStringLiteral("🔄 Recarregar verificação"), this);
    btn_layout->addWidget(check_btn_);
    btn_layout->addWidget(save_btn_);
    btn_layout->addWidget(refresh_btn_);
    layout->addLayout(btn_layout);

    status_tabs_ = new QTabWidget(this);
    status_tabs_->setDocumentMode(true);
    status_tabs_->setUsesScrollButtons(true);
    status_tabs_->setElideMode(Qt::ElideRight);
    layout->addWidget(status_tabs_, 1);

    connect(select_folder_btn_, &QPushButton::clicked, this, [this]() { selectBaseDirectory(); });
    connect(check_btn_, &QPushButton::clicked, this, [this]() { checkFiles(); });
    connect(save_btn_, &QPushButton::clicked, this, [this]() { saveChanges(); });
    connect(refresh_btn_, &QPushButton::clicked, this, [this]() { checkFiles(); });

    checkFiles();
}

void VersionEditorWidget::selectBaseDirectory()
{
    const QString selected = QFileDialog::getExistingDirectory(
        this, QStringLiteral("Selecione a pasta onde os arquivos estão"), selected_base_dir_);
    if (selected.isEmpty()) {
        return;
    }

    selected_base_dir_ = normalize_dir(selected);
    folder_path_->setText(selected_base_dir_);
    checkFiles();
}

void VersionEditorWidget::checkFiles()
{
    const QList<FileStatus> statuses = check_expected_lines(selected_base_dir_);
    QMap<QString, QStringList> grouped_lines;
    for (const QString &group : group_order()) {
        grouped_lines[group].append(QStringLiteral("Base: %1").arg(selected_base_dir_));
        grouped_lines[group].append(QString());
    }

    for (const FileStatus &status : statuses) {
        QStringList &lines = grouped_lines[status.group];
        lines.append(QStringLiteral("%1").arg(status.relative_path));
        lines.append(QStringLiteral("  chave: %1").arg(status.key));
        lines.append(QStringLiteral("  arquivo: %1").arg(status.exists ? QStringLiteral("existe") : QStringLiteral("ausente")));

        if (status.exists) {
            if (!status.found.isEmpty()) {
                lines.append(QStringLiteral("  encontrado:"));
                for (const QString &entry : status.found) {
                    lines.append(QStringLiteral("    - %1").arg(entry));
                }
            }

            if (!status.missing.isEmpty()) {
                lines.append(QStringLiteral("  ausente no conteúdo:"));
                for (const QString &entry : status.missing) {
                    lines.append(QStringLiteral("    - %1").arg(entry));
                }
            }
        }

        lines.append(QString());
    }

    QList<QPair<QString, QStringList>> groups;
    for (const QString &group : group_order()) {
        groups.append({group, grouped_lines.value(group)});
    }
    setStatusGroups(groups);
}

void VersionEditorWidget::saveChanges()
{
    const QString pt_version = pt_version_->text().trimmed();
    const int pt_day = pt_day_->value();
    const int pt_month_index = pt_month_->currentIndex();
    const int pt_year = pt_year_->value();

    const QString en_version = en_version_->text().trimmed();
    const int en_day = en_day_->value();
    const int en_month_index = en_month_->currentIndex();
    const int en_year = en_year_->value();

    const UpdateContext context = {
        pt_version,
        QStringLiteral("%1 de %2 de %3")
            .arg(QString::number(pt_day), kPtMonthsLower.value(pt_month_index), QString::number(pt_year)),
        en_version,
        QStringLiteral("%1 %2, %3")
            .arg(kEnMonths.value(en_month_index), QString::number(en_day), QString::number(en_year)),
        readme_version_token(pt_version),
        readme_version_token(en_version),
    };

    const QList<UpdateResult> results = apply_updates(context, selected_base_dir_);

    QMap<QString, QStringList> grouped_lines;
    for (const QString &group : group_order()) {
        grouped_lines[group].append(QStringLiteral("Resultados:"));
        grouped_lines[group].append(QString());
    }

    for (const UpdateResult &result : results) {
        QStringList &lines = grouped_lines[result.group];
        lines.append(QStringLiteral("%1: %2 - %3")
                         .arg(result.relative_path,
                              result.ok ? QStringLiteral("OK") : QStringLiteral("FAIL"),
                              result.message));
    }

    QList<QPair<QString, QStringList>> groups;
    for (const QString &group : group_order()) {
        groups.append({group, grouped_lines.value(group)});
    }
    setStatusGroups(groups);
}

void VersionEditorWidget::setStatusLines(const QStringList &lines)
{
    setStatusGroups({{group_misc(), lines}});
}

void VersionEditorWidget::setStatusGroups(const QList<QPair<QString, QStringList>> &groups)
{
    if (status_tabs_ == nullptr) {
        return;
    }

    const QString current_title =
        status_tabs_->currentIndex() >= 0 ? status_tabs_->tabText(status_tabs_->currentIndex()) : QString();

    while (status_tabs_->count() > 0) {
        QWidget *page = status_tabs_->widget(0);
        status_tabs_->removeTab(0);
        if (page != nullptr) {
            page->deleteLater();
        }
    }

    int target_index = 0;
    for (const auto &group : groups) {
        auto *editor = new QTextEdit(status_tabs_);
        editor->setReadOnly(true);
        editor->setLineWrapMode(QTextEdit::NoWrap);
        QStringList lines = group.second;
        if (lines.isEmpty()) {
            lines.append(QStringLiteral("Nenhum arquivo neste grupo."));
        }
        editor->setPlainText(lines.join(QLatin1Char('\n')));

        const int index = status_tabs_->addTab(editor, group.first);
        if (!current_title.isEmpty() && group.first == current_title) {
            target_index = index;
        }
    }

    if (status_tabs_->count() > 0) {
        status_tabs_->setCurrentIndex(qBound(0, target_index, status_tabs_->count() - 1));
    }
}

int run_cli_command(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    if (GetStdHandle(STD_OUTPUT_HANDLE) == nullptr && AttachConsole(ATTACH_PARENT_PROCESS)) {
        FILE *stream = nullptr;
        freopen_s(&stream, "CONOUT$", "w", stdout);
        freopen_s(&stream, "CONOUT$", "w", stderr);
    }
#endif

    QCoreApplication app(argc, argv);
    const QStringList args = QCoreApplication::arguments();

    auto arg_value = [&](const QString &flag) -> QString {
        const int idx = args.indexOf(flag);
        if (idx >= 0 && idx + 1 < args.size()) {
            return args.at(idx + 1);
        }
        return QString();
    };

    const bool do_check = args.contains(QStringLiteral("--check"));
    const bool do_apply = args.contains(QStringLiteral("--apply"));
    const QString out_path = arg_value(QStringLiteral("--out"));
    const QString raw_root = arg_value(QStringLiteral("--root"));
    const QString root = raw_root.isEmpty() ? detect_project_root() : normalize_dir(raw_root);

    QStringList report_lines;
    int exit_code = 0;

    if (do_apply) {
        const QString pt_ver = arg_value(QStringLiteral("--pt-version"));
        const QString en_ver = arg_value(QStringLiteral("--en-version")).isEmpty()
                                   ? pt_ver
                                   : arg_value(QStringLiteral("--en-version"));
        const QDate pt_dt = QDate::fromString(arg_value(QStringLiteral("--pt-date")), Qt::ISODate);
        const QDate en_dt = arg_value(QStringLiteral("--en-date")).isEmpty()
                                ? pt_dt
                                : QDate::fromString(arg_value(QStringLiteral("--en-date")), Qt::ISODate);

        if (pt_ver.isEmpty() || !pt_dt.isValid() || !en_dt.isValid()) {
            report_lines.append(QStringLiteral("ERROR: --apply requires --pt-version and valid --pt-date (yyyy-MM-dd)"));
            exit_code = 2;
        } else {
            const UpdateContext ctx = {
                pt_ver,
                QStringLiteral("%1 de %2 de %3")
                    .arg(QString::number(pt_dt.day()),
                         kPtMonthsLower.value(qMax(0, pt_dt.month() - 1)),
                         QString::number(pt_dt.year())),
                en_ver,
                QStringLiteral("%1 %2, %3")
                    .arg(kEnMonths.value(qMax(0, en_dt.month() - 1)),
                         QString::number(en_dt.day()),
                         QString::number(en_dt.year())),
                readme_version_token(pt_ver),
                readme_version_token(en_ver),
            };

            const QList<UpdateResult> results = apply_updates(ctx, root);
            int ok_count = 0;
            int fail_count = 0;
            for (const UpdateResult &r : results) {
                if (r.ok) {
                    ++ok_count;
                } else {
                    ++fail_count;
                    report_lines.append(QStringLiteral("FAIL: %1 (%2) -> %3").arg(r.relative_path, r.key, r.message));
                }
            }
            report_lines.prepend(
                QStringLiteral("APPLIED=%1 OK=%2 FAIL=%3").arg(results.size()).arg(ok_count).arg(fail_count));
            exit_code = (fail_count == 0) ? 0 : 1;
        }
    } else if (do_check) {
        const QList<FileStatus> statuses = check_expected_lines(root);
        int missing_files = 0;
        int missing_patterns = 0;
        for (const FileStatus &s : statuses) {
            if (!s.exists) {
                ++missing_files;
                report_lines.append(QStringLiteral("MISSING_FILE: %1 (%2)").arg(s.relative_path, s.key));
            } else if (!s.missing.isEmpty()) {
                missing_patterns += s.missing.size();
                report_lines.append(QStringLiteral("MISSING_PATTERN: %1 (%2) -> %3")
                                        .arg(s.relative_path, s.key, s.missing.join(QStringLiteral("; "))));
            }
        }
        report_lines.prepend(QStringLiteral("CHECKED=%1 MISSING_FILES=%2 MISSING_PATTERNS=%3")
                                 .arg(statuses.size())
                                 .arg(missing_files)
                                 .arg(missing_patterns));
        exit_code = (missing_files == 0 && missing_patterns == 0) ? 0 : 1;
    }

    const QString report_text = report_lines.join(QLatin1Char('\n')) + QLatin1Char('\n');
    if (!out_path.isEmpty()) {
        write_file_text(out_path, report_text, nullptr);
    }
    const QByteArray utf8 = report_text.toUtf8();
    std::fwrite(utf8.constData(), 1, static_cast<size_t>(utf8.size()), stdout);
    std::fflush(stdout);
    return exit_code;
}

}  // namespace mocks::version_editor

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i) {
        const QString arg = QString::fromLocal8Bit(argv[i]);
        if (arg == QStringLiteral("--check") || arg == QStringLiteral("--apply")) {
            return mocks::version_editor::run_cli_command(argc, argv);
        }
    }

    QApplication app(argc, argv);
    mocks::version_editor::VersionEditorWidget widget;
    widget.show();
    return app.exec();
}
