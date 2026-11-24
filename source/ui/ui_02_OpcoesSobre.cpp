#include "source/ui/ui_02_OpcoesSobre.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "source/utils/ApplicationPathUtils.hpp"
#include "source/utils/LogManager.hpp"

namespace source::GerenciamentoUI {
namespace {

source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

QString load_text(const QString &filename, const QString &folder)
{
    try {
        return source::utils::load_text_file(filename, folder);
    } catch (...) {
        logger()->error(QStringLiteral("Erro ao carregar arquivo de licença: %1").arg(filename));
        return QString();
    }
}

QString filter_plugin_sections(const QString &text,
                               bool /*include_pomodoro_plugin*/ = true,
                               bool /*include_feynman_plugin*/ = true,
                               bool /*include_mapamental_plugin*/ = true,
                               bool /*include_audiolivro_plugin*/ = true,
                               bool /*include_agenda_plugin*/ = true,
                               bool /*include_modelo_plugin*/ = true,
                               bool /*include_gantt_plugin*/ = true)
{
    const QStringList lines = text.split(QLatin1Char('\n'));

    QStringList filtered_lines;
    filtered_lines.reserve(lines.size());

    for (const QString &line : lines) {
        const QString trimmed = line.trimmed();
        if (trimmed.startsWith(QStringLiteral("[[")) && trimmed.endsWith(QStringLiteral("]]"))) {
            continue;
        }
        filtered_lines.append(line);
    }

    return filtered_lines.join(QLatin1Char('\n')).trimmed();
}

QString load_text(const QString &filename,
                  const QString &folder,
                  bool include_pomodoro_plugin,
                  bool include_feynman_plugin,
                  bool include_mapamental_plugin,
                  bool include_audiolivro_plugin,
                  bool include_agenda_plugin = false,
                  bool include_modelo_plugin = false,
                  bool include_gantt_plugin = false)
{
    return filter_plugin_sections(load_text(filename, folder),
                                  include_pomodoro_plugin,
                                  include_feynman_plugin,
                                  include_mapamental_plugin,
                                  include_audiolivro_plugin,
                                  include_agenda_plugin,
                                  include_modelo_plugin,
                                  include_gantt_plugin);
}

void append_documento(QVector<DocumentoSobre> &documentos,
                      const QString &titulo,
                      const QString &filename,
                      const QString &folder,
                      bool include_pomodoro_plugin,
                      bool include_feynman_plugin,
                      bool include_mapamental_plugin,
                      bool include_audiolivro_plugin,
                      bool include_agenda_plugin = false,
                      bool include_modelo_plugin = false,
                      bool include_gantt_plugin = false)
{
    const QString texto = load_text(filename,
                                    folder,
                                    include_pomodoro_plugin,
                                    include_feynman_plugin,
                                    include_mapamental_plugin,
                                    include_audiolivro_plugin,
                                    include_agenda_plugin,
                                    include_modelo_plugin,
                                    include_gantt_plugin);
    if (!texto.trimmed().isEmpty()) {
        documentos.append({titulo, texto});
    }
}

QString document_title(const QString &prefix, const QString &title)
{
    return prefix.isEmpty() ? title : QStringLiteral("%1 - %2").arg(prefix, title);
}

QString translated_label(const QString &key, const QString &fallback)
{
    QString value = QCoreApplication::translate("App", key.toUtf8().constData());
    if (!value.isEmpty() && value != key) {
        return value;
    }
    value = QCoreApplication::translate("InterfaceGrafica", key.toUtf8().constData());
    return (!value.isEmpty() && value != key) ? value : fallback;
}

QString general_tab_title()
{
    return translated_label(QStringLiteral("about_tab_general"), QStringLiteral("Geral"));
}

QString parent_app_tab_title()
{
    return translated_label(QStringLiteral("about_tab_eisenhower"), QStringLiteral("Matriz Eisenhower"));
}

QString pomodoro_tab_title()
{
    return translated_label(QStringLiteral("about_tab_pomodoro"), QStringLiteral("Pomodoro"));
}

QString feynman_tab_title()
{
    return translated_label(QStringLiteral("about_tab_feynman"), QStringLiteral("Feynman"));
}

QString mind_map_tab_title()
{
    return translated_label(QStringLiteral("about_tab_mind_map"), QStringLiteral("Mapa Mental"));
}

QString audiobook_tab_title()
{
    return translated_label(QStringLiteral("about_tab_audiobook"), QStringLiteral("Audiolivro"));
}

QString agenda_tab_title()
{
    return translated_label(QStringLiteral("about_tab_agenda"), QStringLiteral("Agenda"));
}

QString modelo_tab_title()
{
    return translated_label(QStringLiteral("about_tab_modelo"),
                            translated_label(QStringLiteral("about_tab_qwen_model"), QStringLiteral("Modelo Qwen")));
}

QString gantt_tab_title()
{
    return translated_label(QStringLiteral("about_tab_gantt"), QStringLiteral("Gantt BIM 4D"));
}

QString qwen_model_tab_title()
{
    return translated_label(QStringLiteral("about_tab_qwen_model"), QStringLiteral("modelo qwen2.5-3b"));
}

QString load_qwen_model_document(const QString &filename)
{
    const QString app_dir = QCoreApplication::applicationDirPath();
    const QString current_dir = QDir::currentPath();
    const QStringList candidates = {
        QDir(app_dir).filePath(QStringLiteral("plugins/modelo/models/%1").arg(filename)),
        QDir(current_dir).filePath(QStringLiteral("plugins/modelo/models/%1").arg(filename)),
        QDir(current_dir).filePath(
            QStringLiteral("source/modules/modelo/metodoModelo/qwen/models/%1").arg(filename)),
        QDir(app_dir).filePath(
            QStringLiteral("source/modules/modelo/metodoModelo/qwen/models/%1").arg(filename)),
        QDir(app_dir).filePath(
            QStringLiteral("../../../source/modules/modelo/metodoModelo/qwen/models/%1").arg(filename)),
    };

    for (const QString &candidate : candidates) {
        QFile file(QFileInfo(candidate).absoluteFilePath());
        if (file.open(QIODevice::ReadOnly)) {
            return QString::fromUtf8(file.readAll());
        }
    }
    logger()->error(QStringLiteral("Documento do modelo Qwen não encontrado: %1").arg(filename));
    return {};
}

void append_qwen_model_document(QVector<DocumentoSobre> &documentos, const QString &filename)
{
    const QString text = load_qwen_model_document(filename);
    if (!text.trimmed().isEmpty()) {
        documentos.append({qwen_model_tab_title(), text});
    }
}

void append_document_set(QVector<DocumentoSobre> &documentos,
                         const QString &title_prefix,
                         const QString &general_title,
                         const QString &parent_title,
                         const QString &pomodoro_title,
                         const QString &feynman_title,
                         const QString &mapamental_title,
                         const QString &audiolivro_title,
                         const QString &agenda_title,
                         const QString &modelo_title,
                         const QString &gantt_title,
                         const QString &general_filename,
                         const QString &parent_filename,
                         const QString &pomodoro_filename,
                         const QString &feynman_filename,
                         const QString &mapamental_filename,
                         const QString &audiolivro_filename,
                         const QString &agenda_filename,
                         const QString &modelo_filename,
                         const QString &gantt_filename,
                         const QString &folder,
                         bool include_pomodoro_plugin,
                         bool include_feynman_plugin,
                         bool include_mapamental_plugin,
                         bool include_audiolivro_plugin,
                         bool include_agenda_plugin,
                         bool include_modelo_plugin,
                         bool include_gantt_plugin)
{
    append_documento(documentos,
                     document_title(title_prefix, general_title),
                     general_filename,
                     folder,
                     include_pomodoro_plugin,
                     include_feynman_plugin,
                     include_mapamental_plugin,
                     include_audiolivro_plugin,
                     include_agenda_plugin,
                     include_modelo_plugin,
                     include_gantt_plugin);
    append_documento(documentos,
                     document_title(title_prefix, parent_title),
                     parent_filename,
                     folder,
                     include_pomodoro_plugin,
                     include_feynman_plugin,
                     include_mapamental_plugin,
                     include_audiolivro_plugin,
                     include_agenda_plugin,
                     include_modelo_plugin,
                     include_gantt_plugin);
    if (include_pomodoro_plugin) {
        append_documento(documentos,
                         document_title(title_prefix, pomodoro_title),
                         pomodoro_filename,
                         folder,
                         include_pomodoro_plugin,
                         include_feynman_plugin,
                         include_mapamental_plugin,
                         include_audiolivro_plugin,
                         include_agenda_plugin,
                         include_modelo_plugin,
                         include_gantt_plugin);
    }
    if (include_feynman_plugin) {
        append_documento(documentos,
                         document_title(title_prefix, feynman_title),
                         feynman_filename,
                         folder,
                         include_pomodoro_plugin,
                         include_feynman_plugin,
                         include_mapamental_plugin,
                         include_audiolivro_plugin,
                         include_agenda_plugin,
                         include_modelo_plugin,
                         include_gantt_plugin);
    }
    if (include_mapamental_plugin) {
        append_documento(documentos,
                         document_title(title_prefix, mapamental_title),
                         mapamental_filename,
                         folder,
                         include_pomodoro_plugin,
                         include_feynman_plugin,
                         include_mapamental_plugin,
                         include_audiolivro_plugin,
                         include_agenda_plugin,
                         include_modelo_plugin,
                         include_gantt_plugin);
    }
    if (include_audiolivro_plugin) {
        append_documento(documentos,
                         document_title(title_prefix, audiolivro_title),
                         audiolivro_filename,
                         folder,
                         include_pomodoro_plugin,
                         include_feynman_plugin,
                         include_mapamental_plugin,
                         include_audiolivro_plugin,
                         include_agenda_plugin,
                         include_modelo_plugin,
                         include_gantt_plugin);
    }
    if (include_agenda_plugin) {
        append_documento(documentos,
                         document_title(title_prefix, agenda_title),
                         agenda_filename,
                         folder,
                         include_pomodoro_plugin,
                         include_feynman_plugin,
                         include_mapamental_plugin,
                         include_audiolivro_plugin,
                         include_agenda_plugin,
                         include_modelo_plugin,
                         include_gantt_plugin);
    }
    if (include_modelo_plugin) {
        append_documento(documentos,
                         document_title(title_prefix, modelo_title),
                         modelo_filename,
                         folder,
                         include_pomodoro_plugin,
                         include_feynman_plugin,
                         include_mapamental_plugin,
                         include_audiolivro_plugin,
                         include_agenda_plugin,
                         include_modelo_plugin,
                         include_gantt_plugin);
    }
    if (include_gantt_plugin) {
        append_documento(documentos,
                         document_title(title_prefix, gantt_title),
                         gantt_filename,
                         folder,
                         include_pomodoro_plugin,
                         include_feynman_plugin,
                         include_mapamental_plugin,
                         include_audiolivro_plugin,
                         include_agenda_plugin,
                         include_modelo_plugin,
                         include_gantt_plugin);
    }
}

}  // namespace

const QString &OpcoesSobre::SITE_LICENSES()
{
    static const QString value = QStringLiteral(
        "GNU Affero General Public License, Version 3|GNU AFFERO GENERAL PUBLIC LICENSE Version 3.txt|https://www.gnu.org/licenses/agpl-3.0.html\n"
        "GNU General Public License, Version 3|GPL-3.0.txt|https://www.gnu.org/licenses/gpl-3.0.html\n"
        "GNU Lesser General Public License, Version 3|GNU LESSER GENERAL PUBLIC LICENSE Version 3.txt|https://www.gnu.org/licenses/lgpl-3.0.html\n"
        "GNU Lesser General Public License, Version 2.1|GNU LESSER GENERAL PUBLIC LICENSE Version 2.1.txt|https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html\n"
        "GCC Runtime Library Exception, Version 3.1|GCC Runtime Library Exception 3.1.txt|https://www.gnu.org/licenses/gcc-exception-3.1.html\n"
        "BSD 2-Clause License|BSD License Clause-2.txt|https://opensource.org/licenses/BSD-2-Clause\n"
        "BSD 3-Clause License|BSD License Clause-3.txt|https://opensource.org/licenses/BSD-3-Clause\n"
        "Apache License, Version 2.0|Apache License Version 2.0.txt|https://www.apache.org/licenses/LICENSE-2.0\n"
        "Python Software Foundation License|PYTHON SOFTWARE FOUNDATION LICENSE.txt|https://docs.python.org/3/license.html\n"
        "MIT License|MIT License.txt|https://opensource.org/licenses/MIT\n"
        "ISC License|ISC LICENSE.txt|https://opensource.org/license/isc-license-txt\n"
        "FFmpeg LGPL Build Configuration and Legal Checklist|FFmpeg_LGPL_BUILD_CONFIGURATION.txt|https://ffmpeg.org/legal.html\n"
        "llama.cpp MIT License|llama.cpp MIT License.txt|https://raw.githubusercontent.com/ggml-org/llama.cpp/master/LICENSE\n"
        "LLVM OpenMP Runtime License|LLVM OpenMP License.txt|https://raw.githubusercontent.com/llvm/llvm-project/main/openmp/LICENSE.TXT\n"
        "MinGW-w64 Runtime License|MinGW-w64 Runtime License.txt|https://raw.githubusercontent.com/mingw-w64/mingw-w64/master/COPYING.MinGW-w64-runtime/COPYING.MinGW-w64-runtime.txt\n"
        "DirectXShaderCompiler License|DirectXShaderCompiler License.txt|https://raw.githubusercontent.com/microsoft/DirectXShaderCompiler/main/LICENSE.TXT\n"
        "SQLite Blessing / Public Domain Dedication|SQLite Blessing.txt|https://www.sqlite.org/copyright.html\n"
        "Zlib License|Zlib License.txt|https://zlib.net/zlib_license.html\n"
        "Microsoft Visual Studio Redistributable Runtime Notice|Microsoft Redistributable Runtime Notice.txt|https://learn.microsoft.com/visualstudio/releases/2022/redistribution\n");
    return value;
}

QString OpcoesSobre::LICENSE_TEXT_PT_BR(bool include_pomodoro_plugin,
                                        bool include_feynman_plugin,
                                        bool include_mapamental_plugin,
                                        bool include_audiolivro_plugin,
                                        bool include_agenda_plugin,
                                        bool include_modelo_plugin,
                                        bool include_gantt_plugin)
{
    return load_text(QStringLiteral("EULA_pt_BR.txt"),
                     QStringLiteral("EULA"),
                     include_pomodoro_plugin,
                     include_feynman_plugin,
                     include_mapamental_plugin,
                     include_audiolivro_plugin,
                     include_agenda_plugin,
                     include_modelo_plugin,
                     include_gantt_plugin);
}

QString OpcoesSobre::LICENSE_TEXT_EN_US(bool include_pomodoro_plugin,
                                        bool include_feynman_plugin,
                                        bool include_mapamental_plugin,
                                        bool include_audiolivro_plugin,
                                        bool include_agenda_plugin,
                                        bool include_modelo_plugin,
                                        bool include_gantt_plugin)
{
    return load_text(QStringLiteral("EULA_en_US.txt"),
                     QStringLiteral("EULA"),
                     include_pomodoro_plugin,
                     include_feynman_plugin,
                     include_mapamental_plugin,
                     include_audiolivro_plugin,
                     include_agenda_plugin,
                     include_modelo_plugin,
                     include_gantt_plugin);
}

QString OpcoesSobre::NOTICE_TEXT_PT_BR(bool include_pomodoro_plugin,
                                       bool include_feynman_plugin,
                                       bool include_mapamental_plugin,
                                       bool include_audiolivro_plugin,
                                       bool include_agenda_plugin,
                                       bool include_modelo_plugin,
                                       bool include_gantt_plugin)
{
    return load_text(QStringLiteral("NOTICE_pt_BR.txt"),
                     QStringLiteral("NOTICES"),
                     include_pomodoro_plugin,
                     include_feynman_plugin,
                     include_mapamental_plugin,
                     include_audiolivro_plugin,
                     include_agenda_plugin,
                     include_modelo_plugin,
                     include_gantt_plugin);
}

QString OpcoesSobre::NOTICE_TEXT_EN_US(bool include_pomodoro_plugin,
                                       bool include_feynman_plugin,
                                       bool include_mapamental_plugin,
                                       bool include_audiolivro_plugin,
                                       bool include_agenda_plugin,
                                       bool include_modelo_plugin,
                                       bool include_gantt_plugin)
{
    return load_text(QStringLiteral("NOTICE_en_US.txt"),
                     QStringLiteral("NOTICES"),
                     include_pomodoro_plugin,
                     include_feynman_plugin,
                     include_mapamental_plugin,
                     include_audiolivro_plugin,
                     include_agenda_plugin,
                     include_modelo_plugin,
                     include_gantt_plugin);
}

QString OpcoesSobre::ABOUT_TEXT_PT_BR(bool include_pomodoro_plugin,
                                      bool include_feynman_plugin,
                                      bool include_mapamental_plugin,
                                      bool include_audiolivro_plugin,
                                      bool include_agenda_plugin,
                                      bool include_modelo_plugin,
                                      bool include_gantt_plugin)
{
    return load_text(QStringLiteral("ABOUT_pt_BR.txt"),
                     QStringLiteral("ABOUT"),
                     include_pomodoro_plugin,
                     include_feynman_plugin,
                     include_mapamental_plugin,
                     include_audiolivro_plugin,
                     include_agenda_plugin,
                     include_modelo_plugin,
                     include_gantt_plugin);
}

QString OpcoesSobre::ABOUT_TEXT_EN_US(bool include_pomodoro_plugin,
                                      bool include_feynman_plugin,
                                      bool include_mapamental_plugin,
                                      bool include_audiolivro_plugin,
                                      bool include_agenda_plugin,
                                      bool include_modelo_plugin,
                                      bool include_gantt_plugin)
{
    return load_text(QStringLiteral("ABOUT_en_US.txt"),
                     QStringLiteral("ABOUT"),
                     include_pomodoro_plugin,
                     include_feynman_plugin,
                     include_mapamental_plugin,
                     include_audiolivro_plugin,
                     include_agenda_plugin,
                     include_modelo_plugin,
                     include_gantt_plugin);
}

QVector<DocumentoSobre> OpcoesSobre::About_Documents_pt_BR(bool include_pomodoro_plugin,
                                                           bool include_feynman_plugin,
                                                           bool include_mapamental_plugin,
                                                           bool include_audiolivro_plugin,
                                                           bool include_agenda_plugin,
                                                           bool include_modelo_plugin,
                                                           bool include_gantt_plugin)
{
    QVector<DocumentoSobre> documentos;
    append_document_set(documentos,
                        QString(),
                        general_tab_title(),
                        parent_app_tab_title(),
                        pomodoro_tab_title(),
                        feynman_tab_title(),
                        mind_map_tab_title(),
                        audiobook_tab_title(),
                        agenda_tab_title(),
                        modelo_tab_title(),
                        gantt_tab_title(),
                        QStringLiteral("ABOUT_pt_BR.txt"),
                        QStringLiteral("ABOUT_Eisenhower_Organizer_pt_BR.txt"),
                        QStringLiteral("ABOUT_plugin_pomodoro_pt_BR.txt"),
                        QStringLiteral("ABOUT_plugin_feynman_pt_BR.txt"),
                        QStringLiteral("ABOUT_plugin_mapamental_pt_BR.txt"),
                        QStringLiteral("ABOUT_plugin_audiolivro_pt_BR.txt"),
                        QStringLiteral("ABOUT_plugin_agenda_pt_BR.txt"),
                        QStringLiteral("ABOUT_plugin_modelo_pt_BR.txt"),
                        QStringLiteral("ABOUT_plugin_gantt_pt_BR.txt"),
                        QStringLiteral("ABOUT"),
                        include_pomodoro_plugin,
                        include_feynman_plugin,
                        include_mapamental_plugin,
                        include_audiolivro_plugin,
                        include_agenda_plugin,
                        include_modelo_plugin,
                        include_gantt_plugin);
    if (include_modelo_plugin) {
        append_qwen_model_document(documentos, QStringLiteral("LEIAME.txt"));
    }
    return documentos;
}

QVector<DocumentoSobre> OpcoesSobre::About_Documents_en_US(bool include_pomodoro_plugin,
                                                           bool include_feynman_plugin,
                                                           bool include_mapamental_plugin,
                                                           bool include_audiolivro_plugin,
                                                           bool include_agenda_plugin,
                                                           bool include_modelo_plugin,
                                                           bool include_gantt_plugin)
{
    QVector<DocumentoSobre> documentos;
    append_document_set(documentos,
                        QString(),
                        general_tab_title(),
                        parent_app_tab_title(),
                        pomodoro_tab_title(),
                        feynman_tab_title(),
                        mind_map_tab_title(),
                        audiobook_tab_title(),
                        agenda_tab_title(),
                        modelo_tab_title(),
                        gantt_tab_title(),
                        QStringLiteral("ABOUT_en_US.txt"),
                        QStringLiteral("ABOUT_Eisenhower_Organizer_en_US.txt"),
                        QStringLiteral("ABOUT_plugin_pomodoro_en_US.txt"),
                        QStringLiteral("ABOUT_plugin_feynman_en_US.txt"),
                        QStringLiteral("ABOUT_plugin_mapamental_en_US.txt"),
                        QStringLiteral("ABOUT_plugin_audiolivro_en_US.txt"),
                        QStringLiteral("ABOUT_plugin_agenda_en_US.txt"),
                        QStringLiteral("ABOUT_plugin_modelo_en_US.txt"),
                        QStringLiteral("ABOUT_plugin_gantt_en_US.txt"),
                        QStringLiteral("ABOUT"),
                        include_pomodoro_plugin,
                        include_feynman_plugin,
                        include_mapamental_plugin,
                        include_audiolivro_plugin,
                        include_agenda_plugin,
                        include_modelo_plugin,
                        include_gantt_plugin);
    if (include_modelo_plugin) {
        append_qwen_model_document(documentos, QStringLiteral("README.txt"));
    }
    return documentos;
}

QVector<DocumentoSobre> OpcoesSobre::License_Documents_pt_BR(bool include_pomodoro_plugin,
                                                             bool include_feynman_plugin,
                                                             bool include_mapamental_plugin,
                                                             bool include_audiolivro_plugin,
                                                             bool include_agenda_plugin,
                                                             bool include_modelo_plugin,
                                                             bool include_gantt_plugin)
{
    QVector<DocumentoSobre> documentos;
    append_document_set(documentos,
                        QString(),
                        general_tab_title(),
                        parent_app_tab_title(),
                        pomodoro_tab_title(),
                        feynman_tab_title(),
                        mind_map_tab_title(),
                        audiobook_tab_title(),
                        agenda_tab_title(),
                        modelo_tab_title(),
                        gantt_tab_title(),
                        QStringLiteral("EULA_pt_BR.txt"),
                        QStringLiteral("EULA_Eisenhower_Organizer_pt_BR.txt"),
                        QStringLiteral("EULA_plugin_pomodoro_pt_BR.txt"),
                        QStringLiteral("EULA_plugin_feynman_pt_BR.txt"),
                        QStringLiteral("EULA_plugin_mapamental_pt_BR.txt"),
                        QStringLiteral("EULA_plugin_audiolivro_pt_BR.txt"),
                        QStringLiteral("EULA_plugin_agenda_pt_BR.txt"),
                        QStringLiteral("EULA_plugin_modelo_pt_BR.txt"),
                        QStringLiteral("EULA_plugin_gantt_pt_BR.txt"),
                        QStringLiteral("EULA"),
                        include_pomodoro_plugin,
                        include_feynman_plugin,
                        include_mapamental_plugin,
                        include_audiolivro_plugin,
                        include_agenda_plugin,
                        include_modelo_plugin,
                        include_gantt_plugin);
    if (include_modelo_plugin) {
        append_qwen_model_document(documentos, QStringLiteral("LICENCA_QWEN2.5.txt"));
    }
    return documentos;
}

QVector<DocumentoSobre> OpcoesSobre::License_Documents_en_US(bool include_pomodoro_plugin,
                                                             bool include_feynman_plugin,
                                                             bool include_mapamental_plugin,
                                                             bool include_audiolivro_plugin,
                                                             bool include_agenda_plugin,
                                                             bool include_modelo_plugin,
                                                             bool include_gantt_plugin)
{
    QVector<DocumentoSobre> documentos;
    append_document_set(documentos,
                        QString(),
                        general_tab_title(),
                        parent_app_tab_title(),
                        pomodoro_tab_title(),
                        feynman_tab_title(),
                        mind_map_tab_title(),
                        audiobook_tab_title(),
                        agenda_tab_title(),
                        modelo_tab_title(),
                        gantt_tab_title(),
                        QStringLiteral("EULA_en_US.txt"),
                        QStringLiteral("EULA_Eisenhower_Organizer_en_US.txt"),
                        QStringLiteral("EULA_plugin_pomodoro_en_US.txt"),
                        QStringLiteral("EULA_plugin_feynman_en_US.txt"),
                        QStringLiteral("EULA_plugin_mapamental_en_US.txt"),
                        QStringLiteral("EULA_plugin_audiolivro_en_US.txt"),
                        QStringLiteral("EULA_plugin_agenda_en_US.txt"),
                        QStringLiteral("EULA_plugin_modelo_en_US.txt"),
                        QStringLiteral("EULA_plugin_gantt_en_US.txt"),
                        QStringLiteral("EULA"),
                        include_pomodoro_plugin,
                        include_feynman_plugin,
                        include_mapamental_plugin,
                        include_audiolivro_plugin,
                        include_agenda_plugin,
                        include_modelo_plugin,
                        include_gantt_plugin);
    if (include_modelo_plugin) {
        append_qwen_model_document(documentos, QStringLiteral("LICENSE_QWEN2.5.txt"));
    }
    return documentos;
}

QVector<DocumentoSobre> OpcoesSobre::Contract_Documents_pt_BR(bool include_pomodoro_plugin,
                                                              bool include_feynman_plugin,
                                                              bool include_mapamental_plugin,
                                                              bool include_audiolivro_plugin,
                                                              bool include_agenda_plugin,
                                                              bool include_modelo_plugin,
                                                              bool include_gantt_plugin)
{
    QVector<DocumentoSobre> documentos;
    append_document_set(documentos,
                        QString(),
                        general_tab_title(),
                        parent_app_tab_title(),
                        pomodoro_tab_title(),
                        feynman_tab_title(),
                        mind_map_tab_title(),
                        audiobook_tab_title(),
                        agenda_tab_title(),
                        modelo_tab_title(),
                        gantt_tab_title(),
                        QStringLiteral("CLC_pt_BR.txt"),
                        QStringLiteral("CLC_Eisenhower_Organizer_pt_BR.txt"),
                        QStringLiteral("CLC_plugin_pomodoro_pt_BR.txt"),
                        QStringLiteral("CLC_plugin_feynman_pt_BR.txt"),
                        QStringLiteral("CLC_plugin_mapamental_pt_BR.txt"),
                        QStringLiteral("CLC_plugin_audiolivro_pt_BR.txt"),
                        QStringLiteral("CLC_plugin_agenda_pt_BR.txt"),
                        QStringLiteral("CLC_plugin_modelo_pt_BR.txt"),
                        QStringLiteral("CLC_plugin_gantt_pt_BR.txt"),
                        QStringLiteral("CLC"),
                        include_pomodoro_plugin,
                        include_feynman_plugin,
                        include_mapamental_plugin,
                        include_audiolivro_plugin,
                        include_agenda_plugin,
                        include_modelo_plugin,
                        include_gantt_plugin);
    return documentos;
}

QVector<DocumentoSobre> OpcoesSobre::Contract_Documents_en_US(bool include_pomodoro_plugin,
                                                              bool include_feynman_plugin,
                                                              bool include_mapamental_plugin,
                                                              bool include_audiolivro_plugin,
                                                              bool include_agenda_plugin,
                                                              bool include_modelo_plugin,
                                                              bool include_gantt_plugin)
{
    QVector<DocumentoSobre> documentos;
    append_document_set(documentos,
                        QString(),
                        general_tab_title(),
                        parent_app_tab_title(),
                        pomodoro_tab_title(),
                        feynman_tab_title(),
                        mind_map_tab_title(),
                        audiobook_tab_title(),
                        agenda_tab_title(),
                        modelo_tab_title(),
                        gantt_tab_title(),
                        QStringLiteral("CLC_en_US.txt"),
                        QStringLiteral("CLC_Eisenhower_Organizer_en_US.txt"),
                        QStringLiteral("CLC_plugin_pomodoro_en_US.txt"),
                        QStringLiteral("CLC_plugin_feynman_en_US.txt"),
                        QStringLiteral("CLC_plugin_mapamental_en_US.txt"),
                        QStringLiteral("CLC_plugin_audiolivro_en_US.txt"),
                        QStringLiteral("CLC_plugin_agenda_en_US.txt"),
                        QStringLiteral("CLC_plugin_modelo_en_US.txt"),
                        QStringLiteral("CLC_plugin_gantt_en_US.txt"),
                        QStringLiteral("CLC"),
                        include_pomodoro_plugin,
                        include_feynman_plugin,
                        include_mapamental_plugin,
                        include_audiolivro_plugin,
                        include_agenda_plugin,
                        include_modelo_plugin,
                        include_gantt_plugin);
    return documentos;
}

QVector<DocumentoSobre> OpcoesSobre::Copyright_Documents_pt_BR(bool include_pomodoro_plugin,
                                                               bool include_feynman_plugin,
                                                               bool include_mapamental_plugin,
                                                               bool include_audiolivro_plugin,
                                                               bool include_agenda_plugin,
                                                               bool include_modelo_plugin,
                                                               bool include_gantt_plugin)
{
    QVector<DocumentoSobre> documentos;
    append_document_set(documentos,
                        QString(),
                        general_tab_title(),
                        parent_app_tab_title(),
                        pomodoro_tab_title(),
                        feynman_tab_title(),
                        mind_map_tab_title(),
                        audiobook_tab_title(),
                        agenda_tab_title(),
                        modelo_tab_title(),
                        gantt_tab_title(),
                        QStringLiteral("AVISO DE COPYRIGHT E MARCA REGISTRA_pt_BR.txt"),
                        QStringLiteral("Copyright_Notice_Eisenhower_Organizer_pt_BR.txt"),
                        QStringLiteral("Copyright_Notice_plugin_pomodoro_pt_BR.txt"),
                        QStringLiteral("Copyright_Notice_plugin_feynman_pt_BR.txt"),
                        QStringLiteral("Copyright_Notice_plugin_mapamental_pt_BR.txt"),
                        QStringLiteral("Copyright_Notice_plugin_audiolivro_pt_BR.txt"),
                        QStringLiteral("Copyright_Notice_plugin_agenda_pt_BR.txt"),
                        QStringLiteral("Copyright_Notice_plugin_modelo_pt_BR.txt"),
                        QStringLiteral("Copyright_Notice_plugin_gantt_pt_BR.txt"),
                        QStringLiteral("COPYRIGHT"),
                        include_pomodoro_plugin,
                        include_feynman_plugin,
                        include_mapamental_plugin,
                        include_audiolivro_plugin,
                        include_agenda_plugin,
                        include_modelo_plugin,
                        include_gantt_plugin);
    return documentos;
}

QVector<DocumentoSobre> OpcoesSobre::Copyright_Documents_en_US(bool include_pomodoro_plugin,
                                                               bool include_feynman_plugin,
                                                               bool include_mapamental_plugin,
                                                               bool include_audiolivro_plugin,
                                                               bool include_agenda_plugin,
                                                               bool include_modelo_plugin,
                                                               bool include_gantt_plugin)
{
    QVector<DocumentoSobre> documentos;
    append_document_set(documentos,
                        QString(),
                        general_tab_title(),
                        parent_app_tab_title(),
                        pomodoro_tab_title(),
                        feynman_tab_title(),
                        mind_map_tab_title(),
                        audiobook_tab_title(),
                        agenda_tab_title(),
                        modelo_tab_title(),
                        gantt_tab_title(),
                        QStringLiteral("COPYRIGHT AND TRADEMARK NOTICE_en_US.txt"),
                        QStringLiteral("Copyright_Notice_Eisenhower_Organizer_en_US.txt"),
                        QStringLiteral("Copyright_Notice_plugin_pomodoro_en_US.txt"),
                        QStringLiteral("Copyright_Notice_plugin_feynman_en_US.txt"),
                        QStringLiteral("Copyright_Notice_plugin_mapamental_en_US.txt"),
                        QStringLiteral("Copyright_Notice_plugin_audiolivro_en_US.txt"),
                        QStringLiteral("Copyright_Notice_plugin_agenda_en_US.txt"),
                        QStringLiteral("Copyright_Notice_plugin_modelo_en_US.txt"),
                        QStringLiteral("Copyright_Notice_plugin_gantt_en_US.txt"),
                        QStringLiteral("COPYRIGHT"),
                        include_pomodoro_plugin,
                        include_feynman_plugin,
                        include_mapamental_plugin,
                        include_audiolivro_plugin,
                        include_agenda_plugin,
                        include_modelo_plugin,
                        include_gantt_plugin);
    return documentos;
}

QVector<DocumentoSobre> OpcoesSobre::Notice_Documents_pt_BR(bool include_pomodoro_plugin,
                                                            bool include_feynman_plugin,
                                                            bool include_mapamental_plugin,
                                                            bool include_audiolivro_plugin,
                                                            bool include_agenda_plugin,
                                                            bool include_modelo_plugin,
                                                            bool include_gantt_plugin)
{
    QVector<DocumentoSobre> documentos;
    append_document_set(documentos,
                        QString(),
                        general_tab_title(),
                        parent_app_tab_title(),
                        pomodoro_tab_title(),
                        feynman_tab_title(),
                        mind_map_tab_title(),
                        audiobook_tab_title(),
                        agenda_tab_title(),
                        modelo_tab_title(),
                        gantt_tab_title(),
                        QStringLiteral("NOTICE_pt_BR.txt"),
                        QStringLiteral("NOTICE_Eisenhower_Organizer_pt_BR.txt"),
                        QStringLiteral("NOTICE_plugin_pomodoro_pt_BR.txt"),
                        QStringLiteral("NOTICE_plugin_feynman_pt_BR.txt"),
                        QStringLiteral("NOTICE_plugin_mapamental_pt_BR.txt"),
                        QStringLiteral("NOTICE_plugin_audiolivro_pt_BR.txt"),
                        QStringLiteral("NOTICE_plugin_agenda_pt_BR.txt"),
                        QStringLiteral("NOTICE_plugin_modelo_pt_BR.txt"),
                        QStringLiteral("NOTICE_plugin_gantt_pt_BR.txt"),
                        QStringLiteral("NOTICES"),
                        include_pomodoro_plugin,
                        include_feynman_plugin,
                        include_mapamental_plugin,
                        include_audiolivro_plugin,
                        include_agenda_plugin,
                        include_modelo_plugin,
                        include_gantt_plugin);
    return documentos;
}

QVector<DocumentoSobre> OpcoesSobre::Notice_Documents_en_US(bool include_pomodoro_plugin,
                                                            bool include_feynman_plugin,
                                                            bool include_mapamental_plugin,
                                                            bool include_audiolivro_plugin,
                                                            bool include_agenda_plugin,
                                                            bool include_modelo_plugin,
                                                            bool include_gantt_plugin)
{
    QVector<DocumentoSobre> documentos;
    append_document_set(documentos,
                        QString(),
                        general_tab_title(),
                        parent_app_tab_title(),
                        pomodoro_tab_title(),
                        feynman_tab_title(),
                        mind_map_tab_title(),
                        audiobook_tab_title(),
                        agenda_tab_title(),
                        modelo_tab_title(),
                        gantt_tab_title(),
                        QStringLiteral("NOTICE_en_US.txt"),
                        QStringLiteral("NOTICE_Eisenhower_Organizer_en_US.txt"),
                        QStringLiteral("NOTICE_plugin_pomodoro_en_US.txt"),
                        QStringLiteral("NOTICE_plugin_feynman_en_US.txt"),
                        QStringLiteral("NOTICE_plugin_mapamental_en_US.txt"),
                        QStringLiteral("NOTICE_plugin_audiolivro_en_US.txt"),
                        QStringLiteral("NOTICE_plugin_agenda_en_US.txt"),
                        QStringLiteral("NOTICE_plugin_modelo_en_US.txt"),
                        QStringLiteral("NOTICE_plugin_gantt_en_US.txt"),
                        QStringLiteral("NOTICES"),
                        include_pomodoro_plugin,
                        include_feynman_plugin,
                        include_mapamental_plugin,
                        include_audiolivro_plugin,
                        include_agenda_plugin,
                        include_modelo_plugin,
                        include_gantt_plugin);
    return documentos;
}

QString OpcoesSobre::Privacy_Policy_pt_BR(bool include_pomodoro_plugin,
                                          bool include_feynman_plugin,
                                          bool include_mapamental_plugin,
                                          bool include_audiolivro_plugin,
                                          bool include_agenda_plugin,
                                          bool include_modelo_plugin,
                                          bool include_gantt_plugin)
{
    return load_text(QStringLiteral("Privacy_Policy_pt_BR.txt"),
                     QStringLiteral("PRIVACY_POLICY"),
                     include_pomodoro_plugin,
                     include_feynman_plugin,
                     include_mapamental_plugin,
                     include_audiolivro_plugin,
                     include_agenda_plugin,
                     include_modelo_plugin,
                     include_gantt_plugin);
}

QString OpcoesSobre::Privacy_Policy_en_US(bool include_pomodoro_plugin,
                                          bool include_feynman_plugin,
                                          bool include_mapamental_plugin,
                                          bool include_audiolivro_plugin,
                                          bool include_agenda_plugin,
                                          bool include_modelo_plugin,
                                          bool include_gantt_plugin)
{
    return load_text(QStringLiteral("Privacy_Policy_en_US.txt"),
                     QStringLiteral("PRIVACY_POLICY"),
                     include_pomodoro_plugin,
                     include_feynman_plugin,
                     include_mapamental_plugin,
                     include_audiolivro_plugin,
                     include_agenda_plugin,
                     include_modelo_plugin,
                     include_gantt_plugin);
}

QVector<DocumentoSobre> OpcoesSobre::Privacy_Policy_Documents_pt_BR(bool include_pomodoro_plugin,
                                                                    bool include_feynman_plugin,
                                                                    bool include_mapamental_plugin,
                                                                    bool include_audiolivro_plugin,
                                                                    bool include_agenda_plugin,
                                                                    bool include_modelo_plugin,
                                                                    bool include_gantt_plugin)
{
    QVector<DocumentoSobre> documentos;
    append_document_set(documentos,
                        QString(),
                        general_tab_title(),
                        parent_app_tab_title(),
                        pomodoro_tab_title(),
                        feynman_tab_title(),
                        mind_map_tab_title(),
                        audiobook_tab_title(),
                        agenda_tab_title(),
                        modelo_tab_title(),
                        gantt_tab_title(),
                        QStringLiteral("Privacy_Policy_pt_BR.txt"),
                        QStringLiteral("Privacy_Policy_Eisenhower_Organizer_pt_BR.txt"),
                        QStringLiteral("Privacy_Policy_plugin_pomodoro_pt_BR.txt"),
                        QStringLiteral("Privacy_Policy_plugin_feynman_pt_BR.txt"),
                        QStringLiteral("Privacy_Policy_plugin_mapamental_pt_BR.txt"),
                        QStringLiteral("Privacy_Policy_plugin_audiolivro_pt_BR.txt"),
                        QStringLiteral("Privacy_Policy_plugin_agenda_pt_BR.txt"),
                        QStringLiteral("Privacy_Policy_plugin_modelo_pt_BR.txt"),
                        QStringLiteral("Privacy_Policy_plugin_gantt_pt_BR.txt"),
                        QStringLiteral("PRIVACY_POLICY"),
                        include_pomodoro_plugin,
                        include_feynman_plugin,
                        include_mapamental_plugin,
                        include_audiolivro_plugin,
                        include_agenda_plugin,
                        include_modelo_plugin,
                        include_gantt_plugin);
    return documentos;
}

QVector<DocumentoSobre> OpcoesSobre::Privacy_Policy_Documents_en_US(bool include_pomodoro_plugin,
                                                                    bool include_feynman_plugin,
                                                                    bool include_mapamental_plugin,
                                                                    bool include_audiolivro_plugin,
                                                                    bool include_agenda_plugin,
                                                                    bool include_modelo_plugin,
                                                                    bool include_gantt_plugin)
{
    QVector<DocumentoSobre> documentos;
    append_document_set(documentos,
                        QString(),
                        general_tab_title(),
                        parent_app_tab_title(),
                        pomodoro_tab_title(),
                        feynman_tab_title(),
                        mind_map_tab_title(),
                        audiobook_tab_title(),
                        agenda_tab_title(),
                        modelo_tab_title(),
                        gantt_tab_title(),
                        QStringLiteral("Privacy_Policy_en_US.txt"),
                        QStringLiteral("Privacy_Policy_Eisenhower_Organizer_en_US.txt"),
                        QStringLiteral("Privacy_Policy_plugin_pomodoro_en_US.txt"),
                        QStringLiteral("Privacy_Policy_plugin_feynman_en_US.txt"),
                        QStringLiteral("Privacy_Policy_plugin_mapamental_en_US.txt"),
                        QStringLiteral("Privacy_Policy_plugin_audiolivro_en_US.txt"),
                        QStringLiteral("Privacy_Policy_plugin_agenda_en_US.txt"),
                        QStringLiteral("Privacy_Policy_plugin_modelo_en_US.txt"),
                        QStringLiteral("Privacy_Policy_plugin_gantt_en_US.txt"),
                        QStringLiteral("PRIVACY_POLICY"),
                        include_pomodoro_plugin,
                        include_feynman_plugin,
                        include_mapamental_plugin,
                        include_audiolivro_plugin,
                        include_agenda_plugin,
                        include_modelo_plugin,
                        include_gantt_plugin);
    return documentos;
}

QString OpcoesSobre::History_APP_pt_BR(bool include_pomodoro_plugin,
                                       bool include_feynman_plugin,
                                       bool include_mapamental_plugin,
                                       bool include_audiolivro_plugin,
                                       bool include_agenda_plugin,
                                       bool include_modelo_plugin,
                                       bool include_gantt_plugin)
{
    return load_text(QStringLiteral("History_APP_pt_BR.txt"),
                     QStringLiteral("ABOUT"),
                     include_pomodoro_plugin,
                     include_feynman_plugin,
                     include_mapamental_plugin,
                     include_audiolivro_plugin,
                     include_agenda_plugin,
                     include_modelo_plugin,
                     include_gantt_plugin);
}

QString OpcoesSobre::History_APP_en_US(bool include_pomodoro_plugin,
                                       bool include_feynman_plugin,
                                       bool include_mapamental_plugin,
                                       bool include_audiolivro_plugin,
                                       bool include_agenda_plugin,
                                       bool include_modelo_plugin,
                                       bool include_gantt_plugin)
{
    return load_text(QStringLiteral("History_APP_en_US.txt"),
                     QStringLiteral("ABOUT"),
                     include_pomodoro_plugin,
                     include_feynman_plugin,
                     include_mapamental_plugin,
                     include_audiolivro_plugin,
                     include_agenda_plugin,
                     include_modelo_plugin,
                     include_gantt_plugin);
}

QString OpcoesSobre::RELEASE_NOTES_pt_BR(bool include_pomodoro_plugin,
                                         bool include_feynman_plugin,
                                         bool include_mapamental_plugin,
                                         bool include_audiolivro_plugin,
                                         bool include_agenda_plugin,
                                         bool include_modelo_plugin,
                                         bool include_gantt_plugin)
{
    return load_text(QStringLiteral("RELEASE NOTES_pt_BR.txt"),
                     QStringLiteral("RELEASE"),
                     include_pomodoro_plugin,
                     include_feynman_plugin,
                     include_mapamental_plugin,
                     include_audiolivro_plugin,
                     include_agenda_plugin,
                     include_modelo_plugin,
                     include_gantt_plugin);
}

QString OpcoesSobre::RELEASE_NOTES_en_US(bool include_pomodoro_plugin,
                                         bool include_feynman_plugin,
                                         bool include_mapamental_plugin,
                                         bool include_audiolivro_plugin,
                                         bool include_agenda_plugin,
                                         bool include_modelo_plugin,
                                         bool include_gantt_plugin)
{
    return load_text(QStringLiteral("RELEASE NOTES_en_US.txt"),
                     QStringLiteral("RELEASE"),
                     include_pomodoro_plugin,
                     include_feynman_plugin,
                     include_mapamental_plugin,
                     include_audiolivro_plugin,
                     include_agenda_plugin,
                     include_modelo_plugin,
                     include_gantt_plugin);
}

}  // namespace source::GerenciamentoUI
