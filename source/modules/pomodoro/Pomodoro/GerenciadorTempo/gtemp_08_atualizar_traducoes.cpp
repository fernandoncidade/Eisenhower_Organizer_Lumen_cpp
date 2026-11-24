#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_08_atualizar_traducoes.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_internal.hpp"

#include <QEvent>

namespace source::plugin::Pomodoro {
using namespace GerenciadorTempoInternal;

void GerenciadorTempo::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event != nullptr && event->type() == QEvent::LanguageChange) {
        atualizar_traducoes();
    }
}

void GerenciadorTempo::atualizar_traducoes()
{
    try {
        if (tarefas_group != nullptr) {
            tarefas_group->setTitle(QCoreApplication::translate("PomodoroPlugin", "📊 Quadro de Tarefas"));
        }

        if (pomodoro != nullptr) {
            pomodoro->atualizar_traducoes();
        }

        if (tarefas != nullptr) {
            tarefas->atualizar_traducoes();
        }

        if (col_doing_timer_group != nullptr) {
            col_doing_timer_group->setTitle(QCoreApplication::translate("PomodoroPlugin", "⚙️ Em Progresso"));
        }

        atualizar_coluna_doing_timer();

        if (btn_toggle_painel_em_progresso_ != nullptr) {
            const QString expand_text = expand_in_progress_panel_text(this);
            const QString collapse_text = collapse_in_progress_panel_text(this);
            btn_toggle_painel_em_progresso_->setSizeReferenceTexts({expand_text, collapse_text});
            btn_toggle_painel_em_progresso_->setText(painel_em_progresso_expandido_ ? collapse_text : expand_text);
        }

        _ajustar_limites_verticais();
        _ajustar_largura_minima_coluna_esquerda();
        _ajustar_limites_splitter_principal();
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao atualizar traduções do GerenciadorTempo: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao atualizar traduções do GerenciadorTempo"), true);
    }
}

void GerenciadorTempo::_ajustar_limites_verticais()
{
    const int bounded_height = bounded_module_min_height(this);
    if (minimumHeight() != bounded_height) {
        setMinimumHeight(bounded_height);
    }

    if (pomodoro != nullptr) {
        const int reference_height = height() > 0 ? height() : available_window_size(this).height();
        int timer_max_height = 460;
        if (_layout_robusto()) {
            timer_max_height = qBound(180, reference_height / 3, 320);
        } else {
            timer_max_height = painel_em_progresso_expandido_ ? qBound(200, reference_height / 3, 340)
                                                              : qBound(260, reference_height / 2, 460);
        }
        timer_max_height =
            qMax(timer_max_height, qMax(pomodoro->minimumHeight(), pomodoro->minimumSizeHint().height()));
        if (pomodoro->maximumHeight() != timer_max_height) {
            pomodoro->setMaximumHeight(timer_max_height);
        }
        pomodoro->updateGeometry();
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTempoFns { void atualizar_traducoes(GerenciadorTempo &self) { self.atualizar_traducoes(); } }
