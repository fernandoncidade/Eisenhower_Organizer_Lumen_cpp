#include "CsvExchangeService.hpp"
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

namespace source::gantt {

CsvColumnMapping CsvColumnMapping::default_mapping()
{
    return CsvColumnMapping();
}

CsvExchangeService::CsvExchangeService()
{
}

static QString escape_csv_cell(const QString &cell, const QString &delimiter)
{
    QString escaped = cell;
    escaped.replace(QStringLiteral("\""), QStringLiteral("\"\""));
    if (escaped.contains(delimiter) || escaped.contains(QStringLiteral("\"")) || escaped.contains(QStringLiteral("\n"))) {
        return QStringLiteral("\"%1\"").arg(escaped);
    }
    return escaped;
}

static QStringList parse_csv_line(const QString &line, const QString &delimiter)
{
    QStringList fields;
    QString current;
    bool in_quotes = false;

    for (int i = 0; i < line.size(); ++i) {
        QChar c = line[i];
        if (c == QLatin1Char('"')) {
            if (in_quotes && i + 1 < line.size() && line[i + 1] == QLatin1Char('"')) {
                current += QLatin1Char('"');
                i++; // skip escaped quote
            } else {
                in_quotes = !in_quotes;
            }
        } else if (!in_quotes && line.mid(i, delimiter.size()) == delimiter) {
            fields.append(current.trimmed());
            current.clear();
            i += delimiter.size() - 1;
        } else {
            current += c;
        }
    }
    fields.append(current.trimmed());
    return fields;
}

QString CsvExchangeService::export_to_csv_string(const Schedule &schedule, const QString &delimiter) const
{
    QString out;
    QTextStream stream(&out);

    // Header
    QStringList headers = {
        QStringLiteral("WBS"),
        QStringLiteral("Nome"),
        QStringLiteral("Inicio"),
        QStringLiteral("Fim"),
        QStringLiteral("Duracao_Dias"),
        QStringLiteral("Progresso_Pct"),
        QStringLiteral("Predecessoras"),
        QStringLiteral("BIM_GUID"),
        QStringLiteral("Marco")
    };
    stream << headers.join(delimiter) << "\n";

    QList<ScheduleItem> items = schedule.get_sorted_by_wbs();

    // Map item ID -> WBS or name for predecessor notation
    QHash<QString, QString> id_to_wbs;
    for (const auto &item : items) {
        id_to_wbs.insert(item.id, item.wbs_code.isEmpty() ? item.name : item.wbs_code);
    }

    for (const auto &item : items) {
        QStringList preds_strs;
        for (const auto &dep : schedule.dependencies) {
            if (dep.successor_item_id == item.id && id_to_wbs.contains(dep.predecessor_item_id)) {
                QString notation = id_to_wbs[dep.predecessor_item_id] + dependency_type_to_string(dep.type);
                if (dep.lag_hours != 0.0) {
                    notation += QStringLiteral("%1%2h").arg(dep.lag_hours > 0 ? "+" : "").arg(dep.lag_hours);
                }
                preds_strs.append(notation);
            }
        }

        QString bim_guids;
        for (const auto &link : schedule.bim_links) {
            if (link.schedule_item_id == item.id) {
                if (!bim_guids.isEmpty()) bim_guids += QStringLiteral(";");
                bim_guids += link.bim_element_id;
            }
        }

        QStringList row = {
            escape_csv_cell(item.wbs_code, delimiter),
            escape_csv_cell(item.name, delimiter),
            escape_csv_cell(item.start_date.isValid() ? item.start_date.toString(QStringLiteral("yyyy-MM-dd HH:mm")) : QString(), delimiter),
            escape_csv_cell(item.end_date.isValid() ? item.end_date.toString(QStringLiteral("yyyy-MM-dd HH:mm")) : QString(), delimiter),
            QString::number(item.duration_hours / 8.0, 'f', 1),
            QString::number(item.percent_complete, 'f', 1),
            escape_csv_cell(preds_strs.join(QStringLiteral(", ")), delimiter),
            escape_csv_cell(bim_guids, delimiter),
            item.is_milestone ? QStringLiteral("1") : QStringLiteral("0")
        };

        stream << row.join(delimiter) << "\n";
    }

    return out;
}

bool CsvExchangeService::export_to_file(const Schedule &schedule, const QString &file_path, QString *error_message, const QString &delimiter) const
{
    QFile file(file_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error_message) *error_message = QStringLiteral("Não foi possível abrir o arquivo para escrita: %1").arg(file.errorString());
        return false;
    }
    QTextStream stream(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    stream.setCodec("UTF-8");
#endif
    stream << export_to_csv_string(schedule, delimiter);
    file.close();
    return true;
}

CsvImportResult CsvExchangeService::import_from_csv_string(Schedule &schedule,
                                                          const QString &csv_content,
                                                          const CsvColumnMapping &mapping,
                                                          const QString &delimiter)
{
    CsvImportResult result;
    QStringList lines = csv_content.split(QLatin1Char('\n'));

    if (lines.isEmpty()) {
        result.error_messages.append(QStringLiteral("Conteúdo CSV vazio."));
        return result;
    }

    QHash<QString, QString> wbs_or_name_to_id;
    struct DeferredDep {
        QString successor_id;
        QString notation;
        int line_num;
    };
    QList<DeferredDep> deferred_deps;

    int line_num = 0;
    for (const QString &raw_line : lines) {
        line_num++;
        QString line = raw_line.trimmed();
        if (line.isEmpty()) continue;

        QStringList fields = parse_csv_line(line, delimiter);
        if (fields.isEmpty()) continue;

        // Header detection
        if (line_num == 1) {
            QString first = fields.value(0).toLower();
            if (first.contains(QStringLiteral("wbs")) || first.contains(QStringLiteral("nome")) || first.contains(QStringLiteral("item"))) {
                continue; // skip header line
            }
        }

        QString wbs = fields.value(mapping.wbs_col);
        QString name = fields.value(mapping.name_col);
        if (name.isEmpty()) {
            result.warning_messages.append(QStringLiteral("Linha %1 ignorada: nome da atividade ausente.").arg(line_num));
            continue;
        }

        ScheduleItem item(name);
        item.wbs_code = wbs;

        QString start_str = fields.value(mapping.start_date_col);
        if (!start_str.isEmpty()) {
            item.start_date = QDateTime::fromString(start_str, QStringLiteral("yyyy-MM-dd HH:mm"));
            if (!item.start_date.isValid()) {
                item.start_date = QDateTime::fromString(start_str, Qt::ISODate);
            }
        }

        QString end_str = fields.value(mapping.end_date_col);
        if (!end_str.isEmpty()) {
            item.end_date = QDateTime::fromString(end_str, QStringLiteral("yyyy-MM-dd HH:mm"));
            if (!item.end_date.isValid()) {
                item.end_date = QDateTime::fromString(end_str, Qt::ISODate);
            }
        }

        bool ok_dur = false;
        double days = fields.value(mapping.duration_col).toDouble(&ok_dur);
        if (ok_dur && days >= 0.0) {
            item.duration_hours = days * 8.0;
        }

        bool ok_prog = false;
        double prog = fields.value(mapping.progress_col).toDouble(&ok_prog);
        if (ok_prog) {
            item.percent_complete = prog;
        }

        QString milestone_str = fields.value(mapping.is_milestone_col);
        if (milestone_str == QStringLiteral("1") || milestone_str.toLower() == QStringLiteral("true") || milestone_str.toLower() == QStringLiteral("sim")) {
            item.is_milestone = true;
            item.duration_hours = 0.0;
        }

        item.sanitize();

        QString add_err;
        if (!schedule.add_item(item, &add_err)) {
            result.error_messages.append(QStringLiteral("Linha %1: Erro ao adicionar atividade '%2': %3").arg(line_num).arg(name, add_err));
            continue;
        }

        result.items_imported++;
        if (!wbs.isEmpty()) wbs_or_name_to_id.insert(wbs, item.id);
        wbs_or_name_to_id.insert(name, item.id);

        // Predecessors deferral
        QString preds_cell = fields.value(mapping.predecessors_col);
        if (!preds_cell.isEmpty()) {
            deferred_deps.append({item.id, preds_cell, line_num});
        }

        // BIM GUID
        QString bim_cell = fields.value(mapping.bim_guid_col);
        if (!bim_cell.isEmpty()) {
            QStringList guids = bim_cell.split(QLatin1Char(';'));
            for (const QString &guid : guids) {
                QString trimmed_guid = guid.trimmed();
                if (!trimmed_guid.isEmpty()) {
                    BimLink link(item.id, trimmed_guid, BimSource::IFC);
                    schedule.add_bim_link(link);
                    result.bim_links_imported++;
                }
            }
        }
    }

    // Process deferred dependencies
    static const QRegularExpression dep_regex(QStringLiteral("^([A-Za-z0-9._-]+)(FS|SS|FF|SF)?([+-]?\\d+(?:\\.\\d+)?(?:d|h)?)?$"), QRegularExpression::CaseInsensitiveOption);

    for (const auto &d : deferred_deps) {
        QStringList tokens = d.notation.split(QRegularExpression(QStringLiteral("[,;]")), Qt::SkipEmptyParts);
        for (const QString &raw_token : tokens) {
            QString token = raw_token.trimmed();
            if (token.isEmpty()) continue;

            auto match = dep_regex.match(token);
            if (!match.hasMatch()) {
                result.warning_messages.append(QStringLiteral("Linha %1: Sintaxe de predecessora não reconhecida: '%2'").arg(d.line_num).arg(token));
                continue;
            }

            QString pred_key = match.captured(1).trimmed();
            QString type_str = match.captured(2).trimmed().toUpper();
            QString lag_str = match.captured(3).trimmed();

            if (!wbs_or_name_to_id.contains(pred_key)) {
                result.warning_messages.append(QStringLiteral("Linha %1: Predecessora '%2' não encontrada.").arg(d.line_num).arg(pred_key));
                continue;
            }

            QString pred_id = wbs_or_name_to_id[pred_key];
            DependencyType dep_type = DependencyType::FinishToStart;
            if (!type_str.isEmpty()) {
                dep_type = string_to_dependency_type(type_str);
            }

            double lag_hours = 0.0;
            if (!lag_str.isEmpty()) {
                bool is_days = lag_str.endsWith(QLatin1Char('d'), Qt::CaseInsensitive);
                QString num_part = lag_str;
                num_part.remove(QLatin1Char('d'), Qt::CaseInsensitive);
                num_part.remove(QLatin1Char('h'), Qt::CaseInsensitive);
                bool ok_num = false;
                double val = num_part.toDouble(&ok_num);
                if (ok_num) {
                    lag_hours = is_days ? (val * 8.0) : val;
                }
            }

            Dependency dep(pred_id, d.successor_id, dep_type, lag_hours);
            QString dep_err;
            if (schedule.add_dependency(dep, &dep_err)) {
                result.dependencies_imported++;
            } else {
                result.warning_messages.append(QStringLiteral("Linha %1: Não foi possível adicionar vínculo (%2): %3").arg(d.line_num).arg(token, dep_err));
            }
        }
    }

    result.success = (result.items_imported > 0);
    return result;
}

CsvImportResult CsvExchangeService::import_from_file(Schedule &schedule,
                                                    const QString &file_path,
                                                    const CsvColumnMapping &mapping,
                                                    const QString &delimiter)
{
    CsvImportResult result;
    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.error_messages.append(QStringLiteral("Não foi possível abrir o arquivo CSV: %1").arg(file.errorString()));
        return result;
    }
    QTextStream stream(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    stream.setCodec("UTF-8");
#endif
    QString content = stream.readAll();
    file.close();

    return import_from_csv_string(schedule, content, mapping, delimiter);
}

} // namespace source::gantt
