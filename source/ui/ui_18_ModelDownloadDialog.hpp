#pragma once

#include <QDialog>
#include <QElapsedTimer>
#include <QFile>
#include <QPointer>
#include <QUrl>

class QCloseEvent;
class QEvent;
class QFrame;
class QGroupBox;
class QLabel;
class QNetworkAccessManager;
class QNetworkReply;
class QProgressBar;
class QPushButton;
class QShowEvent;

namespace source {
class EisenhowerMatrixApp;
}

namespace source::GerenciamentoUI {

class ModelDownloadDialog : public QDialog {
    Q_OBJECT

public:
    explicit ModelDownloadDialog(source::EisenhowerMatrixApp *app, QWidget *parent = nullptr);
    ~ModelDownloadDialog() override;

signals:
    void model_download_status_changed();

public slots:
    void retranslate_ui();
    void refresh_theme();
    void refresh_status();

protected:
    void changeEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_start_clicked();
    void on_pause_clicked();
    void on_cancel_clicked();
    void on_delete_clicked();
    void on_open_folder_clicked();

    void on_network_reply_progress(qint64 bytes_received, qint64 bytes_total);
    void on_network_reply_ready_read();
    void on_network_reply_finished();
    void on_network_reply_error();

private:
    enum class DownloadState {
        Idle,
        Downloading,
        Paused,
        Finished,
        Error
    };

    void iniciar_download(bool retomar);
    void pausar_download();
    void cancelar_download(bool confirmar = true);
    void atualizar_widgets_estado();
    void aplicar_estilos_tema();
    void agendar_refresh_tema();

    QString format_bytes(qint64 bytes) const;
    QString format_speed(double bytes_per_sec) const;
    QString format_eta(int seconds) const;

    source::EisenhowerMatrixApp *app_ = nullptr;
    QNetworkAccessManager *network_manager_ = nullptr;
    QPointer<QNetworkReply> current_reply_;
    QFile part_file_;
    QUrl current_download_url_;

    DownloadState state_ = DownloadState::Idle;
    qint64 bytes_offset_ = 0;
    qint64 total_expected_bytes_ = 2104932768LL;
    qint64 last_bytes_received_ = 0;
    double smoothed_speed_ = 0.0;
    QElapsedTimer speed_timer_;
    int redirect_count_ = 0;

    // UI elements
    QFrame *header_frame_ = nullptr;
    QLabel *title_label_ = nullptr;
    QLabel *sub_label_ = nullptr;

    QGroupBox *info_group_ = nullptr;
    QLabel *lbl_name_title_ = nullptr;
    QLabel *model_name_label_ = nullptr;
    QLabel *lbl_source_title_ = nullptr;
    QLabel *model_source_label_ = nullptr;
    QLabel *lbl_size_title_ = nullptr;
    QLabel *model_size_label_ = nullptr;
    QLabel *lbl_path_title_ = nullptr;
    QLabel *model_path_label_ = nullptr;

    QGroupBox *progress_group_ = nullptr;
    QLabel *status_label_ = nullptr;
    QProgressBar *progress_bar_ = nullptr;
    QLabel *progress_details_label_ = nullptr;
    QLabel *speed_eta_label_ = nullptr;

    QPushButton *btn_start_resume_ = nullptr;
    QPushButton *btn_pause_ = nullptr;
    QPushButton *btn_cancel_ = nullptr;
    QPushButton *btn_delete_ = nullptr;
    QPushButton *btn_open_folder_ = nullptr;
    QPushButton *btn_close_ = nullptr;

    bool applying_theme_ = false;
    bool theme_refresh_pending_ = false;
    bool checked_http_response_ = false;
};

}  // namespace source::GerenciamentoUI
