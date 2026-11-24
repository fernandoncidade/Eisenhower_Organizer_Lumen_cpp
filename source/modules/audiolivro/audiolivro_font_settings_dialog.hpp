#pragma once

#include <QDialog>
#include <QFont>

class QCheckBox;
class QComboBox;
class QFontComboBox;
class QLabel;
class QPushButton;
class QSpinBox;

namespace source::plugin::Audiolivro {

class FontSettingsDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit FontSettingsDialog(const QFont &initial_font, QWidget *parent = nullptr);

    QFont selected_font() const;

protected:
    void changeEvent(QEvent *event) override;

private:
    void set_selected_font(const QFont &font);
    void update_style_options(const QString &preferred_style, bool keep_current_style_when_empty);
    void restore_defaults();
    void update_preview();
    void retranslate_ui();

    QLabel *family_label_ = nullptr;
    QLabel *style_label_ = nullptr;
    QLabel *size_label_ = nullptr;
    QLabel *format_label_ = nullptr;
    QLabel *preview_title_label_ = nullptr;
    QFontComboBox *family_combo_ = nullptr;
    QComboBox *style_combo_ = nullptr;
    QSpinBox *size_spin_ = nullptr;
    QCheckBox *bold_check_ = nullptr;
    QCheckBox *italic_check_ = nullptr;
    QCheckBox *underline_check_ = nullptr;
    QCheckBox *strike_out_check_ = nullptr;
    QLabel *preview_label_ = nullptr;
    QPushButton *restore_button_ = nullptr;
    QPushButton *ok_button_ = nullptr;
    QPushButton *cancel_button_ = nullptr;
};

}  // namespace source::plugin::Audiolivro
