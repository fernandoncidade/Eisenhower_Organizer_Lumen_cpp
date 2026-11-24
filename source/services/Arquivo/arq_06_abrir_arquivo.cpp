#include "source/services/Arquivo/arq_06_abrir_arquivo.hpp"

#include <algorithm>

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMap>
#include <QMessageBox>
#include <QPair>
#include <QProcess>
#include <QProcessEnvironment>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QXmlStreamReader>

#include "source/ui/agregador_01_ui.hpp"
#include "source/services/incore_09_Arquivo.hpp"
#include "source/eisenhower_app.hpp"
#include "source/utils/LogManager.hpp"
#include "source/utils/UnifiedTaskStore.hpp"

namespace source::InterfaceCore::ArquivoModules {
namespace {

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

QStringList unified_record_headers()
{
    return source::utils::UnifiedTaskStore::normalize_record(QVariantMap{}).keys();
}

QStringList csv_or_xlsx_headers()
{
    QStringList headers = {QStringLiteral("schema"), QStringLiteral("schema_version")};
    headers << unified_record_headers();
    return headers;
}

QString fold_identifier(QString value)
{
    return value.trimmed().toCaseFolded();
}

bool is_truthy(const QString &value)
{
    const QString folded = fold_identifier(value);
    return folded == QStringLiteral("1") || folded == QStringLiteral("true") || folded == QStringLiteral("yes")
           || folded == QStringLiteral("sim") || folded == QStringLiteral("done")
           || folded == QStringLiteral("completed") || folded == QStringLiteral("concluida")
           || folded == QStringLiteral("concluido");
}

QVariant parse_record_field(const QString &header, const QString &raw_value)
{
    const QString trimmed = raw_value.trimmed();

    if (header == QStringLiteral("concluida") || header == QStringLiteral("descricao_customizada")
        || header == QStringLiteral("sincronizada_com_eisenhower")
        || header == QStringLiteral("integracao_completed")) {
        return is_truthy(trimmed);
    }

    if (header == QStringLiteral("prioridade") || header == QStringLiteral("quadrante")
        || header == QStringLiteral("dominio") || header == QStringLiteral("pomodoros")) {
        bool ok = false;
        const int value = trimmed.toInt(&ok);
        return ok ? QVariant(value) : QVariant(raw_value);
    }

    if (header == QStringLiteral("ordem_insercao")) {
        bool ok = false;
        const qint64 value = trimmed.toLongLong(&ok);
        return ok ? QVariant(value) : QVariant(qint64(0));
    }

    if (header == QStringLiteral("etapas")) {
        if (trimmed.isEmpty()) {
            return QVariantList{};
        }

        QJsonParseError error{};
        const QJsonDocument document = QJsonDocument::fromJson(trimmed.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError && document.isArray()) {
            return document.array().toVariantList();
        }
        return QVariantList{raw_value};
    }

    return raw_value;
}

QVariantMap build_record_from_fields(const QHash<QString, QString> &fields)
{
    QVariantMap record;
    bool has_any_data = false;

    for (const QString &header : unified_record_headers()) {
        const auto it = fields.constFind(header);
        if (it == fields.cend()) {
            continue;
        }

        if (!it.value().isEmpty()) {
            has_any_data = true;
        }
        record.insert(header, parse_record_field(header, it.value()));
    }

    return has_any_data ? record : QVariantMap{};
}

bool is_unified_document_object(const QVariantMap &document)
{
    return document.value(QStringLiteral("schema")).toString() == QStringLiteral("eisenhower_unified_tasks")
        && document.value(QStringLiteral("records")).canConvert<QVariantList>();
}

bool persist_unified_document_to_shared_store(source::EisenhowerMatrixApp *app, const QVariantMap &document)
{
    if (app == nullptr || !is_unified_document_object(document)) {
        return false;
    }

    if (!source::utils::UnifiedTaskStore::save_records(app->tasks_path,
                                                       document.value(QStringLiteral("records")).toList())) {
        return false;
    }

    app->reload_all_modules_from_shared_store();
    return true;
}

bool is_supported_json_document(const QJsonDocument &document)
{
    if (!document.isObject()) {
        return false;
    }

    const QJsonObject object = document.object();
    if (object.value(QStringLiteral("schema")).toString() == QStringLiteral("eisenhower_unified_tasks")
        && object.value(QStringLiteral("records")).isArray()) {
        return true;
    }

    static const QStringList expected_keys = {
        QStringLiteral("quadrant1"),
        QStringLiteral("quadrant1_completed"),
        QStringLiteral("quadrant2"),
        QStringLiteral("quadrant2_completed"),
        QStringLiteral("quadrant3"),
        QStringLiteral("quadrant3_completed"),
        QStringLiteral("quadrant4"),
        QStringLiteral("quadrant4_completed"),
    };

    for (const QString &key : expected_keys) {
        if (object.contains(key) && object.value(key).isArray()) {
            return true;
        }
    }

    return false;
}

bool import_json_path(source::EisenhowerMatrixApp *app, const QString &path)
{
    if (app == nullptr) {
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonParseError error{};
    const QByteArray payload = file.readAll();
    const QJsonDocument document = QJsonDocument::fromJson(payload, &error);
    if (error.error != QJsonParseError::NoError || !is_supported_json_document(document)) {
        return false;
    }

    if (!source::utils::UnifiedTaskStore::save_records(app->tasks_path,
                                                       source::utils::UnifiedTaskStore::load_records(path))) {
        return false;
    }

    app->reload_all_modules_from_shared_store();
    return true;
}

QList<QStringList> parse_csv_rows(const QString &content)
{
    QList<QStringList> rows;
    QStringList current_row;
    QString current_field;
    bool in_quotes = false;

    const QString normalized = QString(content).remove(QChar(0xFEFF));
    for (int i = 0; i < normalized.size(); ++i) {
        const QChar ch = normalized.at(i);
        if (in_quotes) {
            if (ch == QLatin1Char('"')) {
                if (i + 1 < normalized.size() && normalized.at(i + 1) == QLatin1Char('"')) {
                    current_field += QLatin1Char('"');
                    ++i;
                } else {
                    in_quotes = false;
                }
            } else {
                current_field += ch;
            }
            continue;
        }

        if (ch == QLatin1Char('"')) {
            in_quotes = true;
            continue;
        }

        if (ch == QLatin1Char(',')) {
            current_row.append(current_field);
            current_field.clear();
            continue;
        }

        if (ch == QLatin1Char('\n')) {
            current_row.append(current_field);
            rows.append(current_row);
            current_row.clear();
            current_field.clear();
            continue;
        }

        if (ch != QLatin1Char('\r')) {
            current_field += ch;
        }
    }

    current_row.append(current_field);
    if (!current_row.isEmpty()) {
        const bool has_any_value =
            std::any_of(current_row.cbegin(), current_row.cend(), [](const QString &value) { return !value.isEmpty(); });
        if (has_any_value) {
            rows.append(current_row);
        }
    }

    return rows;
}

bool parse_csv_document(const QString &path, QVariantMap *document)
{
    if (document == nullptr) {
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    const QList<QStringList> rows = parse_csv_rows(QString::fromUtf8(file.readAll()));
    if (rows.isEmpty()) {
        *document = {
            {QStringLiteral("schema"), QStringLiteral("eisenhower_unified_tasks")},
            {QStringLiteral("schema_version"), 2},
            {QStringLiteral("records"), QVariantList{}},
        };
        return true;
    }

    const QStringList headers = rows.first();
    QHash<int, QString> header_map;
    for (int i = 0; i < headers.size(); ++i) {
        header_map.insert(i, fold_identifier(headers.at(i)));
    }

    if (!header_map.values().contains(QStringLiteral("schema"))
        || !header_map.values().contains(QStringLiteral("schema_version"))
        || !header_map.values().contains(QStringLiteral("titulo"))) {
        return false;
    }

    QVariantList records;
    QString schema = QStringLiteral("eisenhower_unified_tasks");
    int schema_version = 2;

    for (int row_index = 1; row_index < rows.size(); ++row_index) {
        const QStringList &row = rows.at(row_index);
        QHash<QString, QString> fields;
        bool has_any_value = false;

        for (int column = 0; column < row.size(); ++column) {
            const QString header = header_map.value(column);
            if (header.isEmpty()) {
                continue;
            }
            const QString value = row.at(column);
            if (!value.isEmpty()) {
                has_any_value = true;
            }
            fields.insert(header, value);
        }

        if (!has_any_value) {
            continue;
        }

        const QString row_schema = fields.value(QStringLiteral("schema")).trimmed();
        if (!row_schema.isEmpty()) {
            schema = row_schema;
        }

        bool version_ok = false;
        const int row_schema_version = fields.value(QStringLiteral("schema_version")).trimmed().toInt(&version_ok);
        if (version_ok) {
            schema_version = row_schema_version;
        }

        const QVariantMap record = build_record_from_fields(fields);
        if (!record.isEmpty()) {
            records.append(record);
        }
    }

    *document = {
        {QStringLiteral("schema"), schema},
        {QStringLiteral("schema_version"), schema_version},
        {QStringLiteral("records"), records},
    };
    return true;
}

bool extract_xlsx_archive(const QString &archive_path, const QString &destination_path)
{
    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("EIS_XLSX_ARCHIVE"), QDir::toNativeSeparators(archive_path));
    env.insert(QStringLiteral("EIS_XLSX_DESTINATION"), QDir::toNativeSeparators(destination_path));
    process.setProcessEnvironment(env);

    const QString script =
        QStringLiteral(
            "$src = $env:EIS_XLSX_ARCHIVE; "
            "$dst = $env:EIS_XLSX_DESTINATION; "
            "Add-Type -AssemblyName System.IO.Compression.FileSystem; "
            "if (Test-Path -LiteralPath $dst) { Remove-Item -LiteralPath $dst -Recurse -Force; } "
            "[System.IO.Directory]::CreateDirectory($dst) | Out-Null; "
            "[System.IO.Compression.ZipFile]::ExtractToDirectory($src, $dst);");

    process.start(QStringLiteral("powershell.exe"),
                  {QStringLiteral("-NoProfile"),
                   QStringLiteral("-NonInteractive"),
                   QStringLiteral("-ExecutionPolicy"),
                   QStringLiteral("Bypass"),
                   QStringLiteral("-Command"),
                   script});

    if (!process.waitForFinished(30000)) {
        return false;
    }

    return process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0;
}

QStringList load_xlsx_shared_strings(const QString &path)
{
    QStringList shared_strings;
    QFile file(path);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return shared_strings;
    }

    QXmlStreamReader xml(&file);
    while (!xml.atEnd()) {
        xml.readNext();
        if (!xml.isStartElement() || xml.name() != QStringLiteral("si")) {
            continue;
        }

        QString value;
        while (!(xml.isEndElement() && xml.name() == QStringLiteral("si")) && !xml.atEnd()) {
            xml.readNext();
            if (xml.isStartElement() && xml.name() == QStringLiteral("t")) {
                value += xml.readElementText(QXmlStreamReader::IncludeChildElements);
            }
        }
        shared_strings.append(value);
    }

    return shared_strings;
}

QHash<QString, QString> load_xlsx_relationships(const QString &path)
{
    QHash<QString, QString> relationships;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return relationships;
    }

    QXmlStreamReader xml(&file);
    while (!xml.atEnd()) {
        xml.readNext();
        if (!xml.isStartElement() || xml.name() != QStringLiteral("Relationship")) {
            continue;
        }
        const QString id = xml.attributes().value(QStringLiteral("Id")).toString();
        const QString target = xml.attributes().value(QStringLiteral("Target")).toString();
        if (!id.isEmpty() && !target.isEmpty()) {
            relationships.insert(id, target);
        }
    }

    return relationships;
}

QList<QPair<QString, QString>> load_xlsx_sheets(const QString &workbook_path,
                                                const QHash<QString, QString> &relationships)
{
    QList<QPair<QString, QString>> sheets;
    QFile file(workbook_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return sheets;
    }

    const QDir workbook_dir = QFileInfo(workbook_path).absoluteDir();
    QXmlStreamReader xml(&file);
    while (!xml.atEnd()) {
        xml.readNext();
        if (!xml.isStartElement() || xml.name() != QStringLiteral("sheet")) {
            continue;
        }

        const QString name = xml.attributes().value(QStringLiteral("name")).toString();
        QString relationship_id =
            xml.attributes()
                .value(QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships"),
                       QStringLiteral("id"))
                .toString();
        if (relationship_id.isEmpty()) {
            relationship_id = xml.attributes().value(QStringLiteral("r:id")).toString();
        }

        const QString target = relationships.value(relationship_id);
        if (name.isEmpty() || target.isEmpty()) {
            continue;
        }

        sheets.append({name, QDir::cleanPath(workbook_dir.filePath(target))});
    }

    return sheets;
}

int xlsx_column_from_reference(const QString &reference)
{
    int column = 0;
    for (QChar ch : reference) {
        if (!ch.isLetter()) {
            break;
        }
        column = (column * 26) + (ch.toUpper().unicode() - QLatin1Char('A').unicode() + 1);
    }
    return column;
}

QString read_xlsx_cell(QXmlStreamReader &xml, const QString &type, const QStringList &shared_strings)
{
    QString value;
    while (!(xml.isEndElement() && xml.name() == QStringLiteral("c")) && !xml.atEnd()) {
        xml.readNext();
        if (!xml.isStartElement()) {
            continue;
        }
        if (xml.name() == QStringLiteral("v") || xml.name() == QStringLiteral("t")) {
            value += xml.readElementText(QXmlStreamReader::IncludeChildElements);
        }
    }

    if (type == QStringLiteral("s")) {
        bool ok = false;
        const int index = value.toInt(&ok);
        if (ok && index >= 0 && index < shared_strings.size()) {
            return shared_strings.at(index);
        }
    }

    return value;
}

QList<QHash<QString, QString>> load_xlsx_rows(const QString &sheet_path, const QStringList &shared_strings)
{
    QList<QHash<QString, QString>> rows;
    QFile file(sheet_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return rows;
    }

    QHash<int, QString> header_map;
    QXmlStreamReader xml(&file);

    while (!xml.atEnd()) {
        xml.readNext();
        if (!xml.isStartElement() || xml.name() != QStringLiteral("row")) {
            continue;
        }

        QMap<int, QString> row_cells;
        while (!(xml.isEndElement() && xml.name() == QStringLiteral("row")) && !xml.atEnd()) {
            xml.readNext();
            if (!xml.isStartElement() || xml.name() != QStringLiteral("c")) {
                continue;
            }

            const QString cell_ref = xml.attributes().value(QStringLiteral("r")).toString();
            const int column = xlsx_column_from_reference(cell_ref);
            const QString cell_type = xml.attributes().value(QStringLiteral("t")).toString();
            const QString cell_value = read_xlsx_cell(xml, cell_type, shared_strings);
            if (column > 0) {
                row_cells.insert(column, cell_value);
            }
        }

        if (row_cells.isEmpty()) {
            continue;
        }

        if (header_map.isEmpty()) {
            for (auto it = row_cells.cbegin(); it != row_cells.cend(); ++it) {
                header_map.insert(it.key(), fold_identifier(it.value()));
            }
            continue;
        }

        QHash<QString, QString> fields;
        for (auto it = row_cells.cbegin(); it != row_cells.cend(); ++it) {
            const QString header = header_map.value(it.key());
            if (!header.isEmpty()) {
                fields.insert(header, it.value());
            }
        }
        rows.append(fields);
    }

    return rows;
}

bool parse_xlsx_document(const QString &path, QVariantMap *document)
{
    if (document == nullptr) {
        return false;
    }

    QTemporaryDir temp_dir;
    if (!temp_dir.isValid()) {
        return false;
    }

    const QString extraction_dir = QDir(temp_dir.path()).filePath(QStringLiteral("xlsx"));
    if (!extract_xlsx_archive(path, extraction_dir)) {
        return false;
    }

    const QString workbook_path = QDir(extraction_dir).filePath(QStringLiteral("xl/workbook.xml"));
    const QString rels_path = QDir(extraction_dir).filePath(QStringLiteral("xl/_rels/workbook.xml.rels"));
    if (!QFileInfo::exists(workbook_path) || !QFileInfo::exists(rels_path)) {
        return false;
    }

    const QStringList shared_strings =
        load_xlsx_shared_strings(QDir(extraction_dir).filePath(QStringLiteral("xl/sharedStrings.xml")));
    const QHash<QString, QString> relationships = load_xlsx_relationships(rels_path);
    const QList<QPair<QString, QString>> sheets = load_xlsx_sheets(workbook_path, relationships);

    QString records_sheet_path;
    for (const auto &sheet : sheets) {
        if (fold_identifier(sheet.first) == QStringLiteral("records")) {
            records_sheet_path = sheet.second;
            break;
        }
    }
    if (records_sheet_path.isEmpty() && !sheets.isEmpty()) {
        records_sheet_path = sheets.first().second;
    }
    if (records_sheet_path.isEmpty()) {
        return false;
    }

    const QList<QHash<QString, QString>> rows = load_xlsx_rows(records_sheet_path, shared_strings);

    QVariantList records;
    QString schema = QStringLiteral("eisenhower_unified_tasks");
    int schema_version = 2;

    for (const QHash<QString, QString> &fields : rows) {
        const QString row_schema = fields.value(QStringLiteral("schema")).trimmed();
        if (!row_schema.isEmpty()) {
            schema = row_schema;
        }

        bool version_ok = false;
        const int row_schema_version = fields.value(QStringLiteral("schema_version")).trimmed().toInt(&version_ok);
        if (version_ok) {
            schema_version = row_schema_version;
        }

        const QVariantMap record = build_record_from_fields(fields);
        if (!record.isEmpty()) {
            records.append(record);
        }
    }

    *document = {
        {QStringLiteral("schema"), schema},
        {QStringLiteral("schema_version"), schema_version},
        {QStringLiteral("records"), records},
    };
    return true;
}

bool parse_pdf_document(const QString &path, QVariantMap *document)
{
    if (document == nullptr) {
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    static const QByteArray marker_begin("\n%EISENHOWER_ORGANIZER_PAYLOAD_BEGIN\n");
    static const QByteArray marker_end("\n%EISENHOWER_ORGANIZER_PAYLOAD_END\n");

    const QByteArray raw = file.readAll();
    const int marker_start = raw.indexOf(marker_begin);
    if (marker_start < 0) {
        return false;
    }

    const int payload_start = marker_start + marker_begin.size();
    const int marker_finish = raw.indexOf(marker_end, payload_start);
    if (marker_finish < 0) {
        return false;
    }

    const QByteArray payload = QByteArray::fromBase64(raw.mid(payload_start, marker_finish - payload_start).trimmed());
    QJsonParseError error{};
    const QJsonDocument json_document = QJsonDocument::fromJson(payload, &error);
    if (error.error != QJsonParseError::NoError || !json_document.isObject()) {
        return false;
    }

    *document = json_document.object().toVariantMap();
    return true;
}

bool import_from_non_json_path(source::EisenhowerMatrixApp *app, const QString &path)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    QVariantMap document;

    if (suffix == QStringLiteral("csv")) {
        if (!parse_csv_document(path, &document)) {
            logger()->error(QStringLiteral("Falha ao importar CSV unificado: %1").arg(path));
            return false;
        }
    } else if (suffix == QStringLiteral("xlsx")) {
        if (!parse_xlsx_document(path, &document)) {
            logger()->error(QStringLiteral("Falha ao importar XLSX unificado: %1").arg(path));
            return false;
        }
    } else if (suffix == QStringLiteral("pdf")) {
        if (!parse_pdf_document(path, &document)) {
            logger()->error(QStringLiteral("Falha ao importar PDF unificado: %1").arg(path));
            return false;
        }
    } else {
        return false;
    }

    return persist_unified_document_to_shared_store(app, document);
}

}  // namespace

void abrir_arquivo(Arquivo *self)
{
    if (self == nullptr || self->app() == nullptr) {
        return;
    }

    source::EisenhowerMatrixApp *app = self->app();
    const QString path =
        QFileDialog::getOpenFileName(app,
                                     get_text(QStringLiteral("Abrir")),
                                     QFileInfo(app->tasks_path).absolutePath(),
                                     QStringLiteral("JSON (*.json);;Excel (*.xlsx);;CSV (*.csv);;PDF (*.pdf)"));
    if (path.isEmpty()) {
        return;
    }

    const QString suffix = QFileInfo(path).suffix().toLower();
    const bool imported = suffix == QStringLiteral("json") ? import_json_path(app, path)
                                                           : import_from_non_json_path(app, path);
    if (imported) {
        if (app->calendar_pane != nullptr && app->calendar_pane->calendar_panel != nullptr) {
            app->calendar_pane->calendar_panel->update_task_list();
        }
        QMessageBox::information(app,
                                 get_text(QStringLiteral("Abrir")),
                                 get_text(QStringLiteral("Arquivo importado com sucesso.")));
        return;
    }

    QMessageBox::warning(app,
                         get_text(QStringLiteral("Abrir")),
                         get_text(QStringLiteral("Formato de arquivo não suportado.")));
}

}  // namespace source::InterfaceCore::ArquivoModules
