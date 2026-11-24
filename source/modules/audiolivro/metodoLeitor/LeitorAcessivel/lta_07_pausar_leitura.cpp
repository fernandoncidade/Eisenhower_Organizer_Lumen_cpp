#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_07_pausar_leitura.hpp"

#include <QMediaPlayer>

namespace source::plugin::Audiolivro {

void LeitorAcessivel::pausar_leitura()
{
    if (player_ == nullptr) {
        return;
    }

    if (player_->playbackState() == QMediaPlayer::PlaybackState::PlayingState) {
        player_->pause();
        is_paused_ = true;
        pause_speech_highlight();
        pause_pdf_speech_highlight();
    } else if (player_->playbackState() == QMediaPlayer::PlaybackState::PausedState) {
        player_->play();
        is_paused_ = false;
        resume_speech_highlight();
        resume_pdf_speech_highlight();
    }
    update_pause_button();
}

}  // namespace source::plugin::Audiolivro
