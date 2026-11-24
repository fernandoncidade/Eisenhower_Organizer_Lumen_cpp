#pragma once

#include <QDialog>
#include <QSpinBox>
#include "../application/services/CsvExchangeService.hpp"

namespace source::gantt {

class CsvMappingDialog : public QDialog {
    Q_OBJECT

public:
    explicit CsvMappingDialog(const QStringList &preview_headers = QStringList(), QWidget *parent = nullptr);

    CsvColumnMapping mapping() const;

private:
    QSpinBox *spin_wbs_ = nullptr;
    QSpinBox *spin_name_ = nullptr;
    QSpinBox *spin_start_ = nullptr;
    QSpinBox *spin_end_ = nullptr;
    QSpinBox *spin_duration_ = nullptr;
    QSpinBox *spin_progress_ = nullptr;
    QSpinBox *spin_preds_ = nullptr;
    QSpinBox *spin_bim_ = nullptr;
    QSpinBox *spin_milestone_ = nullptr;

    void setup_ui(const QStringList &preview_headers);
};

} // namespace source::gantt
