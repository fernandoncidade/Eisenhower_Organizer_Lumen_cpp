#include "source/modules/audiolivro/metodoLeitor/lt_03_LeitorAcessivel.hpp"

#include "source/modules/audiolivro/audiolivro_settings.hpp"
#include "source/modules/audiolivro/metodoLeitor/lt_06_VoiceCatalog.hpp"

#include <QAudioOutput>
#include <QMediaPlayer>
#include <QSlider>

namespace source::plugin::Audiolivro {

LeitorAcessivel::LeitorAcessivel(QWidget *parent)
    : QWidget(parent)
{
    voz_id_atual_ = carregar_configuracao_texto(QStringLiteral("voice_id"), default_voice_id()).trimmed();
    if (voz_id_atual_.isEmpty()) {
        voz_id_atual_ = default_voice_id();
    }

    setup_ui();
    aplicar_fonte_configurada();

    player_ = new QMediaPlayer(this);
    audio_output_ = new QAudioOutput(this);
    player_->setAudioOutput(audio_output_);

    if (slider_volume_ != nullptr) {
        audio_output_->setVolume(slider_volume_->value() / 100.0);
    }

    connect(player_, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status) {
        on_media_status(static_cast<int>(status));
    });

    init_speech_highlight();
    init_pdf_speech_highlight();
    init_leitura_assistida_state();
}

LeitorAcessivel::~LeitorAcessivel()
{
    cleanup();
}

int LeitorAcessivel::total_task_count() const
{
    return 0;
}

int LeitorAcessivel::selected_task_count() const
{
    return 0;
}

void LeitorAcessivel::reload_shared_state()
{
}

void LeitorAcessivel::refresh_task_views()
{
    emit task_counter_changed();
}

}  // namespace source::plugin::Audiolivro
