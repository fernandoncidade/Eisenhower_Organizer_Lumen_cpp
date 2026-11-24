#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_14_leitura_finalizada.hpp"

#include <QMediaPlayer>
#include <QPushButton>
#include <QUrl>

namespace source::plugin::Audiolivro {

void LeitorAcessivel::leitura_finalizada()
{
    is_paused_ = false;
    if (btn_play_ != nullptr) btn_play_->setEnabled(true);
    if (btn_pause_ != nullptr) btn_pause_->setEnabled(false);
    if (btn_stop_ != nullptr) btn_stop_->setEnabled(false);
    update_pause_button();
    stop_speech_highlight();
    stop_pdf_speech_highlight();
    if (player_ != nullptr) {
        player_->setSource(QUrl());
    }
    cleanup_paths_with_retry(generated_files_);
    generated_files_.clear();
    generated_queue_.clear();
    generated_timestamps_.clear();
    current_timestamps_.clear();
    current_generated_.clear();
    current_reading_is_pdf_ = false;
    cleanup_edge_tts_temp_in_outdir(true, true);
}

}  // namespace source::plugin::Audiolivro
