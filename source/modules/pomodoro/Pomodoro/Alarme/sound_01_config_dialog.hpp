#pragma once

#include <QDialog>

#include "source/modules/pomodoro/Pomodoro/pomodoro_support.hpp"

class QComboBox;
class QFormLayout;
class QGroupBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QCloseEvent;

namespace source::plugin::Pomodoro::Alarme {

class SoundConfigDialog final : public QDialog {
    Q_OBJECT

public:
    explicit SoundConfigDialog(QWidget *parent = nullptr);

    void retranslate_ui();

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    void on_sound_changed(int index);
    void update_duration_label();
    void update_total_label();
    void on_play();
    void on_stop_preview();
    void on_save();

    source::utils::LogManager *logger_ = nullptr;
    compat::SoundManager *sound_manager_ = nullptr;

    QGroupBox *sound_group_ = nullptr;
    QLabel *sound_hint_label_ = nullptr;
    QComboBox *sound_combo_ = nullptr;
    QLabel *duration_label_ = nullptr;
    QPushButton *play_button_ = nullptr;
    QPushButton *stop_preview_button_ = nullptr;

    QGroupBox *interval_group_ = nullptr;
    QFormLayout *interval_layout_ = nullptr;
    QSpinBox *interval_spin_ = nullptr;
    QLabel *total_label_ = nullptr;
    QLabel *hint_label_ = nullptr;

    QPushButton *save_button_ = nullptr;
    QPushButton *cancel_button_ = nullptr;
};

class AutomaticModeConfigDialog final : public QDialog {
    Q_OBJECT

public:
    explicit AutomaticModeConfigDialog(QWidget *parent = nullptr);

    void retranslate_ui();

protected:
    void changeEvent(QEvent *event) override;

private:
    void on_save();

    source::utils::LogManager *logger_ = nullptr;
    compat::SoundManager *sound_manager_ = nullptr;
    QGroupBox *automatic_group_ = nullptr;
    QFormLayout *automatic_layout_ = nullptr;
    QSpinBox *repetitions_spin_ = nullptr;
    QLabel *hint_label_ = nullptr;
    QPushButton *save_button_ = nullptr;
    QPushButton *cancel_button_ = nullptr;
};

}  // namespace source::plugin::Pomodoro::Alarme
