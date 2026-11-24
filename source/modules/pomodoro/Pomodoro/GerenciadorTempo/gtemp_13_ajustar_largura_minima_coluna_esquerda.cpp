#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_13_ajustar_largura_minima_coluna_esquerda.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTempoInternal;

int GerenciadorTempo::_largura_minima_coluna_esquerda_efetiva() const
{
    int left_min_width = 320;
    if (pomodoro != nullptr) {
        left_min_width = qMax(left_min_width,
                              qMax(pomodoro->minimumWidth(),
                                   qMax(pomodoro->minimumSizeHint().width(), pomodoro->sizeHint().width())));
    }
    if (tarefas_group != nullptr) {
        left_min_width = qMax(left_min_width,
                              qMax(tarefas_group->minimumWidth(),
                                   qMax(tarefas_group->minimumSizeHint().width(), tarefas_group->sizeHint().width())));
    }
    if (!_layout_robusto()) {
        if (btn_toggle_painel_em_progresso_ != nullptr) {
            left_min_width = qMax(left_min_width,
                                  qMax(btn_toggle_painel_em_progresso_->minimumWidth(),
                                       qMax(btn_toggle_painel_em_progresso_->minimumSizeHint().width(),
                                            btn_toggle_painel_em_progresso_->sizeHint().width())));
        }
        if (col_doing_timer_group != nullptr && col_doing_timer_group->isVisible()) {
            left_min_width = qMax(left_min_width,
                                  qMax(col_doing_timer_group->minimumWidth(),
                                       qMax(col_doing_timer_group->minimumSizeHint().width(),
                                            col_doing_timer_group->sizeHint().width())));
        }
    }
    return left_min_width;
}

int GerenciadorTempo::_largura_minima_coluna_direita_efetiva() const
{
    if (tarefas == nullptr) {
        return 0;
    }

    int tabs_width = 0;
    if (tarefas->status_tabs != nullptr && tarefas->status_tabs->tabBar() != nullptr) {
        QTabBar *tab_bar = tarefas->status_tabs->tabBar();
        tabs_width = qMax(tabs_width, tab_bar->sizeHint().width());

        int tab_rects_width = 0;
        for (int i = 0; i < tab_bar->count(); ++i) {
            tab_rects_width += tab_bar->tabRect(i).width();
        }
        tabs_width = qMax(tabs_width, tab_rects_width);
    }

    int horizontal_margins = 24;
    if (tarefas->layout() != nullptr) {
        const QMargins margins = tarefas->layout()->contentsMargins();
        horizontal_margins += margins.left() + margins.right();
    }

    return qMax(320, tabs_width + horizontal_margins);
}

bool GerenciadorTempo::_aplicar_divisao_inicial_splitter()
{
    try {
        if (_layout_robusto()) {
            return false;
        }
        if (main_splitter == nullptr || left_widget == nullptr || tarefas == nullptr) {
            return false;
        }
        if (usuario_ajustou_splitter_) {
            return false;
        }

        _ajustar_largura_minima_coluna_esquerda();

        QList<int> sizes = main_splitter->sizes();
        int total_width = main_splitter->width() - qMax(0, main_splitter->handleWidth());
        if (total_width <= 0) {
            if (sizes.size() >= 2) {
                total_width = sizes.at(0) + sizes.at(1);
            }
        }
        if (total_width <= 0) {
            return false;
        }

        const int left_min_width =
            qMax(_largura_minima_coluna_esquerda_efetiva(), qMax(left_widget->minimumWidth(), left_widget->minimumSizeHint().width()));
        const int right_min_width = _largura_minima_coluna_direita_efetiva();

        int left_width = left_min_width;
        int right_width = qMax(right_min_width, total_width - left_width);

        if (right_width < right_min_width) {
            right_width = right_min_width;
            left_width = qMax(left_min_width, total_width - right_width);
        }
        if (total_width < left_min_width + right_min_width) {
            left_width = left_min_width;
            right_width = right_min_width;
        }

        {
            QSignalBlocker blocker(main_splitter);
            main_splitter->setSizes({left_width, right_width});
        }

        largura_painel_em_progresso_ = right_width;
        largura_inicial_painel_em_progresso_capturada_ = true;
        divisao_inicial_splitter_aplicada_ = true;
        return true;
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao aplicar divisão inicial do splitter: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao aplicar divisão inicial do splitter"), true);
    }

    return false;
}

void GerenciadorTempo::_ajustar_largura_minima_coluna_esquerda()
{
    try {
        if (left_widget == nullptr || main_splitter == nullptr) {
            return;
        }

        if (_layout_robusto()) {
            int min_tarefas = 540;
            if (tarefas != nullptr) {
                min_tarefas = qMax(min_tarefas, qMin(920, qMax(tarefas->minimumSizeHint().width(), tarefas->sizeHint().width())));
            }

            const QSize available = available_window_size(this);
            const int splitter_width = main_splitter->width() > 0
                                           ? main_splitter->width()
                                           : qMax(320, available.width() - _largura_minima_efetiva_painel_em_progresso() - 24);
            const int handle_width = qMax(0, main_splitter->handleWidth());
            const bool right_panel_visible =
                painel_em_progresso_expandido_ && col_doing_timer_group != nullptr && col_doing_timer_group->isVisible();
            const int right_min_width = right_panel_visible ? (splitter_width > 0 && splitter_width < 1120 ? 220 : 240) : 0;
            const int left_floor = 420;
            int left_min_width = qMax(left_floor, min_tarefas);

            if (right_panel_visible && col_doing_timer_group != nullptr) {
                if (col_doing_timer_group->minimumWidth() != right_min_width) {
                    col_doing_timer_group->setMinimumWidth(right_min_width);
                }
            }

            if (splitter_width > 0) {
                const int max_left_width = qMax(left_floor, splitter_width - right_min_width - handle_width);
                left_min_width = qMin(left_min_width, max_left_width);
            }

            if (left_widget->minimumWidth() != left_min_width) {
                left_widget->setMinimumWidth(left_min_width);
            }
            left_widget->updateGeometry();
            if (col_doing_timer_group != nullptr) {
                col_doing_timer_group->updateGeometry();
            }

            _ajustar_limites_splitter_principal();
            return;
        }

        const bool right_panel_visible = tarefas != nullptr && tarefas->isVisible();
        const int right_min_width = right_panel_visible ? _largura_minima_coluna_direita_efetiva() : 0;
        const int left_min_width = _largura_minima_coluna_esquerda_efetiva();

        if (right_panel_visible && tarefas != nullptr) {
            if (tarefas->minimumWidth() != right_min_width) {
                tarefas->setMinimumWidth(right_min_width);
            }
        }

        if (left_widget->minimumWidth() != left_min_width) {
            left_widget->setMinimumWidth(left_min_width);
        }
        left_widget->updateGeometry();
        if (tarefas != nullptr) {
            tarefas->updateGeometry();
        }

        _ajustar_limites_splitter_principal();
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao ajustar largura mínima da coluna esquerda: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao ajustar largura mínima da coluna esquerda"), true);
    }
}

void GerenciadorTempo::_ajustar_limites_splitter_principal()
{
    try {
        if (main_splitter == nullptr || left_widget == nullptr) {
            return;
        }

        if (_layout_robusto()) {
            const int handle_width = qMax(0, main_splitter->handleWidth());
            const int left_min_width = qMax(left_widget->minimumWidth(), left_widget->minimumSizeHint().width());
            const bool right_panel_visible =
                painel_em_progresso_expandido_ && col_doing_timer_group != nullptr && col_doing_timer_group->isVisible();
            const int right_min_width =
                right_panel_visible ? qMax(col_doing_timer_group->minimumWidth(), col_doing_timer_group->minimumSizeHint().width()) : 0;
            const int required_width = left_min_width + right_min_width + handle_width;
            const QSize available = available_window_size(this);
            const int max_window_width = qMax(320, available.width() - 24);
            const int max_splitter_width = qMax(left_min_width, max_window_width - _largura_minima_efetiva_painel_em_progresso());
            const int bounded_splitter_width = qMin(required_width, max_splitter_width);
            const int bounded_window_width =
                qMin(required_width + _largura_minima_efetiva_painel_em_progresso(), max_window_width);
            const int bounded_window_height = bounded_module_min_height(this);

            if (main_splitter->minimumWidth() != bounded_splitter_width) {
                main_splitter->setMinimumWidth(bounded_splitter_width);
            }
            if (minimumWidth() != bounded_window_width) {
                setMinimumWidth(bounded_window_width);
            }
            if (minimumHeight() != bounded_window_height) {
                setMinimumHeight(bounded_window_height);
            }

            const QList<int> current_sizes = main_splitter->sizes();
            if (current_sizes.size() < 2) {
                return;
            }

            const int total_width = current_sizes[0] + current_sizes[1];
            if (total_width <= 0) {
                return;
            }

            int left_width = current_sizes[0];
            int right_width = right_panel_visible ? current_sizes[1] : 0;
            if (right_panel_visible) {
                const int max_left_width = qMax(left_min_width, total_width - right_min_width);
                left_width = qBound(left_min_width, left_width, max_left_width);
                right_width = total_width - left_width;

                if (right_width < right_min_width) {
                    right_width = right_min_width;
                    left_width = qMax(left_min_width, total_width - right_width);
                }
            } else {
                left_width = total_width;
            }

            if (left_width != current_sizes[0] || right_width != current_sizes[1]) {
                QSignalBlocker blocker(main_splitter);
                main_splitter->setSizes({left_width, right_width});
            }
            return;
        }

        const int handle_width = qMax(0, main_splitter->handleWidth());
        const int left_min_width =
            qMax(_largura_minima_coluna_esquerda_efetiva(), qMax(left_widget->minimumWidth(), left_widget->minimumSizeHint().width()));
        const bool right_panel_visible = tarefas != nullptr && tarefas->isVisible();
        const int right_min_width = right_panel_visible ? _largura_minima_coluna_direita_efetiva() : 0;
        const int required_width = left_min_width + right_min_width + handle_width;
        const int bounded_splitter_width = required_width;
        const int bounded_window_width = required_width;
        const int bounded_window_height = bounded_module_min_height(this);

        if (main_splitter->minimumWidth() != bounded_splitter_width) {
            main_splitter->setMinimumWidth(bounded_splitter_width);
        }
        if (minimumWidth() != bounded_window_width) {
            setMinimumWidth(bounded_window_width);
        }
        if (minimumHeight() != bounded_window_height) {
            setMinimumHeight(bounded_window_height);
        }

        const QList<int> current_sizes = main_splitter->sizes();
        if (current_sizes.size() < 2) {
            return;
        }

        const int total_width = current_sizes[0] + current_sizes[1];
        if (total_width <= 0) {
            return;
        }

        int left_width = current_sizes[0];
        int right_width = right_panel_visible ? current_sizes[1] : 0;
        if (right_panel_visible) {
            const int max_left_width = qMax(left_min_width, total_width - right_min_width);
            left_width = qBound(left_min_width, left_width, max_left_width);
            right_width = total_width - left_width;

            if (right_width < right_min_width) {
                right_width = right_min_width;
                left_width = qMax(left_min_width, total_width - right_width);
            }
        } else {
            left_width = total_width;
        }

        if (left_width != current_sizes[0] || right_width != current_sizes[1]) {
            QSignalBlocker blocker(main_splitter);
            main_splitter->setSizes({left_width, right_width});
        }
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao ajustar limites do splitter principal: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao ajustar limites do splitter principal"), true);
    }
}

int GerenciadorTempo::_largura_minima_efetiva_painel_em_progresso() const
{
    return _layout_robusto() ? rotated_button_width(btn_toggle_painel_em_progresso_) : 0;
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTempoFns { void ajustar_largura_minima_coluna_esquerda(GerenciadorTempo &self) { self._ajustar_largura_minima_coluna_esquerda(); } }
