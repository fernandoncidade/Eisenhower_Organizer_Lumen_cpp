#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_10_on_media_status.hpp"

#include <QMediaPlayer>
#include <QSlider>
#include <QThread>
#include <QUrl>

namespace source::plugin::Audiolivro {

void LeitorAcessivel::on_media_status(int status)
{
    if (status != static_cast<int>(QMediaPlayer::MediaStatus::EndOfMedia)) {
        return;
    }

    if (!generated_queue_.isEmpty() && player_ != nullptr) {
        current_generated_ = generated_queue_.takeFirst();
        ++current_chunk_index_;
        notify_chunk_changed(current_chunk_index_);
        notify_pdf_chunk_changed(current_chunk_index_);
        activate_timestamps_for_audio(current_generated_);
        if (slider_velocidade_ != nullptr) {
            const double base = qMax(1, current_reading_speed_wpm_);
            player_->setPlaybackRate(qBound(0.25, slider_velocidade_->value() / base, 3.0));
        }
        player_->setSource(QUrl::fromLocalFile(current_generated_));
        player_->play();
        return;
    }

    if (tts_thread_ != nullptr && tts_thread_->isRunning()) {
        return;
    }

    leitura_finalizada();
}

}  // namespace source::plugin::Audiolivro
