#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_13_on_speed_changed.hpp"

#include <QLabel>
#include <QMediaPlayer>

namespace source::plugin::Audiolivro {

void LeitorAcessivel::on_speed_changed(int value)
{
    if (label_velocidade_ != nullptr) {
        label_velocidade_->setText(tr("%1 wpm").arg(value));
    }

    if (player_ != nullptr && player_->playbackState() != QMediaPlayer::PlaybackState::StoppedState) {
        const double base = qMax(1, current_reading_speed_wpm_);
        player_->setPlaybackRate(qBound(0.25, value / base, 3.0));
    }
}

}  // namespace source::plugin::Audiolivro
