#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_20_definir_voz.hpp"

#include "source/modules/audiolivro/audiolivro_settings.hpp"
#include "source/modules/audiolivro/metodoLeitor/lt_06_VoiceCatalog.hpp"

#include <QMediaPlayer>

namespace source::plugin::Audiolivro {

void LeitorAcessivel::definir_voz(const QString &voz_id)
{
    QString normalized = voz_id.trimmed();
    if (normalized.isEmpty()) {
        normalized = default_voice_id();
    }

    if (voz_id_atual_ == normalized) {
        return;
    }

    if (tts_thread_ != nullptr || (player_ != nullptr && player_->playbackState() != QMediaPlayer::StoppedState)) {
        parar_leitura();
    }

    voz_id_atual_ = normalized;
    salvar_configuracao_texto(QStringLiteral("voice_id"), voz_id_atual_);
}

QString LeitorAcessivel::voz_atual() const
{
    return voz_id_atual_.trimmed().isEmpty() ? default_voice_id() : voz_id_atual_;
}

QVariantList LeitorAcessivel::vozes_disponiveis() const
{
    return available_voice_options();
}

}  // namespace source::plugin::Audiolivro
