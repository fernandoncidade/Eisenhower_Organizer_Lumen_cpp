#include "source/services/Arquivo/arq_07_salvar_como.hpp"

#include <QAbstractTextDocumentLayout>
#include <QCoreApplication>
#include <QDataStream>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonDocument>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPdfWriter>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStringList>
#include <QTextDocument>
#include <QVariantList>
#include <QVariantMap>
#include <QXmlStreamWriter>
#include <array>
#include <functional>
#include <utility>

#include "source/services/incore_09_Arquivo.hpp"
#include "source/eisenhower_app.hpp"
#include "source/utils/LogManager.hpp"
#include "source/utils/UnifiedTaskStore.hpp"

namespace source::InterfaceCore::ArquivoModules {
namespace {

struct TableSheet {
    QString name;
    QList<QStringList> rows;
};

struct ZipEntry {
    QString name;
    QByteArray data;
    quint32 crc = 0;
    quint32 offset = 0;
};

QString get_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

QString documents_directory()
{
    const QString documents_dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (!documents_dir.isEmpty()) {
        return documents_dir;
    }

    const QString home_dir = QDir::homePath();
    const QString fallback_documents = QDir(home_dir).filePath(QStringLiteral("Documents"));
    return QDir(fallback_documents).exists() ? fallback_documents : home_dir;
}

bool is_supported_export_suffix(const QString &suffix)
{
    return suffix == QStringLiteral("json") || suffix == QStringLiteral("xlsx") || suffix == QStringLiteral("pdf")
           || suffix == QStringLiteral("csv");
}

QString extension_from_filter(const QString &selected_filter)
{
    if (selected_filter.contains(QStringLiteral("*.xlsx"))) {
        return QStringLiteral("xlsx");
    }
    if (selected_filter.contains(QStringLiteral("*.csv"))) {
        return QStringLiteral("csv");
    }
    if (selected_filter.contains(QStringLiteral("*.pdf"))) {
        return QStringLiteral("pdf");
    }
    return QStringLiteral("json");
}

QString normalize_save_path(const QString &path, const QString &selected_filter)
{
    QString normalized = path.trimmed();
    if (normalized.isEmpty()) {
        return normalized;
    }

    if (QFileInfo(normalized).suffix().isEmpty()) {
        normalized += QStringLiteral(".") + extension_from_filter(selected_filter);
    }

    return QDir::toNativeSeparators(normalized);
}

QString sanitize_text(const QString &text)
{
    QString sanitized;
    sanitized.reserve(text.size());
    for (QChar ch : text) {
        const ushort code = ch.unicode();
        if (code == 0x09 || code == 0x0A || code == 0x0D || code >= 0x20) {
            sanitized.append(ch);
        }
    }
    return sanitized;
}

QString soft_break_html(const QString &text)
{
    QString escaped = sanitize_text(text).toHtmlEscaped();
    escaped.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    escaped.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    escaped.replace(QLatin1Char('\n'), QStringLiteral("<br/>"));
    escaped.replace(QStringLiteral("\\"), QStringLiteral("\\&#8203;"));
    escaped.replace(QStringLiteral("/"), QStringLiteral("/&#8203;"));
    escaped.replace(QStringLiteral("_"), QStringLiteral("_&#8203;"));
    escaped.replace(QStringLiteral("-"), QStringLiteral("-&#8203;"));
    escaped.replace(QStringLiteral(";"), QStringLiteral(";&#8203;"));
    return escaped;
}

bool ensure_parent_directory(const QString &path, QString *error_message)
{
    const QFileInfo file_info(path);
    QDir parent_dir = file_info.absoluteDir();
    if (parent_dir.exists() || parent_dir.mkpath(QStringLiteral("."))) {
        return true;
    }

    if (error_message != nullptr) {
        *error_message = get_text(QStringLiteral("Nao foi possivel salvar o arquivo."));
    }
    return false;
}

QString default_export_file_name(const source::EisenhowerMatrixApp *app)
{
    QString base_name = QFileInfo(app != nullptr ? app->export_path : QString()).completeBaseName();
    if (base_name.isEmpty()) {
        base_name = QFileInfo(app != nullptr ? app->tasks_path : QString()).completeBaseName();
    }
    if (base_name.isEmpty()) {
        base_name = QStringLiteral("tarefas");
    }
    return base_name;
}

QString default_export_path(const source::EisenhowerMatrixApp *app, const QString &suffix)
{
    return QDir(documents_directory()).filePath(QStringLiteral("%1.%2").arg(default_export_file_name(app), suffix));
}

QString save_target_in_documents(const source::EisenhowerMatrixApp *app)
{
    const QFileInfo current_export(app != nullptr ? app->export_path : QString());
    const QString suffix = current_export.suffix().toLower();
    const QString file_name =
        (!current_export.fileName().isEmpty() && is_supported_export_suffix(suffix))
            ? current_export.fileName()
            : QFileInfo(default_export_path(app, QStringLiteral("json"))).fileName();
    return QDir(documents_directory()).filePath(file_name);
}

QString save_target_base_path(const source::EisenhowerMatrixApp *app)
{
    const QFileInfo current_export(app != nullptr ? app->export_path : QString());
    const QString suffix = current_export.suffix().toLower();
    if (!current_export.filePath().isEmpty() && is_supported_export_suffix(suffix)) {
        return current_export.absoluteDir().filePath(current_export.completeBaseName());
    }

    const QFileInfo default_json(default_export_path(app, QStringLiteral("json")));
    return default_json.absoluteDir().filePath(default_json.completeBaseName());
}

QStringList unified_record_headers()
{
    return source::utils::UnifiedTaskStore::normalize_record(QVariantMap{}).keys();
}

QVariantList export_records(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return {};
    }

    app->save_tasks_now();
    return source::utils::UnifiedTaskStore::load_records(app->tasks_path);
}

QVariantMap build_unified_document(const QVariantList &records)
{
    return {
        {QStringLiteral("schema"), QStringLiteral("eisenhower_unified_tasks")},
        {QStringLiteral("schema_version"), 2},
        {QStringLiteral("records"), records},
    };
}

QString field_to_string(const QVariant &value)
{
    if (!value.isValid() || value.isNull()) {
        return {};
    }

    if (value.typeId() == QMetaType::Bool) {
        return value.toBool() ? QStringLiteral("true") : QStringLiteral("false");
    }

    if (value.canConvert<QVariantList>() && value.typeId() != QMetaType::QStringList
        && value.typeId() != QMetaType::QString) {
        return QString::fromUtf8(QJsonDocument::fromVariant(value).toJson(QJsonDocument::Compact));
    }

    if (value.typeId() == QMetaType::QStringList) {
        return QString::fromUtf8(QJsonDocument::fromVariant(value.toStringList()).toJson(QJsonDocument::Compact));
    }

    if (value.typeId() == QMetaType::QVariantMap || value.typeId() == QMetaType::QVariantList) {
        return QString::fromUtf8(QJsonDocument::fromVariant(value).toJson(QJsonDocument::Compact));
    }

    return value.toString();
}

QList<QStringList> build_record_table_rows(const QVariantList &records)
{
    QList<QStringList> rows;
    const QStringList headers = unified_record_headers();

    QStringList header_row;
    header_row << QStringLiteral("schema") << QStringLiteral("schema_version");
    header_row << headers;
    rows.append(header_row);

    for (const QVariant &value : records) {
        const QVariantMap record = source::utils::UnifiedTaskStore::normalize_record(value.toMap());

        QStringList row;
        row << QStringLiteral("eisenhower_unified_tasks") << QStringLiteral("2");
        for (const QString &header : headers) {
            row << field_to_string(record.value(header));
        }
        rows.append(row);
    }

    return rows;
}

bool write_json_document(const QString &path, const QVariantList &records, QString *error_message)
{
    if (!ensure_parent_directory(path, error_message)) {
        return false;
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error_message != nullptr) {
            *error_message = file.errorString();
        }
        return false;
    }

    const QByteArray payload =
        QJsonDocument::fromVariant(build_unified_document(records)).toJson(QJsonDocument::Indented);
    if (file.write(payload) != payload.size()) {
        if (error_message != nullptr) {
            *error_message = file.errorString();
        }
        return false;
    }

    if (!file.commit()) {
        if (error_message != nullptr) {
            *error_message = file.errorString();
        }
        return false;
    }

    return true;
}

QString csv_escape(const QString &value)
{
    QString escaped = sanitize_text(value);
    escaped.replace(QStringLiteral("\""), QStringLiteral("\"\""));
    return QStringLiteral("\"%1\"").arg(escaped);
}

bool write_csv_file(const QString &path, const QVariantList &records, QString *error_message)
{
    if (!ensure_parent_directory(path, error_message)) {
        return false;
    }

    QStringList lines;
    const QList<QStringList> rows = build_record_table_rows(records);
    for (const QStringList &row : rows) {
        QStringList escaped_row;
        for (const QString &column : row) {
            escaped_row << csv_escape(column);
        }
        lines << escaped_row.join(QLatin1Char(','));
    }

    QByteArray payload("\xEF\xBB\xBF", 3);
    payload += lines.join(QStringLiteral("\r\n")).toUtf8();
    payload += QByteArray("\r\n");

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error_message != nullptr) {
            *error_message = file.errorString();
        }
        return false;
    }

    if (file.write(payload) != payload.size()) {
        if (error_message != nullptr) {
            *error_message = file.errorString();
        }
        return false;
    }

    if (!file.commit()) {
        if (error_message != nullptr) {
            *error_message = file.errorString();
        }
        return false;
    }

    return true;
}

QString column_name(int column_index)
{
    QString result;
    int index = column_index;
    while (index > 0) {
        const int remainder = (index - 1) % 26;
        result.prepend(QChar(static_cast<char16_t>('A' + remainder)));
        index = (index - 1) / 26;
    }
    return result;
}

void write_inline_cell(QXmlStreamWriter &xml, int column_index, int row_index, const QString &value)
{
    const QString sanitized = sanitize_text(value);
    if (sanitized.isEmpty()) {
        return;
    }

    xml.writeStartElement(QStringLiteral("c"));
    xml.writeAttribute(QStringLiteral("r"), QStringLiteral("%1%2").arg(column_name(column_index)).arg(row_index));
    xml.writeAttribute(QStringLiteral("t"), QStringLiteral("inlineStr"));
    xml.writeStartElement(QStringLiteral("is"));
    xml.writeStartElement(QStringLiteral("t"));
    if (sanitized != sanitized.trimmed() || sanitized.contains(QLatin1Char('\n'))
        || sanitized.contains(QLatin1Char('\t'))) {
        xml.writeAttribute(QStringLiteral("http://www.w3.org/XML/1998/namespace"),
                           QStringLiteral("space"),
                           QStringLiteral("preserve"));
    }
    xml.writeCharacters(sanitized);
    xml.writeEndElement();
    xml.writeEndElement();
    xml.writeEndElement();
}

QByteArray build_xml(const std::function<void(QXmlStreamWriter &)> &writer_fn)
{
    QByteArray data;
    QXmlStreamWriter xml(&data);
    xml.setAutoFormatting(false);
    xml.writeStartDocument();
    writer_fn(xml);
    xml.writeEndDocument();
    return data;
}

QByteArray build_content_types_xml(const QList<TableSheet> &sheets)
{
    return build_xml([&sheets](QXmlStreamWriter &xml) {
        xml.writeStartElement(QStringLiteral("Types"));
        xml.writeDefaultNamespace(QStringLiteral("http://schemas.openxmlformats.org/package/2006/content-types"));

        xml.writeEmptyElement(QStringLiteral("Default"));
        xml.writeAttribute(QStringLiteral("Extension"), QStringLiteral("rels"));
        xml.writeAttribute(QStringLiteral("ContentType"),
                           QStringLiteral("application/vnd.openxmlformats-package.relationships+xml"));

        xml.writeEmptyElement(QStringLiteral("Default"));
        xml.writeAttribute(QStringLiteral("Extension"), QStringLiteral("xml"));
        xml.writeAttribute(QStringLiteral("ContentType"), QStringLiteral("application/xml"));

        xml.writeEmptyElement(QStringLiteral("Override"));
        xml.writeAttribute(QStringLiteral("PartName"), QStringLiteral("/xl/workbook.xml"));
        xml.writeAttribute(QStringLiteral("ContentType"),
                           QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"));

        for (int i = 0; i < sheets.size(); ++i) {
            xml.writeEmptyElement(QStringLiteral("Override"));
            xml.writeAttribute(QStringLiteral("PartName"),
                               QStringLiteral("/xl/worksheets/sheet%1.xml").arg(i + 1));
            xml.writeAttribute(QStringLiteral("ContentType"),
                               QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"));
        }

        xml.writeEndElement();
    });
}

QByteArray build_root_relationships_xml()
{
    return build_xml([](QXmlStreamWriter &xml) {
        xml.writeStartElement(QStringLiteral("Relationships"));
        xml.writeDefaultNamespace(QStringLiteral("http://schemas.openxmlformats.org/package/2006/relationships"));

        xml.writeEmptyElement(QStringLiteral("Relationship"));
        xml.writeAttribute(QStringLiteral("Id"), QStringLiteral("rId1"));
        xml.writeAttribute(QStringLiteral("Type"),
                           QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument"));
        xml.writeAttribute(QStringLiteral("Target"), QStringLiteral("xl/workbook.xml"));

        xml.writeEndElement();
    });
}

QByteArray build_workbook_xml(const QList<TableSheet> &sheets)
{
    return build_xml([&sheets](QXmlStreamWriter &xml) {
        xml.writeStartElement(QStringLiteral("workbook"));
        xml.writeDefaultNamespace(QStringLiteral("http://schemas.openxmlformats.org/spreadsheetml/2006/main"));
        xml.writeNamespace(QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships"),
                           QStringLiteral("r"));

        xml.writeStartElement(QStringLiteral("sheets"));
        for (int i = 0; i < sheets.size(); ++i) {
            xml.writeEmptyElement(QStringLiteral("sheet"));
            xml.writeAttribute(QStringLiteral("name"), sheets.at(i).name);
            xml.writeAttribute(QStringLiteral("sheetId"), QString::number(i + 1));
            xml.writeAttribute(QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships"),
                               QStringLiteral("id"),
                               QStringLiteral("rId%1").arg(i + 1));
        }
        xml.writeEndElement();

        xml.writeEndElement();
    });
}

QByteArray build_workbook_relationships_xml(const QList<TableSheet> &sheets)
{
    return build_xml([&sheets](QXmlStreamWriter &xml) {
        xml.writeStartElement(QStringLiteral("Relationships"));
        xml.writeDefaultNamespace(QStringLiteral("http://schemas.openxmlformats.org/package/2006/relationships"));

        for (int i = 0; i < sheets.size(); ++i) {
            xml.writeEmptyElement(QStringLiteral("Relationship"));
            xml.writeAttribute(QStringLiteral("Id"), QStringLiteral("rId%1").arg(i + 1));
            xml.writeAttribute(QStringLiteral("Type"),
                               QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet"));
            xml.writeAttribute(QStringLiteral("Target"), QStringLiteral("worksheets/sheet%1.xml").arg(i + 1));
        }

        xml.writeEndElement();
    });
}

QByteArray build_sheet_xml(const TableSheet &sheet)
{
    return build_xml([&sheet](QXmlStreamWriter &xml) {
        xml.writeStartElement(QStringLiteral("worksheet"));
        xml.writeDefaultNamespace(QStringLiteral("http://schemas.openxmlformats.org/spreadsheetml/2006/main"));
        xml.writeStartElement(QStringLiteral("sheetData"));

        for (int row = 0; row < sheet.rows.size(); ++row) {
            const QStringList &columns = sheet.rows.at(row);
            const int row_index = row + 1;
            xml.writeStartElement(QStringLiteral("row"));
            xml.writeAttribute(QStringLiteral("r"), QString::number(row_index));
            for (int column = 0; column < columns.size(); ++column) {
                write_inline_cell(xml, column + 1, row_index, columns.at(column));
            }
            xml.writeEndElement();
        }

        xml.writeEndElement();
        xml.writeEndElement();
    });
}

quint32 crc32(const QByteArray &data)
{
    static const std::array<quint32, 256> table = [] {
        std::array<quint32, 256> generated{};
        for (quint32 i = 0; i < generated.size(); ++i) {
            quint32 value = i;
            for (int bit = 0; bit < 8; ++bit) {
                value = (value & 1U) != 0U ? 0xEDB88320U ^ (value >> 1U) : (value >> 1U);
            }
            generated.at(i) = value;
        }
        return generated;
    }();

    quint32 value = 0xFFFFFFFFU;
    for (unsigned char byte : data) {
        value = table.at((value ^ byte) & 0xFFU) ^ (value >> 8U);
    }
    return value ^ 0xFFFFFFFFU;
}

bool write_xlsx_archive(const QString &path, const QVariantList &records, QString *error_message)
{
    if (!ensure_parent_directory(path, error_message)) {
        return false;
    }

    const QList<TableSheet> sheets = {{QStringLiteral("records"), build_record_table_rows(records)}};

    QList<ZipEntry> entries;
    entries.append({QStringLiteral("[Content_Types].xml"), build_content_types_xml(sheets)});
    entries.append({QStringLiteral("_rels/.rels"), build_root_relationships_xml()});
    entries.append({QStringLiteral("xl/workbook.xml"), build_workbook_xml(sheets)});
    entries.append({QStringLiteral("xl/_rels/workbook.xml.rels"), build_workbook_relationships_xml(sheets)});
    for (int i = 0; i < sheets.size(); ++i) {
        entries.append({QStringLiteral("xl/worksheets/sheet%1.xml").arg(i + 1), build_sheet_xml(sheets.at(i))});
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error_message != nullptr) {
            *error_message = file.errorString();
        }
        return false;
    }

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    for (ZipEntry &entry : entries) {
        entry.crc = crc32(entry.data);
        entry.offset = static_cast<quint32>(file.pos());
        const QByteArray name_bytes = entry.name.toUtf8();
        const quint32 size = static_cast<quint32>(entry.data.size());

        stream << quint32(0x04034B50U) << quint16(20U) << quint16(0U) << quint16(0U) << quint16(0U)
               << quint16(0U) << quint32(entry.crc) << quint32(size) << quint32(size)
               << quint16(name_bytes.size()) << quint16(0U);
        file.write(name_bytes);
        file.write(entry.data);
    }

    const quint32 central_directory_offset = static_cast<quint32>(file.pos());
    for (const ZipEntry &entry : std::as_const(entries)) {
        const QByteArray name_bytes = entry.name.toUtf8();
        const quint32 size = static_cast<quint32>(entry.data.size());

        stream << quint32(0x02014B50U) << quint16(20U) << quint16(20U) << quint16(0U) << quint16(0U)
               << quint16(0U) << quint16(0U) << quint32(entry.crc) << quint32(size) << quint32(size)
               << quint16(name_bytes.size()) << quint16(0U) << quint16(0U) << quint16(0U) << quint16(0U)
               << quint32(0U) << quint32(entry.offset);
        file.write(name_bytes);
    }

    const quint32 central_directory_size = static_cast<quint32>(file.pos()) - central_directory_offset;
    stream << quint32(0x06054B50U) << quint16(0U) << quint16(0U) << quint16(entries.size())
           << quint16(entries.size()) << quint32(central_directory_size) << quint32(central_directory_offset)
           << quint16(0U);

    if (stream.status() != QDataStream::Ok) {
        if (error_message != nullptr) {
            *error_message = get_text(QStringLiteral("Nao foi possivel salvar o arquivo."));
        }
        return false;
    }

    if (!file.commit()) {
        if (error_message != nullptr) {
            *error_message = file.errorString();
        }
        return false;
    }

    return true;
}

void append_pdf_field(QString &html, const QString &label, const QString &value)
{
    html += QStringLiteral("<p class=\"meta\"><span class=\"label\">%1:</span> %2</p>")
                .arg(label.toHtmlEscaped(), soft_break_html(value));
}

QByteArray build_pdf_embedded_payload(const QVariantList &records)
{
    static const QByteArray marker_begin("\n%EISENHOWER_ORGANIZER_PAYLOAD_BEGIN\n");
    static const QByteArray marker_end("\n%EISENHOWER_ORGANIZER_PAYLOAD_END\n");
    const QByteArray encoded_payload =
        QJsonDocument::fromVariant(build_unified_document(records)).toJson(QJsonDocument::Compact).toBase64();
    return marker_begin + encoded_payload + marker_end;
}

QString build_pdf_html(const QVariantList &records)
{
    QString html;
    html += QStringLiteral(
        "<html><head><meta charset=\"utf-8\"/>"
        "<style>"
        "body{font-family:'Segoe UI';font-size:9pt;line-height:1.35;color:#1f1f1f;}"
        "h1{font-size:18pt;margin:0 0 16px 0;}"
        "h2{font-size:12pt;margin:18px 0 8px 0;padding-bottom:4px;border-bottom:1px solid #bdbdbd;}"
        ".task{margin:0 0 12px 0;padding:8px 10px;border:1px solid #d7d7d7;border-radius:4px;page-break-inside:avoid;}"
        ".title{margin:0 0 5px 0;font-weight:600;}"
        ".meta{margin:2px 0 0 14px;}"
        ".label{font-weight:600;}"
        ".empty{margin:0 0 10px 14px;color:#666666;font-style:italic;}"
        "</style></head><body>");

    html += QStringLiteral("<h1>Eisenhower Organizer</h1>");
    append_pdf_field(html, QStringLiteral("schema"), QStringLiteral("eisenhower_unified_tasks"));
    append_pdf_field(html, QStringLiteral("schema_version"), QStringLiteral("2"));

    const QStringList headers = unified_record_headers();
    if (records.isEmpty()) {
        html += QStringLiteral("<p class=\"empty\">records: []</p>");
    }

    for (const QVariant &value : records) {
        const QVariantMap record = source::utils::UnifiedTaskStore::normalize_record(value.toMap());
        const QString title = record.value(QStringLiteral("titulo")).toString().trimmed().isEmpty()
                                  ? record.value(QStringLiteral("id")).toString()
                                  : record.value(QStringLiteral("titulo")).toString();

        html += QStringLiteral("<div class=\"task\">");
        html += QStringLiteral("<p class=\"title\">%1</p>").arg(soft_break_html(title));
        for (const QString &header : headers) {
            append_pdf_field(html, header, field_to_string(record.value(header)));
        }
        html += QStringLiteral("</div>");
    }

    html += QStringLiteral("</body></html>");
    return html;
}

bool write_pdf_document(const QString &path, const QVariantList &records, QString *error_message)
{
    if (!ensure_parent_directory(path, error_message)) {
        return false;
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error_message != nullptr) {
            *error_message = file.errorString();
        }
        return false;
    }

    {
        QPdfWriter writer(&file);
        writer.setResolution(144);
        writer.setPageLayout(QPageLayout(QPageSize(QPageSize::A4),
                                         QPageLayout::Portrait,
                                         QMarginsF(18.0, 18.0, 18.0, 18.0),
                                         QPageLayout::Millimeter));
        writer.setTitle(QStringLiteral("Eisenhower Organizer"));

        QTextDocument document;
        document.setDocumentMargin(0.0);
        document.setHtml(build_pdf_html(records));

        const QRectF page_rect = writer.pageLayout().paintRectPixels(writer.resolution());
        document.setPageSize(page_rect.size());

        QPainter painter(&writer);
        if (!painter.isActive()) {
            if (error_message != nullptr) {
                *error_message = get_text(QStringLiteral("Nao foi possivel salvar o arquivo."));
            }
            return false;
        }

        auto *layout = document.documentLayout();
        if (layout == nullptr) {
            if (error_message != nullptr) {
                *error_message = get_text(QStringLiteral("Nao foi possivel salvar o arquivo."));
            }
            return false;
        }

        const qreal page_height = page_rect.height();
        const qreal document_height = document.size().height();
        QAbstractTextDocumentLayout::PaintContext context;

        for (qreal offset = 0.0; offset < document_height || offset == 0.0; offset += page_height) {
            context.clip = QRectF(0.0, offset, page_rect.width(), page_height);

            painter.save();
            painter.translate(page_rect.left(), page_rect.top() - offset);
            layout->draw(&painter, context);
            painter.restore();

            if (offset + page_height >= document_height) {
                break;
            }
            writer.newPage();
        }

        painter.end();
    }

    const QByteArray embedded_payload = build_pdf_embedded_payload(records);
    if (file.write(embedded_payload) != embedded_payload.size()) {
        if (error_message != nullptr) {
            *error_message = file.errorString();
        }
        return false;
    }

    if (!file.commit()) {
        if (error_message != nullptr) {
            *error_message = file.errorString();
        }
        return false;
    }

    return true;
}

bool export_to_path(source::EisenhowerMatrixApp *app, const QString &path, QString *error_message)
{
    const QString suffix = QFileInfo(path).suffix().toLower();
    const QVariantList records = export_records(app);

    if (suffix == QStringLiteral("json")) {
        return write_json_document(path, records, error_message);
    }
    if (suffix == QStringLiteral("xlsx")) {
        return write_xlsx_archive(path, records, error_message);
    }
    if (suffix == QStringLiteral("csv")) {
        return write_csv_file(path, records, error_message);
    }
    if (suffix == QStringLiteral("pdf")) {
        return write_pdf_document(path, records, error_message);
    }

    if (error_message != nullptr) {
        *error_message = get_text(QStringLiteral("Extensão não suportada."));
    }
    return false;
}

bool save_and_report(source::EisenhowerMatrixApp *app, const QString &path)
{
    if (app == nullptr) {
        return false;
    }

    const QString suffix = QFileInfo(path).suffix().toLower();
    if (!is_supported_export_suffix(suffix)) {
        QMessageBox::warning(app,
                             get_text(QStringLiteral("Erro")),
                             get_text(QStringLiteral("Extensão não suportada.")));
        return false;
    }

    QString error_message;
    if (export_to_path(app, path, &error_message)) {
        app->export_path = path;
        QMessageBox::information(app,
                                 get_text(QStringLiteral("Salvar")),
                                 QStringLiteral("%1\n%2")
                                     .arg(get_text(QStringLiteral("Arquivo salvo com sucesso.")),
                                          QDir::toNativeSeparators(path)));
        return true;
    }

    const QString detailed_error =
        error_message.trimmed().isEmpty() ? get_text(QStringLiteral("Erro desconhecido.")) : error_message;
    logger()->error(QStringLiteral("Falha ao exportar arquivo '%1': %2").arg(path, error_message));
    QMessageBox::warning(app,
                         get_text(QStringLiteral("Erro")),
                         QStringLiteral("%1\n%2\n\n%3: %4")
                             .arg(get_text(QStringLiteral("Nao foi possivel salvar o arquivo.")),
                                  QDir::toNativeSeparators(path),
                                  get_text(QStringLiteral("Erro")),
                                  detailed_error));
    return false;
}

bool save_all_default_exports(source::EisenhowerMatrixApp *app)
{
    if (app == nullptr) {
        return false;
    }

    const QString base_path = save_target_base_path(app);
    const QStringList suffixes = {
        QStringLiteral("json"),
        QStringLiteral("csv"),
        QStringLiteral("xlsx"),
        QStringLiteral("pdf"),
    };

    QStringList saved_paths;
    QStringList failures;
    for (const QString &suffix : suffixes) {
        const QString path = QStringLiteral("%1.%2").arg(base_path, suffix);
        QString error_message;
        if (export_to_path(app, path, &error_message)) {
            saved_paths << QDir::toNativeSeparators(path);
            continue;
        }

        const QString detailed_error =
            error_message.trimmed().isEmpty() ? get_text(QStringLiteral("Erro desconhecido.")) : error_message;
        logger()->error(QStringLiteral("Falha ao exportar arquivo '%1': %2").arg(path, error_message));
        failures << QStringLiteral("%1: %2").arg(QDir::toNativeSeparators(path), detailed_error);
    }

    if (saved_paths.isEmpty()) {
        QMessageBox::warning(app,
                             get_text(QStringLiteral("Erro")),
                             QStringLiteral("%1\n\n%2")
                                 .arg(get_text(QStringLiteral("Nao foi possivel salvar o arquivo.")),
                                      failures.join(QLatin1Char('\n'))));
        return false;
    }

    app->export_path = QStringLiteral("%1.%2").arg(base_path, suffixes.constLast());

    QString message = get_text(QStringLiteral("Arquivo salvo com sucesso."));
    message += QStringLiteral("\n");
    message += saved_paths.join(QLatin1Char('\n'));

    if (!failures.isEmpty()) {
        message += QStringLiteral("\n\n%1\n%2")
                       .arg(get_text(QStringLiteral("Erro")), failures.join(QLatin1Char('\n')));
    }

    if (failures.isEmpty()) {
        QMessageBox::information(app, get_text(QStringLiteral("Salvar")), message);
        return true;
    }

    QMessageBox::warning(app, get_text(QStringLiteral("Salvar")), message);
    return false;
}

}  // namespace

void salvar(Arquivo *self)
{
    if (self == nullptr || self->app() == nullptr) {
        return;
    }

    source::EisenhowerMatrixApp *app = self->app();
    save_all_default_exports(app);
}

void salvar_como(Arquivo *self)
{
    if (self == nullptr || self->app() == nullptr) {
        return;
    }

    source::EisenhowerMatrixApp *app = self->app();
    QString selected_filter = QStringLiteral("JSON (*.json)");
    const QString initial_path = QFileInfo(app->export_path).suffix().toLower().isEmpty()
                                     ? default_export_path(app, QStringLiteral("json"))
                                     : app->export_path;
    const QString raw_path = QFileDialog::getSaveFileName(
        app,
        get_text(QStringLiteral("Salvar como")),
        initial_path,
        QStringLiteral("JSON (*.json);;Excel (*.xlsx);;CSV (*.csv);;PDF (*.pdf)"),
        &selected_filter);
    if (raw_path.isEmpty()) {
        return;
    }

    save_and_report(app, normalize_save_path(raw_path, selected_filter));
}

}  // namespace source::InterfaceCore::ArquivoModules
