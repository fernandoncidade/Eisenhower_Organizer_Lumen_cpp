#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_14_interacoes_coluna_doing_timer.hpp"

#include "source/modules/pomodoro/Pomodoro/GerenciadorTempo/gtemp_internal.hpp"

namespace source::plugin::Pomodoro {
using namespace GerenciadorTempoInternal;

bool GerenciadorTempo::_capturar_largura_inicial_painel_em_progresso()
{
    if (largura_inicial_painel_em_progresso_capturada_) {
        return true;
    }

    if (_layout_robusto()) {
        if (!painel_em_progresso_expandido_ || main_splitter == nullptr || col_doing_timer_group == nullptr
            || !col_doing_timer_group->isVisible()) {
            return false;
        }

        const QList<int> sizes = main_splitter->sizes();
        if (sizes.size() < 2 || sizes[1] <= 0) {
            return false;
        }

        const int right_min_width = qMax(col_doing_timer_group->minimumWidth(), col_doing_timer_group->minimumSizeHint().width());
        largura_painel_em_progresso_ = qMax(right_min_width, sizes[1]);
        largura_inicial_painel_em_progresso_capturada_ = true;
        return true;
    }

    if (main_splitter == nullptr || tarefas == nullptr || !tarefas->isVisible()) {
        return false;
    }

    const QList<int> sizes = main_splitter->sizes();
    if (sizes.size() < 2 || sizes[1] <= 0) {
        return false;
    }

    const int right_min_width = _largura_minima_coluna_direita_efetiva();
    largura_painel_em_progresso_ = qMax(right_min_width, sizes[1]);
    largura_inicial_painel_em_progresso_capturada_ = true;
    return true;
}

void GerenciadorTempo::_atualizar_interacao_painel_em_progresso()
{
    if (main_splitter == nullptr) {
        return;
    }

    if (_layout_robusto()) {
        main_splitter->setHandleWidth(
            painel_em_progresso_expandido_ ? kTempoSplitterExpandedHandleWidth : kTempoSplitterCollapsedHandleWidth);
        if (QSplitterHandle *handle = main_splitter->handle(1)) {
            handle->setEnabled(painel_em_progresso_expandido_);
        }
        return;
    }

    main_splitter->setHandleWidth(kTempoSplitterExpandedHandleWidth);
    if (QSplitterHandle *handle = main_splitter->handle(1)) {
        handle->setEnabled(true);
    }
}

void GerenciadorTempo::_sincronizar_splitter_painel_em_progresso(bool preserve_current)
{
    if (main_splitter == nullptr || left_widget == nullptr) {
        return;
    }

    const QList<int> sizes = main_splitter->sizes();
    if (sizes.size() < 2) {
        return;
    }

    const int total_width = sizes[0] + sizes[1];
    if (total_width <= 0) {
        return;
    }

    const int left_min_width = _layout_robusto()
                                   ? qMax(left_widget->minimumWidth(), left_widget->minimumSizeHint().width())
                                   : qMax(_largura_minima_coluna_esquerda_efetiva(),
                                          qMax(left_widget->minimumWidth(), left_widget->minimumSizeHint().width()));
    int target_right = 0;

    if (_layout_robusto() && painel_em_progresso_expandido_ && col_doing_timer_group != nullptr) {
        const int right_min_width = qMax(col_doing_timer_group->minimumWidth(), col_doing_timer_group->minimumSizeHint().width());
        target_right = preserve_current && sizes[1] > 0 ? qMax(sizes[1], right_min_width)
                                                        : qMax(largura_painel_em_progresso_, right_min_width);
        const int max_right_width = qMax(right_min_width, total_width - left_min_width);
        target_right = qMin(target_right, max_right_width);
    } else if (!_layout_robusto() && tarefas != nullptr && tarefas->isVisible()) {
        const int right_min_width = _largura_minima_coluna_direita_efetiva();
        target_right = preserve_current && sizes[1] > 0 ? qMax(sizes[1], right_min_width)
                                                        : qMax(largura_painel_em_progresso_, right_min_width);
        const int max_right_width = qMax(right_min_width, total_width - left_min_width);
        target_right = qMin(target_right, max_right_width);
    }

    int target_left = qMax(left_min_width, total_width - target_right);
    if (target_left + target_right != total_width) {
        target_right = qMax(0, total_width - target_left);
        target_left = qMax(left_min_width, total_width - target_right);
    }

    QSignalBlocker blocker(main_splitter);
    main_splitter->setSizes({target_left, target_right});
}

void GerenciadorTempo::_registrar_largura_atual_painel_em_progresso()
{
    if (_layout_robusto()) {
        if (!painel_em_progresso_expandido_ || main_splitter == nullptr || col_doing_timer_group == nullptr
            || !col_doing_timer_group->isVisible()) {
            return;
        }

        const QList<int> sizes = main_splitter->sizes();
        if (sizes.size() < 2 || sizes[1] <= 0) {
            return;
        }

        const int right_min_width = qMax(col_doing_timer_group->minimumWidth(), col_doing_timer_group->minimumSizeHint().width());
        largura_painel_em_progresso_ = qMax(right_min_width, sizes[1]);
        largura_inicial_painel_em_progresso_capturada_ = true;
        return;
    }

    if (main_splitter == nullptr || tarefas == nullptr || !tarefas->isVisible()) {
        return;
    }

    const QList<int> sizes = main_splitter->sizes();
    if (sizes.size() < 2 || sizes[1] <= 0) {
        return;
    }

    const int right_min_width = _largura_minima_coluna_direita_efetiva();
    largura_painel_em_progresso_ = qMax(right_min_width, sizes[1]);
    largura_inicial_painel_em_progresso_capturada_ = true;
}

void GerenciadorTempo::_estabilizar_largura_painel_em_progresso()
{
    if (main_splitter == nullptr) {
        return;
    }

    QScopedValueRollback<bool> guard(ajustando_geometria_, true);
    _ajustar_largura_minima_coluna_esquerda();

    if (_layout_robusto()) {
        if (painel_em_progresso_expandido_) {
            if (largura_inicial_painel_em_progresso_capturada_) {
                _sincronizar_splitter_painel_em_progresso(false);
            } else {
                _capturar_largura_inicial_painel_em_progresso();
            }
        } else {
            _sincronizar_splitter_painel_em_progresso(false);
        }
    } else {
        if (largura_inicial_painel_em_progresso_capturada_) {
            _sincronizar_splitter_painel_em_progresso(false);
        } else {
            _capturar_largura_inicial_painel_em_progresso();
        }
    }

    _ajustar_limites_splitter_principal();
}

void GerenciadorTempo::_alternar_painel_em_progresso(bool abrir_se_oculto)
{
    if (_layout_robusto()) {
        try {
            if (col_doing_timer_group == nullptr || main_splitter == nullptr) {
                return;
            }

            if (!painel_em_progresso_expandido_ && abrir_se_oculto) {
                painel_em_progresso_expandido_ = true;
            } else {
                if (painel_em_progresso_expandido_) {
                    _registrar_largura_atual_painel_em_progresso();
                }
                painel_em_progresso_expandido_ = !painel_em_progresso_expandido_;
            }

            col_doing_timer_group->setVisible(painel_em_progresso_expandido_);
            _atualizar_interacao_painel_em_progresso();

            if (btn_toggle_painel_em_progresso_ != nullptr) {
                btn_toggle_painel_em_progresso_->setText(
                    painel_em_progresso_expandido_ ? collapse_in_progress_panel_text(this) : expand_in_progress_panel_text(this));
            }

            _ajustar_largura_minima_coluna_esquerda();
            updateGeometry();
            _sincronizar_splitter_painel_em_progresso(false);
            _ajustar_limites_splitter_principal();
            if (painel_em_progresso_expandido_) {
                QTimer::singleShot(0, this, [this]() { _estabilizar_largura_painel_em_progresso(); });
                QTimer::singleShot(50, this, [this]() { _estabilizar_largura_painel_em_progresso(); });
            }
        } catch (const std::exception &e) {
            logger_->error(QStringLiteral("Erro ao alternar painel Em Progresso: %1").arg(QString::fromUtf8(e.what())), true);
        } catch (...) {
            logger_->error(QStringLiteral("Erro ao alternar painel Em Progresso"), true);
        }
        return;
    }

    try {
        if (!painel_em_progresso_expandido_ && abrir_se_oculto) {
            painel_em_progresso_expandido_ = true;
        } else {
            painel_em_progresso_expandido_ = !painel_em_progresso_expandido_;
        }

        if (col_doing_timer_group == nullptr) {
            return;
        }

        col_doing_timer_group->setVisible(painel_em_progresso_expandido_);
            if (QWidget *spacer = findChild<QWidget *>(QStringLiteral("simple_layout_spacer"))) {
                spacer->setVisible(!painel_em_progresso_expandido_);
            }
        if (btn_toggle_painel_em_progresso_ != nullptr) {
            btn_toggle_painel_em_progresso_->setText(
                painel_em_progresso_expandido_ ? collapse_in_progress_panel_text(this) : expand_in_progress_panel_text(this));
        }

        _ajustar_limites_verticais();
        _ajustar_largura_minima_coluna_esquerda();
        _ajustar_limites_splitter_principal();
        updateGeometry();
        update();
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao alternar painel Em Progresso: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao alternar painel Em Progresso"), true);
    }
}

void GerenciadorTempo::_abrir_arquivo_vinculado_coluna_doing_timer(QListWidgetItem *item)
{
    try {
        if (item == nullptr || tarefas == nullptr) {
            return;
        }

        const QString key = item_key(item);
        for (Tarefa &task : tarefas->tarefas) {
            const QString task_key = !task.id.isEmpty()
                ? QStringLiteral("id:%1").arg(task.id)
                : QStringLiteral("titulo:%1").arg(task.titulo.trimmed());
            if (task_key == key && !task.file_path.isEmpty()) {
                if (QFileInfo::exists(task.file_path)) {
                    compat::open_local_path(task.file_path, this);
                    return;
                }

                switch (compat::prompt_missing_linked_file(this)) {
                case compat::MissingLinkedFileChoice::RemoveLink:
                    task.file_path.clear();
                    tarefas->salvar_tarefas();
                    tarefas->atualizar_listas();
                    atualizar_coluna_doing_timer();
                    compat::show_link_removed_success(this);
                    return;
                case compat::MissingLinkedFileChoice::Search: {
                    const QString novo_caminho = compat::browse_linked_file_replacement(this, task.file_path).trimmed();
                    if (!novo_caminho.isEmpty()) {
                        task.file_path = QFileInfo(novo_caminho).absoluteFilePath();
                        tarefas->salvar_tarefas();
                        tarefas->atualizar_listas();
                        atualizar_coluna_doing_timer();
                    }
                    return;
                }
                case compat::MissingLinkedFileChoice::None:
                    return;
                }
            }
        }
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro no duplo clique da coluna lateral: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro no duplo clique da coluna lateral"), true);
    }
}

void GerenciadorTempo::_sincronizar_estado_coluna_doing_timer()
{
    try {
        if (col_doing_timer_list == nullptr || tarefas == nullptr || tarefas->col_doing == nullptr || tarefas->col_doing->lista == nullptr) {
            return;
        }

        QMap<QString, bool> states;
        for (int i = 0; i < col_doing_timer_list->count(); ++i) {
            if (auto *item = col_doing_timer_list->item(i)) {
                states.insert(item_key(item), item->isSelected());
            }
        }

        QSignalBlocker blocker(tarefas->col_doing->lista);
        for (int i = 0; i < tarefas->col_doing->lista->count(); ++i) {
            if (auto *item = tarefas->col_doing->lista->item(i)) {
                item->setSelected(states.value(item_key(item), false));
            }
        }
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao sincronizar seleção da coluna lateral: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao sincronizar seleção da coluna lateral"), true);
    }
}

void GerenciadorTempo::_mostrar_menu_contexto_coluna_doing_timer(const QPoint &pos)
{
    try {
        if (col_doing_timer_list == nullptr) {
            return;
        }

        if (auto *item = col_doing_timer_list->itemAt(pos);
            item != nullptr && !is_group_header(item) && (item->flags() & Qt::ItemIsSelectable)) {
            Q_UNUSED(item);
            _sincronizar_estado_coluna_doing_timer();
            if (tarefas != nullptr) {
                tarefas->mostrar_menu_contexto(pos, col_doing_timer_list);
            }
        }
    } catch (const std::exception &e) {
        logger_->error(QStringLiteral("Erro ao abrir menu de contexto da coluna lateral: %1").arg(QString::fromUtf8(e.what())), true);
    } catch (...) {
        logger_->error(QStringLiteral("Erro ao abrir menu de contexto da coluna lateral"), true);
    }
}

}  // namespace source::plugin::Pomodoro

namespace source::plugin::Pomodoro::GerenciadorTempoFns {
void abrir_arquivo_vinculado_coluna_doing_timer(GerenciadorTempo &self, QListWidgetItem *item) { self._abrir_arquivo_vinculado_coluna_doing_timer(item); }
void sincronizar_estado_coluna_doing_timer(GerenciadorTempo &self) { self._sincronizar_estado_coluna_doing_timer(); }
void mostrar_menu_contexto_coluna_doing_timer(GerenciadorTempo &self, const QPoint &pos) { self._mostrar_menu_contexto_coluna_doing_timer(pos); }
}
