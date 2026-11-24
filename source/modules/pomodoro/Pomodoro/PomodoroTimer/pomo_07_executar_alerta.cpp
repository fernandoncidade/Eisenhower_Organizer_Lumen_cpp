#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_07_executar_alerta.hpp"

#include "source/modules/pomodoro/Pomodoro/PomodoroTimer/pomo_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace PomodoroTimerInternal;

void PomodoroTimer::_executar_alerta(const QString &titulo, const QString &texto, bool fechar_automaticamente)
{
    try {
        auto *msg = compat::MessageBox::create(this);
        msg->setIcon(QMessageBox::Information);
        msg->setWindowTitle(titulo);
        msg->setText(texto);

        const bool modo_automatico_ativo = fechar_automaticamente && chk_modo_automatico != nullptr
            && chk_modo_automatico->isChecked();
        if (sound_mgr_ != nullptr && modo_automatico_ativo) {
            connect(sound_mgr_,
                    &compat::SoundManager::maximum_repetitions_reached,
                    msg,
                    &QDialog::accept,
                    Qt::UniqueConnection);
        }

        connect(msg, &QDialog::finished, this, [this](int) {
            if (sound_mgr_ != nullptr) {
                sound_mgr_->stop();
            }
        });

        if (sound_mgr_ != nullptr) {
            const int maximum_repetitions =
                modo_automatico_ativo ? sound_mgr_->get_automatic_alarm_repetitions() : 0;
            QTimer::singleShot(0, msg, [sound_manager = sound_mgr_, maximum_repetitions]() {
                sound_manager->play_looping(maximum_repetitions);
            });
        }

        msg->exec();
        delete msg;
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao executar alerta com som: %1").arg(QString::fromUtf8(e.what())), true);
        compat::MessageBox::information(this, titulo, texto);
        if (sound_mgr_ != nullptr) {
            sound_mgr_->stop();
        }
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao executar alerta com som"), true);
        compat::MessageBox::information(this, titulo, texto);
        if (sound_mgr_ != nullptr) {
            sound_mgr_->stop();
        }
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::PomodoroTimerFns { void executar_alerta(PomodoroTimer &self, const QString &titulo, const QString &texto) { self._executar_alerta(titulo, texto); } }
