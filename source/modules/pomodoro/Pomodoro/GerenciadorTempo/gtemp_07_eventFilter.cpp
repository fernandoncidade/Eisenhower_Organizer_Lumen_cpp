#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_07_eventFilter.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTempoInternal;

bool GerenciadorTempo::eventFilter(QObject *obj, QEvent *event)
{
    try {
        if (event != nullptr) {
            switch (event->type()) {
            case QEvent::MouseButtonPress:
                if (col_doing_timer_list != nullptr
                    && (obj == col_doing_timer_list || obj == col_doing_timer_list->viewport() || obj == col_doing_timer_group)) {
                    auto *me = static_cast<QMouseEvent *>(event);
                    if (obj == col_doing_timer_list->viewport()) {
                        QListWidgetItem *item = col_doing_timer_list->itemAt(me->pos());
                        if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable)) {
                            limpar_selecao_tarefas();
                            emit task_counter_changed();
                        }
                    } else {
                        limpar_selecao_tarefas();
                        emit task_counter_changed();
                    }
                }
                break;
            case QEvent::ApplicationPaletteChange:
            case QEvent::PaletteChange:
            case QEvent::StyleChange:
            case QEvent::ThemeChange:
                _aplicar_tema_dinamico_inputs();
                _ajustar_limites_verticais();
                if (!ajustando_geometria_) {
                    QScopedValueRollback<bool> guard(ajustando_geometria_, true);
                    _ajustar_largura_minima_coluna_esquerda();
                    _ajustar_limites_splitter_principal();
                }
                break;
            case QEvent::LanguageChange:
                atualizar_traducoes();
                break;
            case QEvent::Resize:
                if ((obj == this || obj == parentWidget() || obj == window()) && !ajustando_geometria_) {
                    QScopedValueRollback<bool> guard(ajustando_geometria_, true);
                    _ajustar_limites_verticais();
                    _ajustar_largura_minima_coluna_esquerda();
                    if (_layout_robusto()) {
                        if (painel_em_progresso_expandido_ && largura_inicial_painel_em_progresso_capturada_) {
                            _sincronizar_splitter_painel_em_progresso(false);
                        }
                    } else if (!usuario_ajustou_splitter_) {
                        _aplicar_divisao_inicial_splitter();
                    } else if (largura_inicial_painel_em_progresso_capturada_) {
                        _sincronizar_splitter_painel_em_progresso(false);
                    }
                    _ajustar_limites_splitter_principal();
                }
                break;
            default:
                break;
            }
        }
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro no eventFilter do GerenciadorTempo: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro no eventFilter do GerenciadorTempo"), true);
    }

    return QWidget::eventFilter(obj, event);
}

QSize GerenciadorTempo::minimumSizeHint() const
{
    const QSize available = available_window_size(this);
    const int min_width = qMin(qMax(920, _largura_minima_efetiva_painel_em_progresso() + 760),
                               qMax(480, available.width() - 120));
    return QSize(min_width, bounded_module_min_height(this));
}

QString GerenciadorTempo::ui_layout_mode_key() const
{
    return ui_layout_mode_key_;
}

bool GerenciadorTempo::set_ui_layout_mode_key(const QString &mode_key)
{
    const QString normalized = normalized_ui_layout_key(mode_key);
    if (normalized == ui_layout_mode_key_) {
        return true;
    }

    const bool painel_expandido = painel_em_progresso_expandido_;
    ui_layout_mode_key_ = normalized;
    painel_em_progresso_expandido_ = painel_expandido;
    setup_ui();
    _aplicar_tema_dinamico_inputs();
    updateGeometry();
    update();
    emit task_counter_changed();
    return true;
}

bool GerenciadorTempo::is_in_progress_panel_expanded() const
{
    return painel_em_progresso_expandido_;
}

void GerenciadorTempo::set_in_progress_panel_expanded(bool expanded)
{
    if (painel_em_progresso_expandido_ == expanded) {
        return;
    }

    if (expanded) {
        _alternar_painel_em_progresso(true);
        return;
    }

    _alternar_painel_em_progresso();
}

void GerenciadorTempo::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    _ajustar_limites_verticais();
    if (!_layout_robusto() && !usuario_ajustou_splitter_ && !ajustando_geometria_) {
        QScopedValueRollback<bool> guard(ajustando_geometria_, true);
        _aplicar_divisao_inicial_splitter();
    }
}

void GerenciadorTempo::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (_layout_robusto()) {
        return;
    }
    if (usuario_ajustou_splitter_) {
        return;
    }

    QTimer::singleShot(0, this, [this]() {
        if (!usuario_ajustou_splitter_) {
            _aplicar_divisao_inicial_splitter();
        }
    });
    QTimer::singleShot(100, this, [this]() {
        if (!usuario_ajustou_splitter_) {
            _aplicar_divisao_inicial_splitter();
        }
    });
    QTimer::singleShot(300, this, [this]() {
        if (!usuario_ajustou_splitter_) {
            _aplicar_divisao_inicial_splitter();
        }
    });
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTempoFns { bool eventFilter(GerenciadorTempo &self, QObject *obj, QEvent *event) { return self.eventFilter(obj, event); } }
