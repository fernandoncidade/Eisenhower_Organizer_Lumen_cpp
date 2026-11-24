#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_02_atualizar_rotulo_botao_eisenhower.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTempoInternal;

void GerenciadorTempo::_atualizar_rotulo_botao_eisenhower()
{
    try {
        if (btn_eisenhower_integrado == nullptr) {
            return;
        }
    } catch (const std::exception &e) {
        logger_->debug(QStringLiteral("Falha ao atualizar rótulo do botão Eisenhower: %1").arg(QString::fromUtf8(e.what())));
    } catch (...) {
        logger_->debug(QStringLiteral("Falha ao atualizar rótulo do botão Eisenhower"));
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTempoFns { void atualizar_rotulo_botao_eisenhower(GerenciadorTempo &self) { self._atualizar_rotulo_botao_eisenhower(); } }
