#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_09_play_generated_audio.hpp"

#include <QMediaPlayer>
#include <QSlider>
#include <QUrl>

namespace source::plugin::Audiolivro {

void LeitorAcessivel::play_generated_audio(const QString &path)
{
    if (path.trimmed().isEmpty() || player_ == nullptr) {
        return;
    }

    generated_files_.append(path);
    if (player_->playbackState() == QMediaPlayer::PlaybackState::PlayingState
        || player_->playbackState() == QMediaPlayer::PlaybackState::PausedState) {
        generated_queue_.append(path);
        return;
    }

    current_generated_ = path;
    if (slider_velocidade_ != nullptr) {
        const double base = qMax(1, current_reading_speed_wpm_);
        player_->setPlaybackRate(qBound(0.25, slider_velocidade_->value() / base, 3.0));
    }
    player_->setSource(QUrl::fromLocalFile(path));
    player_->play();
}

}  // namespace source::plugin::Audiolivro
