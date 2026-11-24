#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVariantList>

namespace source::gantt {

class EisenhowerTaskImportDialog : public QDialog {
    Q_OBJECT

public:
    explicit EisenhowerTaskImportDialog(QWidget *parent = nullptr);
    ~EisenhowerTaskImportDialog() override = default;

    int imported_count() const { return imported_count_; }

private slots:
    void on_filter_changed();
    void on_import_clicked();
    void on_select_all();

    void retranslate_ui();

protected:
    void changeEvent(QEvent *event) override;

private:
    QLabel *lbl_info_ = nullptr;
    QLineEdit *txt_search_ = nullptr;
    QComboBox *combo_quadrant_ = nullptr;
    QTableWidget *table_ = nullptr;
    QPushButton *btn_import_ = nullptr;
    QPushButton *btn_select_all_ = nullptr;
    QPushButton *btn_close_ = nullptr;
    QLabel *lbl_status_ = nullptr;

    QVariantList all_tasks_;
    int imported_count_ = 0;

    void setup_ui();
    void load_tasks();
    void populate_table();
};

} // namespace source::gantt
