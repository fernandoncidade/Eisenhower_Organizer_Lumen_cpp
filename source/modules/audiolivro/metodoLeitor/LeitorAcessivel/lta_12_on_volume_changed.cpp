#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_12_on_volume_changed.hpp"

#include <QAudioOutput>

namespace source::plugin::Audiolivro {

void LeitorAcessivel::on_volume_changed(int value)
{
    if (audio_output_ != nullptr) {
        audio_output_->setVolume(qBound(0, value, 100) / 100.0);
    }
}

}  // namespace source::plugin::Audiolivro
