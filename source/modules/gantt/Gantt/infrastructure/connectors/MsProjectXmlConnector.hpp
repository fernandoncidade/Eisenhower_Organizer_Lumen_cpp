#pragma once

#include <QString>
#include "../../domain/models/Schedule.hpp"

namespace source::gantt {

class MsProjectXmlConnector {
public:
    MsProjectXmlConnector();

    QString export_to_xml_string(const Schedule &schedule) const;
    bool export_to_file(const Schedule &schedule, const QString &file_path, QString *error_message = nullptr) const;

    bool import_from_xml_string(Schedule &schedule, const QString &xml_content, QString *error_message = nullptr);
    bool import_from_file(Schedule &schedule, const QString &file_path, QString *error_message = nullptr);
};

} // namespace source::gantt
