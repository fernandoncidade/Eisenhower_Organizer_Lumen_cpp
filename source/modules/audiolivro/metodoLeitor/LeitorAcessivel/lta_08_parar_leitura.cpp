#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_08_parar_leitura.hpp"

#include "source/modules/audiolivro/metodoLeitor/lt_01_TTSThread.hpp"
#include "source/modules/audiolivro/metodoLeitor/lt_02_EdgeTTSThread.hpp"
#include "source/modules/audiolivro/metodoLeitor/lt_04_EdgeTTSWithTimestamps.hpp"

#include <QMediaPlayer>
#include <QPushButton>
#include <QUrl>

namespace source::plugin::Audiolivro {

void LeitorAcessivel::parar_leitura()
{
    QStringList files_to_cleanup = generated_files_;
    if (!current_generated_.trimmed().isEmpty()) {
        files_to_cleanup.append(current_generated_);
    }
    files_to_cleanup.append(generated_queue_);

    if (tts_thread_ != nullptr) {
        tts_thread_->setParent(nullptr);
        QObject::disconnect(tts_thread_.data(), nullptr, this, nullptr);
        if (auto *edge = qobject_cast<EdgeTTSWithTimestamps *>(tts_thread_.data())) {
            edge->stop();
        } else if (auto *edge_plain = qobject_cast<EdgeTTSThread *>(tts_thread_.data())) {
            edge_plain->stop();
        } else if (auto *tts = qobject_cast<TTSThread *>(tts_thread_.data())) {
            tts->stop();
        } else {
            tts_thread_->requestInterruption();
        }
        tts_thread_->quit();
        tts_thread_->wait(1200);
        tts_thread_ = nullptr;
    }

    if (player_ != nullptr) {
        player_->stop();
        player_->setSource(QUrl());
    }

    generated_queue_.clear();
    generated_timestamps_.clear();
    current_timestamps_.clear();
    current_generated_.clear();
    cleanup_paths_with_retry(files_to_cleanup);
    generated_files_.clear();
    cleanup_edge_tts_temp_in_outdir(true, true);
    current_reading_is_pdf_ = false;
    is_paused_ = false;
    stop_speech_highlight();
    stop_pdf_speech_highlight();

    if (btn_play_ != nullptr) btn_play_->setEnabled(true);
    if (btn_pause_ != nullptr) btn_pause_->setEnabled(false);
    if (btn_stop_ != nullptr) btn_stop_->setEnabled(false);
    update_pause_button();
}

}  // namespace source::plugin::Audiolivro
