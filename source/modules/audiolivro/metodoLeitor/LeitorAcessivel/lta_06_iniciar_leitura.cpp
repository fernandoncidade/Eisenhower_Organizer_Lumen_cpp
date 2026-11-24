#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_06_iniciar_leitura.hpp"

#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_37_FileDropEdit.hpp"
#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_38_FileDropPDFView.hpp"
#include "source/modules/audiolivro/metodoLeitor/lt_01_TTSThread.hpp"
#include "source/modules/audiolivro/metodoLeitor/lt_04_EdgeTTSWithTimestamps.hpp"
#include "source/modules/audiolivro/metodoLeitor/lt_06_VoiceCatalog.hpp"

#include <QAudioOutput>
#include <QMediaPlayer>
#include <QPushButton>
#include <QSlider>
#include <QTabWidget>
#include <QTextCursor>

namespace source::plugin::Audiolivro {

QString LeitorAcessivel::selected_or_full_text() const
{
    if (texto_area_ == nullptr) {
        return QString();
    }

    const QTextCursor cursor = texto_area_->textCursor();
    if (cursor.hasSelection()) {
        return cursor.selectedText().replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
    }
    return texto_area_->toPlainText();
}

void LeitorAcessivel::iniciar_leitura()
{
    if (player_ != nullptr && player_->playbackState() == QMediaPlayer::PlaybackState::PausedState) {
        player_->play();
        is_paused_ = false;
        update_pause_button();
        resume_speech_highlight();
        resume_pdf_speech_highlight();
        return;
    }

    const bool active_pdf = content_stack_ != nullptr && content_stack_->currentIndex() == 1;
    parar_leitura();

    current_reading_is_pdf_ = active_pdf;
    QString texto;
    if (active_pdf) {
        const int page = pdf_view_ != nullptr ? pdf_view_->current_page() : -1;
        texto = setup_pdf_speech_highlight_for_reading(last_pdf_path_, QString(), page);
    } else {
        texto = selected_or_full_text();
        setup_speech_highlight_for_reading(texto);
    }

    if (texto.trimmed().isEmpty()) {
        current_reading_is_pdf_ = false;
        return;
    }

    const int velocidade = slider_velocidade_ != nullptr ? slider_velocidade_->value() : 150;
    current_reading_speed_wpm_ = qMax(1, velocidade);
    const int rate_pct = qBound(-50, int(qRound((velocidade - 150) / 2.0)), 50);
    if (audio_output_ != nullptr && slider_volume_ != nullptr) {
        audio_output_->setVolume(slider_volume_->value() / 100.0);
    }
    if (player_ != nullptr) {
        player_->setPlaybackRate(1.0);
    }

    if (usar_edge_tts_) {
        auto *thread = new EdgeTTSWithTimestamps(texto,
                                                 voz_atual(),
                                                 rate_pct,
                                                 0,
                                                 QString(),
                                                 this);
        tts_thread_ = thread;
        connect(thread, &EdgeTTSWithTimestamps::chunk_ready, this, [this](const QString &audio_path, const QVariantList &timestamps) {
            register_generated_audio_timestamps(audio_path, timestamps);
            play_generated_audio(audio_path);
            if (current_generated_ == audio_path) {
                activate_timestamps_for_audio(audio_path);
            }
        });
        connect(thread, &EdgeTTSWithTimestamps::error, this, &LeitorAcessivel::on_tts_error);
        connect(thread, &QThread::finished, this, [this, thread] {
            if (tts_thread_ == thread) {
                tts_thread_ = nullptr;
            }

            const bool no_audio_generated = current_generated_.trimmed().isEmpty()
                                            && generated_queue_.isEmpty()
                                            && generated_files_.isEmpty();
            if (no_audio_generated && player_ != nullptr
                && player_->playbackState() == QMediaPlayer::PlaybackState::StoppedState) {
                leitura_finalizada();
            }
        });
        connect(thread, &QThread::finished, thread, &QObject::deleteLater);
        thread->start();

        if (active_pdf) {
            start_pdf_speech_highlight();
        } else {
            start_speech_highlight();
        }
    } else {
        auto *thread = new TTSThread(texto, velocidade, slider_volume_ != nullptr ? slider_volume_->value() / 100.0 : 1.0, voz_atual(), this);
        tts_thread_ = thread;
        connect(thread, &TTSThread::error, this, &LeitorAcessivel::on_tts_error);
        connect(thread, &QThread::finished, this, &LeitorAcessivel::leitura_finalizada);
        connect(thread, &QThread::finished, thread, &QObject::deleteLater);
        thread->start();
    }

    is_paused_ = false;
    if (btn_play_ != nullptr) btn_play_->setEnabled(false);
    if (btn_pause_ != nullptr) btn_pause_->setEnabled(true);
    if (btn_stop_ != nullptr) btn_stop_->setEnabled(true);
    update_pause_button();
}

}  // namespace source::plugin::Audiolivro
