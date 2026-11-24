#include "MsProjectXmlConnector.hpp"
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace source::gantt {

MsProjectXmlConnector::MsProjectXmlConnector()
{
}

QString MsProjectXmlConnector::export_to_xml_string(const Schedule &schedule) const
{
    QString out;
    QXmlStreamWriter xml(&out);
    xml.setAutoFormatting(true);
    xml.writeStartDocument();

    xml.writeStartElement(QStringLiteral("Project"));
    xml.writeAttribute(QStringLiteral("xmlns"), QStringLiteral("http://schemas.microsoft.com/project"));

    xml.writeTextElement(QStringLiteral("Name"), schedule.name);
    xml.writeTextElement(QStringLiteral("Title"), schedule.name);
    xml.writeTextElement(QStringLiteral("Comments"), schedule.description);
    xml.writeTextElement(QStringLiteral("StartDate"), schedule.project_start_date().toString(Qt::ISODate));
    xml.writeTextElement(QStringLiteral("FinishDate"), schedule.project_finish_date().toString(Qt::ISODate));

    xml.writeStartElement(QStringLiteral("Tasks"));

    QList<ScheduleItem> items = schedule.get_sorted_by_wbs();
    int uid = 1;
    QHash<QString, int> id_to_uid;

    for (const auto &item : items) {
        id_to_uid.insert(item.id, uid);

        xml.writeStartElement(QStringLiteral("Task"));
        xml.writeTextElement(QStringLiteral("UID"), QString::number(uid));
        xml.writeTextElement(QStringLiteral("ID"), QString::number(uid));
        xml.writeTextElement(QStringLiteral("Name"), item.name);
        xml.writeTextElement(QStringLiteral("WBS"), item.wbs_code);
        xml.writeTextElement(QStringLiteral("Start"), item.start_date.toString(Qt::ISODate));
        xml.writeTextElement(QStringLiteral("Finish"), item.end_date.toString(Qt::ISODate));
        xml.writeTextElement(QStringLiteral("Duration"), QStringLiteral("PT%1H").arg(item.duration_hours));
        xml.writeTextElement(QStringLiteral("PercentComplete"), QString::number(static_cast<int>(item.percent_complete)));
        xml.writeTextElement(QStringLiteral("Milestone"), item.is_milestone ? QStringLiteral("1") : QStringLiteral("0"));
        xml.writeTextElement(QStringLiteral("Critical"), item.critical ? QStringLiteral("1") : QStringLiteral("0"));

        // Predecessors
        for (const auto &dep : schedule.dependencies) {
            if (dep.successor_item_id == item.id && id_to_uid.contains(dep.predecessor_item_id)) {
                xml.writeStartElement(QStringLiteral("PredecessorLink"));
                xml.writeTextElement(QStringLiteral("PredecessorUID"), QString::number(id_to_uid[dep.predecessor_item_id]));
                // Type: 1 = FF, 2 = FS, 3 = SF, 4 = SS
                int type_num = 1;
                switch (dep.type) {
                    case DependencyType::FinishToStart: type_num = 1; break;
                    case DependencyType::StartToStart:  type_num = 3; break;
                    case DependencyType::FinishToFinish: type_num = 0; break;
                    case DependencyType::StartToFinish: type_num = 2; break;
                }
                xml.writeTextElement(QStringLiteral("Type"), QString::number(type_num));
                xml.writeTextElement(QStringLiteral("LinkLag"), QString::number(static_cast<int>(dep.lag_hours * 600))); // lag in tenths of a minute
                xml.writeEndElement(); // PredecessorLink
            }
        }

        xml.writeEndElement(); // Task
        uid++;
    }

    xml.writeEndElement(); // Tasks
    xml.writeEndElement(); // Project
    xml.writeEndDocument();

    return out;
}

bool MsProjectXmlConnector::export_to_file(const Schedule &schedule, const QString &file_path, QString *error_message) const
{
    QFile file(file_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        if (error_message) *error_message = file.errorString();
        return false;
    }
    QTextStream stream(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    stream.setCodec("UTF-8");
#endif
    stream << export_to_xml_string(schedule);
    file.close();
    return true;
}

bool MsProjectXmlConnector::import_from_xml_string(Schedule &schedule, const QString &xml_content, QString *error_message)
{
    QXmlStreamReader reader(xml_content);

    QHash<int, QString> uid_to_id;
    struct DeferredLink {
        QString succ_id;
        int pred_uid;
        DependencyType type;
        double lag_hours;
    };
    QList<DeferredLink> links;

    while (!reader.atEnd() && !reader.hasError()) {
        QXmlStreamReader::TokenType token = reader.readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (reader.name() == QLatin1String("Task")) {
                ScheduleItem item;
                int current_uid = -1;

                while (!(reader.tokenType() == QXmlStreamReader::EndElement && reader.name() == QLatin1String("Task"))) {
                    reader.readNext();
                    if (reader.isStartElement()) {
                        QString tag = reader.name().toString();
                        if (tag == QStringLiteral("UID")) {
                            current_uid = reader.readElementText().toInt();
                        } else if (tag == QStringLiteral("Name")) {
                            item.name = reader.readElementText();
                        } else if (tag == QStringLiteral("WBS")) {
                            item.wbs_code = reader.readElementText();
                        } else if (tag == QStringLiteral("Start")) {
                            item.start_date = QDateTime::fromString(reader.readElementText(), Qt::ISODate);
                        } else if (tag == QStringLiteral("Finish")) {
                            item.end_date = QDateTime::fromString(reader.readElementText(), Qt::ISODate);
                        } else if (tag == QStringLiteral("Milestone")) {
                            item.is_milestone = (reader.readElementText() == QStringLiteral("1"));
                        } else if (tag == QStringLiteral("PercentComplete")) {
                            item.percent_complete = reader.readElementText().toDouble();
                        } else if (tag == QStringLiteral("PredecessorLink")) {
                            DeferredLink d_link;
                            d_link.succ_id = item.id;
                            d_link.type = DependencyType::FinishToStart;
                            d_link.lag_hours = 0.0;
                            while (!(reader.tokenType() == QXmlStreamReader::EndElement && reader.name() == QLatin1String("PredecessorLink"))) {
                                reader.readNext();
                                if (reader.isStartElement()) {
                                    if (reader.name() == QLatin1String("PredecessorUID")) {
                                        d_link.pred_uid = reader.readElementText().toInt();
                                    } else if (reader.name() == QLatin1String("Type")) {
                                        int t = reader.readElementText().toInt();
                                        if (t == 1) d_link.type = DependencyType::FinishToStart;
                                        else if (t == 3) d_link.type = DependencyType::StartToStart;
                                        else if (t == 0) d_link.type = DependencyType::FinishToFinish;
                                        else if (t == 2) d_link.type = DependencyType::StartToFinish;
                                    }
                                }
                            }
                            links.append(d_link);
                        }
                    }
                }

                if (!item.name.isEmpty()) {
                    item.sanitize();
                    schedule.add_item(item);
                    if (current_uid != -1) {
                        uid_to_id.insert(current_uid, item.id);
                    }
                }
            }
        }
    }

    if (reader.hasError()) {
        if (error_message) *error_message = reader.errorString();
        return false;
    }

    // Connect links
    for (const auto &link : links) {
        if (uid_to_id.contains(link.pred_uid)) {
            Dependency dep(uid_to_id[link.pred_uid], link.succ_id, link.type, link.lag_hours);
            schedule.add_dependency(dep);
        }
    }

    return true;
}

bool MsProjectXmlConnector::import_from_file(Schedule &schedule, const QString &file_path, QString *error_message)
{
    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        if (error_message) *error_message = file.errorString();
        return false;
    }
    QTextStream stream(&file);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    stream.setCodec("UTF-8");
#endif
    QString content = stream.readAll();
    file.close();
    return import_from_xml_string(schedule, content, error_message);
}

} // namespace source::gantt
