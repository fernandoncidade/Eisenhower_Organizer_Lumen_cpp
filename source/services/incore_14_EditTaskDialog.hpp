#pragma once

#include <QDialog>
#include <QLocale>
#include <QVariant>

class QCheckBox;
class QComboBox;
class QDateEdit;
class QDoubleSpinBox;
class QEvent;
class QLabel;
class QLineEdit;
class QPushButton;
class QResizeEvent;
class QSpinBox;
class QTextEdit;
class QTimeEdit;
class QVBoxLayout;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::InterfaceCore::incore_14_EditTaskDialog {

QString get_text(const QString &text);
int compute_widget_height(source::EisenhowerMatrixApp *app, QWidget *widget, const QString &name = QString());
void apply_widget_min_height(source::EisenhowerMatrixApp *app, QWidget *widget, const QString &name = QString());
QLocale effective_locale(source::EisenhowerMatrixApp *app);
void apply_locale_to_dateedit(source::EisenhowerMatrixApp *app, QDateEdit *date_edit);

class EditTaskDialog : public QDialog {
public:
    EditTaskDialog(source::EisenhowerMatrixApp *app,
                   QWidget *parent = nullptr,
                   const QString &initial_text = QString(),
                   const QString &initial_file = QString(),
                   const QString &initial_description = QString(),
                   const QString &initial_date_iso = QString(),
                   const QString &initial_time_str = QString(),
                   const QVariant &initial_priority = QVariant(),
                   const QVariant &initial_domain = QVariant(),
                   int initial_pomodoros = 0,
                   double initial_duration = 1.0,
                   int initial_ciclos_totais = 0);

    struct Values {
        QString text;
        QString file_path;
        QString description;
        QString date_iso;
        QString time_str;
        int priority = 1;
        int domain = 0;
        int pomodoros = 0;
        double duration = 1.0;
        int ciclos_totais = 4;
    };

    Values get_values() const;
    void retranslateUi();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    enum class LayoutMode { None, Compact, Wide };
    void _update_layout_mode(int width, int height);
    void _apply_compact_layout();
    void _apply_wide_layout();

    source::EisenhowerMatrixApp *app_ = nullptr;
    QLineEdit *title_ = nullptr;
    QLineEdit *file_field_ = nullptr;
    QTextEdit *desc_ = nullptr;
    QCheckBox *date_enabled_ = nullptr;
    QDateEdit *date_edit_ = nullptr;
    QCheckBox *time_enabled_ = nullptr;
    QTimeEdit *time_edit_ = nullptr;
    QComboBox *priority_combo_ = nullptr;
    QComboBox *domain_combo_ = nullptr;
    QSpinBox *pomodoros_spin_ = nullptr;
    QSpinBox *ciclos_totais_spin_ = nullptr;
    QDoubleSpinBox *duration_spin_ = nullptr;
    QStringList priority_labels_;

    QLabel *label_tarefa_ = nullptr;
    QLabel *label_prioridade_ = nullptr;
    QLabel *label_dominio_ = nullptr;
    QLabel *label_pomodoro_ = nullptr;
    QLabel *label_ciclos_ = nullptr;
    QLabel *label_duracao_ = nullptr;
    QLabel *label_arquivo_ = nullptr;
    QPushButton *btn_choose_ = nullptr;
    QPushButton *btn_open_ = nullptr;
    QPushButton *btn_clear_ = nullptr;
    QLabel *label_desc_ = nullptr;
    QLabel *label_data_hora_ = nullptr;
    QPushButton *btn_cancel_ = nullptr;
    QPushButton *btn_ok_ = nullptr;

    LayoutMode current_layout_mode_ = LayoutMode::None;
    QWidget *top_fields_container_ = nullptr;
    QWidget *desc_container_ = nullptr;
    QWidget *bottom_fields_container_ = nullptr;
    QVBoxLayout *root_layout_ = nullptr;
    int top_fields_height_ = 0;
    int bottom_fields_height_ = 0;
};

}  // namespace source::InterfaceCore::incore_14_EditTaskDialog
