#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_06_aplicar_tema_dinamico_inputs.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTempoInternal;

void GerenciadorTempo::_aplicar_tema_dinamico_inputs()
{
    try {
        const QPalette palette = qApp->palette();
        const QColor window_color = palette.color(QPalette::Window);

        if (col_doing_timer_list != nullptr) {
            QPalette list_palette = col_doing_timer_list->palette();
            list_palette.setColor(QPalette::Base, window_color);
            list_palette.setColor(QPalette::AlternateBase, window_color);
            col_doing_timer_list->setPalette(list_palette);
            col_doing_timer_list->viewport()->setAutoFillBackground(true);
        }

        _ajustar_largura_minima_coluna_esquerda();
        update();
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao aplicar tema dinâmico (lista) no GerenciadorTempo: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao aplicar tema dinâmico (lista) no GerenciadorTempo"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTempoFns { void aplicar_tema_dinamico_inputs(GerenciadorTempo &self) { self._aplicar_tema_dinamico_inputs(); } }
