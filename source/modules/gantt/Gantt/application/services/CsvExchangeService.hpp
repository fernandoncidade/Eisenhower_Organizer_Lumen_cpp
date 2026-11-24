#pragma once

#include <QHash>
#include <QList>
#include <QString>
#include "../../domain/models/Schedule.hpp"

namespace source::gantt {

struct CsvColumnMapping {
    int wbs_col = 0;
    int name_col = 1;
    int start_date_col = 2;
    int end_date_col = 3;
    int duration_col = 4;
    int progress_col = 5;
    int predecessors_col = 6;
    int bim_guid_col = 7;
    int is_milestone_col = 8;

    static CsvColumnMapping default_mapping();
};

struct CsvImportResult {
    bool success = false;
    int items_imported = 0;
    int dependencies_imported = 0;
    int bim_links_imported = 0;
    QStringList error_messages;
    QStringList warning_messages;
};

class CsvExchangeService {
public:
    CsvExchangeService();

    // Export schedule to CSV string or file
    QString export_to_csv_string(const Schedule &schedule, const QString &delimiter = QStringLiteral(",")) const;
    bool export_to_file(const Schedule &schedule, const QString &file_path, QString *error_message = nullptr, const QString &delimiter = QStringLiteral(",")) const;

    // Import CSV into existing schedule
    CsvImportResult import_from_csv_string(Schedule &schedule,
                                          const QString &csv_content,
                                          const CsvColumnMapping &mapping = CsvColumnMapping::default_mapping(),
                                          const QString &delimiter = QStringLiteral(","));

    CsvImportResult import_from_file(Schedule &schedule,
                                    const QString &file_path,
                                    const CsvColumnMapping &mapping = CsvColumnMapping::default_mapping(),
                                    const QString &delimiter = QStringLiteral(","));
};

} // namespace source::gantt
