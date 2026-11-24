#include "source/modules/feynman/mod_05_metodo_feynman.hpp"

#include <algorithm>

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QEvent>
#include <QFont>
#include <QFontMetrics>
#include <QGroupBox>
#include <QLabel>
#include <QList>
#include <QListWidget>
#include <QListWidgetItem>
#include <QResizeEvent>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QSplitter>
#include <QSplitterHandle>
#include <QTabBar>
#include <QTabWidget>
#include <QTextEdit>
#include <QThread>
#include <QTimer>

#include "source/modules/feynman/Feynman/fey_01_FeynmanWorker.hpp"
#include "source/modules/feynman/Feynman/fey_19_RotatedTabButton.hpp"
#include "source/modules/feynman/Feynman/feynman_metodo_helpers.hpp"
#include "source/utils/UnifiedTaskStore.hpp"

namespace source::plugin::Feynman {

MetodoFeynman::MetodoFeynman(QWidget *parent)
    : QWidget(parent),
      logger_(source::utils::LogManager::get_logger()),
      gerenciador_botoes_(this)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(0);
    arquivo_conceitos_ = source::utils::UnifiedTaskStore::shared_tasks_path();
    conceitos_ = normalized_concepts(FeynmanWorker::load_file(arquivo_conceitos_));

    setup_ui();

    try {
        feynman_thread_ = new QThread();
        feynman_worker_ = new FeynmanWorker();
        feynman_worker_->moveToThread(feynman_thread_);

        connect(feynman_worker_, &FeynmanWorker::conceitos_loaded, this, &MetodoFeynman::_on_conceitos_loaded);
        connect(feynman_worker_, &FeynmanWorker::save_finished, this, [this](bool ok) {
            const QString origin =
                pending_save_origins_.isEmpty() ? QStringLiteral("feynman") : pending_save_origins_.takeFirst();
            if (ok && compat::host_bridge() != nullptr) {
                compat::host_bridge()->notify_shared_state_changed(origin);
            }
            if (!ok && logger_ != nullptr) {
                logger_->warning(QStringLiteral("Worker do Método Feynman reportou falha ao salvar conceitos."));
            }
        });
        connect(feynman_worker_, &FeynmanWorker::error, this, [this](const QString &message) {
            if (logger_ != nullptr) {
                logger_->error(QStringLiteral("Worker error (Feynman): %1").arg(message));
            }
        });

        feynman_thread_->start();
        QTimer::singleShot(0, this, [this]() { carregar_conceitos(); });

        if (auto *app = QCoreApplication::instance()) {
            connect(app, &QCoreApplication::aboutToQuit, this, &MetodoFeynman::_stop_feynman_thread);
        }
    } catch (...) {
        if (logger_ != nullptr) {
            logger_->error(QStringLiteral("Erro ao iniciar thread do Método Feynman."), true);
        }
    }
}

MetodoFeynman::~MetodoFeynman()
{
    cleanup();
}

QString MetodoFeynman::ui_layout_mode_key() const
{
    return ui_layout_mode_key_;
}

bool MetodoFeynman::set_ui_layout_mode_key(const QString &mode_key)
{
    const QString normalized = normalized_ui_layout_key(mode_key);
    if (normalized == ui_layout_mode_key_) {
        return true;
    }

    QStringList selected_titles;
    if (lista_conceitos != nullptr) {
        const QList<QListWidgetItem *> selected_items = lista_conceitos->selectedItems();
        for (QListWidgetItem *item : selected_items) {
            const QVariantMap conceito = obter_conceito_do_item(item);
            const QString title = conceito.value(QStringLiteral("titulo")).toString().trimmed();
            if (!title.isEmpty()) {
                selected_titles.append(title);
            }
        }
    }

    const QVariantMap current_concept = conceito_atual_;
    const bool had_current_concept = has_conceito_atual_;
    const QString titulo = input_titulo != nullptr ? input_titulo->toPlainText() : QString();
    const QString explicacao = texto_explicacao != nullptr ? texto_explicacao->toPlainText() : QString();
    const QString lacunas = texto_lacunas != nullptr ? texto_lacunas->toPlainText() : QString();
    const QString revisao = texto_revisao != nullptr ? texto_revisao->toPlainText() : QString();
    const QString descricao = texto_descricao != nullptr ? texto_descricao->toPlainText() : QString();
    const QString descricao_gerada = descricao_gerada_atual_;
    const int prioridade = combo_prioridade != nullptr ? normalized_priority_value(combo_prioridade->currentData())
                                                       : kDefaultPriority;
    const int dominio = combo_dominio != nullptr ? normalized_domain_value(combo_dominio->currentData())
                                                 : kDefaultDomain;
    const bool descricao_editada = descricao_editada_manualmente_;
    const bool painel_expandido = painel_tarefas_expandido_;
    int largura_painel = largura_painel_tarefas_;
    if (painel_expandido) {
        int largura_visivel_painel = 0;
        if (splitter_principal_ != nullptr) {
            const QList<int> sizes = splitter_principal_->sizes();
            if (sizes.size() >= 2 && sizes.at(0) > 0) {
                largura_visivel_painel = sizes.at(0);
            }
        }
        if (largura_visivel_painel <= 0 && painel_esquerdo_ != nullptr && painel_esquerdo_->width() > 0) {
            largura_visivel_painel = painel_esquerdo_->width();
        }
        largura_painel = largura_visivel_painel > 0
                              ? std::max(largura_visivel_painel, painel_esquerdo_min_base_)
                              : std::max(largura_painel, painel_esquerdo_min_base_);
    }

    ui_layout_mode_key_ = normalized;
    painel_tarefas_expandido_ = painel_expandido;
    largura_painel_tarefas_ = largura_painel;
    setup_ui();

    conceito_atual_ = current_concept;
    has_conceito_atual_ = had_current_concept;

    sincronizando_descricao_ = true;
    if (input_titulo != nullptr) {
        input_titulo->setPlainText(titulo);
    }
    if (texto_explicacao != nullptr) {
        texto_explicacao->setPlainText(explicacao);
    }
    if (texto_lacunas != nullptr) {
        texto_lacunas->setPlainText(lacunas);
    }
    if (texto_revisao != nullptr) {
        texto_revisao->setPlainText(revisao);
    }
    if (texto_descricao != nullptr) {
        texto_descricao->setPlainText(descricao);
    }
    sincronizando_descricao_ = false;
    descricao_gerada_atual_ = descricao_gerada;
    descricao_editada_manualmente_ = descricao_editada;

    if (combo_prioridade != nullptr) {
        const int index = combo_prioridade->findData(prioridade);
        combo_prioridade->setCurrentIndex(index >= 0 ? index : 0);
    }
    if (combo_dominio != nullptr) {
        const int index = combo_dominio->findData(dominio);
        combo_dominio->setCurrentIndex(index >= 0 ? index : 0);
    }

    int current_row = -1;
    if (lista_conceitos != nullptr) {
        const QString current_title =
            had_current_concept ? current_concept.value(QStringLiteral("titulo")).toString().trimmed() : QString();
        for (int i = 0; i < lista_conceitos->count(); ++i) {
            QListWidgetItem *item = lista_conceitos->item(i);
            const QVariantMap conceito = obter_conceito_do_item(item);
            const QString title = conceito.value(QStringLiteral("titulo")).toString().trimmed();
            if (!title.isEmpty() && selected_titles.contains(title, Qt::CaseInsensitive)) {
                item->setSelected(true);
            }
            if (!current_title.isEmpty() && title.compare(current_title, Qt::CaseInsensitive) == 0) {
                current_row = i;
            }
        }
        if (current_row >= 0) {
            lista_conceitos->setCurrentRow(current_row);
        }
    }

    atualizar_aba_descricao();
    atualizar_largura_minima_painel_esquerdo();
    atualizar_largura_minima_painel_direito();
    atualizar_dimensoes_minimas_iniciais();
    sincronizar_splitter_painel_tarefas(false);
    atualizar_placeholders_dinamicos();

    updateGeometry();
    update();
    emit task_counter_changed();
    return true;
}

void MetodoFeynman::atualizar_aba_descricao()
{
    if (texto_descricao == nullptr || sincronizando_descricao_) {
        return;
    }

    const QString descricao_gerada = composed_description(this,
                                                          input_titulo != nullptr ? input_titulo->toPlainText() : QString(),
                                                          texto_explicacao != nullptr ? texto_explicacao->toPlainText() : QString(),
                                                          texto_lacunas != nullptr ? texto_lacunas->toPlainText() : QString(),
                                                          texto_revisao != nullptr ? texto_revisao->toPlainText() : QString());
    QString descricao;
    if (has_conceito_atual_ && has_external_description_base(conceito_atual_)) {
        QString descricao_base = manual_description_text(texto_descricao->toPlainText(), descricao_gerada_atual_);
        if (descricao_base.trimmed().isEmpty()) {
            descricao_base = conceito_atual_.value(QStringLiteral("descricao")).toString();
        }
        descricao = description_with_external_base(descricao_base, descricao_gerada).trimmed();
    } else {
        descricao = merge_description_edit(descricao_gerada_atual_, texto_descricao->toPlainText(), descricao_gerada).trimmed();
    }
    if (texto_descricao->toPlainText() == descricao && descricao_gerada_atual_ == descricao_gerada) {
        return;
    }

    sincronizando_descricao_ = true;
    texto_descricao->setPlainText(descricao);
    descricao_gerada_atual_ = descricao_gerada;
    descricao_editada_manualmente_ = descricao != descricao_gerada.trimmed();
    sincronizando_descricao_ = false;
}

void MetodoFeynman::limpar_selecao_tarefas()
{
    if (lista_conceitos != nullptr) {
        lista_conceitos->clearSelection();
        if (QItemSelectionModel *sm = lista_conceitos->selectionModel(); sm != nullptr) {
            sm->clear();
        }
    }
}

bool MetodoFeynman::eventFilter(QObject *obj, QEvent *event)
{
    if (event != nullptr) {
        const QEvent::Type type = event->type();
        if (type == QEvent::MouseButtonPress) {
            if (lista_conceitos != nullptr
                && (obj == lista_conceitos || obj == lista_conceitos->viewport() || obj == frame_meus_conceitos)) {
                auto *me = static_cast<QMouseEvent *>(event);
                if (obj == lista_conceitos->viewport()) {
                    QListWidgetItem *item = lista_conceitos->itemAt(me->pos());
                    if (item == nullptr || !(item->flags() & Qt::ItemIsSelectable)) {
                        limpar_selecao_tarefas();
                        emit task_counter_changed();
                    }
                } else {
                    limpar_selecao_tarefas();
                    emit task_counter_changed();
                }
            }
        }
        const bool is_theme_event =
            type == QEvent::ApplicationPaletteChange || type == QEvent::PaletteChange || type == QEvent::StyleChange
            || type == QEvent::ThemeChange;
        const bool is_placeholder_event = type == QEvent::Resize || type == QEvent::Show
            || type == QEvent::LayoutRequest || type == QEvent::FontChange;
        const bool watches_object = obj == QApplication::instance()
            || (lista_conceitos != nullptr
                && (obj == lista_conceitos || obj == lista_conceitos->viewport() || obj == frame_meus_conceitos));
        const bool watches_placeholder =
            obj == input_titulo || obj == texto_explicacao || obj == texto_lacunas || obj == texto_revisao
            || obj == texto_descricao
            || (input_titulo != nullptr && obj == input_titulo->viewport())
            || (texto_explicacao != nullptr && obj == texto_explicacao->viewport())
            || (texto_lacunas != nullptr && obj == texto_lacunas->viewport())
            || (texto_revisao != nullptr && obj == texto_revisao->viewport())
            || (texto_descricao != nullptr && obj == texto_descricao->viewport());
        if (watches_object && is_theme_event) {
            sync_backgrounds(*this);
            QTimer::singleShot(0, this, [this]() { sync_backgrounds(*this); });
            QTimer::singleShot(80, this, [this]() { sync_backgrounds(*this); });
        }
        if (obj == QApplication::instance() && type == QEvent::ApplicationFontChange) {
            atualizar_placeholders_dinamicos();
        }
        if (watches_placeholder && is_placeholder_event) {
            atualizar_placeholders_dinamicos();
        }
    }

    return QWidget::eventFilter(obj, event);
}

bool MetodoFeynman::is_task_panel_expanded() const
{
    return painel_tarefas_expandido_;
}

void MetodoFeynman::set_task_panel_expanded(bool expanded)
{
    if (painel_tarefas_expandido_ == expanded) {
        if (painel_esquerdo_ != nullptr) {
            painel_esquerdo_->setVisible(expanded);
        }
        atualizar_interacao_splitter_painel_tarefas();
        if (btn_toggle_painel_tarefas_ != nullptr) {
            btn_toggle_painel_tarefas_->setText(expanded ? collapse_task_panel_text(this) : expand_task_panel_text(this));
        }
        atualizar_largura_minima_painel_esquerdo();
        atualizar_largura_minima_painel_direito();
        atualizar_dimensoes_minimas_iniciais();
        updateGeometry();
        sincronizar_splitter_painel_tarefas(false);
        atualizar_placeholders_dinamicos();
        return;
    }

    if (expanded) {
        alternar_painel_tarefas(true);
        return;
    }

    alternar_painel_tarefas();
}

QSize MetodoFeynman::minimumSizeHint() const
{
    const QSize available = available_window_size(this);
    const int min_width = std::min(std::max(920, largura_minima_efetiva_painel_tarefas() + 760),
                                   std::max(480, available.width() - 120));
    const int min_height = std::min(520, std::max(360, available.height() - 260));
    return QSize(min_width, min_height);
}

void MetodoFeynman::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    aplicar_larguras_minimas_paineis();
    atualizar_dimensoes_minimas_iniciais();
    sincronizar_splitter_painel_tarefas();
    atualizar_placeholders_dinamicos();
}

void MetodoFeynman::ajustar_largura_combo_dominio()
{
    if (combo_dominio == nullptr) {
        return;
    }

    QFontMetrics metrics(combo_dominio->font());
    int max_width = 0;
    for (int i = 0; i < combo_dominio->count(); ++i) {
        max_width = std::max(max_width, metrics.horizontalAdvance(combo_dominio->itemText(i)));
    }

    combo_dominio->setFixedWidth(max_width + 48);
}

void MetodoFeynman::ajustar_largura_combo_prioridade()
{
    if (combo_prioridade == nullptr) {
        return;
    }

    QFontMetrics metrics(combo_prioridade->font());
    int max_width = 0;
    for (int i = 0; i < combo_prioridade->count(); ++i) {
        max_width = std::max(max_width, metrics.horizontalAdvance(combo_prioridade->itemText(i)));
    }

    combo_prioridade->setFixedWidth(max_width + 48);
}

void MetodoFeynman::ajustar_altura_combos_footer()
{
    if ((combo_dominio == nullptr && combo_prioridade == nullptr) || btn_salvar == nullptr) {
        return;
    }

    const int button_height = std::max(btn_salvar->sizeHint().height(), btn_salvar->minimumSizeHint().height());
    const int combo_height = std::max(button_height, 24);
    if (combo_prioridade != nullptr) {
        combo_prioridade->setFixedHeight(combo_height);
    }
    if (combo_dominio != nullptr) {
        combo_dominio->setFixedHeight(combo_height);
    }
}

void MetodoFeynman::atualizar_largura_minima_painel_esquerdo()
{
    if (painel_esquerdo_ == nullptr) {
        return;
    }

    int title_width = 0;
    if (frame_meus_conceitos != nullptr) {
        title_width = QFontMetrics(frame_meus_conceitos->font()).horizontalAdvance(frame_meus_conceitos->title()) + 44;
    } else if (label_meus_conceitos != nullptr) {
        title_width = label_meus_conceitos->sizeHint().width() + 28;
    }

    const int min_width = std::max(300, title_width);

    painel_esquerdo_min_base_ = min_width;
    aplicar_larguras_minimas_paineis();
}

void MetodoFeynman::atualizar_largura_minima_painel_direito()
{
    if (painel_direito_ == nullptr || combo_dominio == nullptr || combo_prioridade == nullptr || label_nivel == nullptr
        || label_prioridade == nullptr || footer_layout_ == nullptr) {
        return;
    }

    painel_direito_min_base_ = std::max(0, largura_minima_abas());
    aplicar_larguras_minimas_paineis();
}

void MetodoFeynman::aplicar_larguras_minimas_paineis()
{
    if (painel_esquerdo_ == nullptr && painel_direito_ == nullptr) {
        return;
    }

    int total_width = width();
    if (splitter_principal_ != nullptr) {
        const int handle_width = std::max(0, splitter_principal_->handleWidth());
        total_width = std::max(0, splitter_principal_->size().width() - handle_width);
    }

    const PanelMinimums mins = effective_panel_minimums(total_width,
                                                        painel_esquerdo_min_base_,
                                                        painel_direito_min_base_,
                                                        painel_tarefas_expandido_);
    if (painel_esquerdo_ != nullptr) {
        painel_esquerdo_->setMinimumWidth(mins.left);
    }
    if (painel_direito_ != nullptr) {
        painel_direito_->setMinimumWidth(mins.right);
    }
}

int MetodoFeynman::largura_minima_abas() const
{
    QStringList textos;
    if (tab_widget != nullptr && tab_widget->count() > 0) {
        for (int i = 0; i < tab_widget->count(); ++i) {
            const QString text = tab_widget->tabText(i).trimmed();
            if (!text.isEmpty()) {
                textos.append(text);
            }
        }
    }

    if (textos.isEmpty()) {
        textos = {
            compat::tr(this, QStringLiteral("1️⃣ Conceito"), QStringLiteral("1️⃣ Concept")),
            compat::tr(this, QStringLiteral("2️⃣ Explique"), QStringLiteral("2️⃣ Explain")),
            compat::tr(this, QStringLiteral("3️⃣ Lacunas"), QStringLiteral("3️⃣ Gaps")),
            compat::tr(this, QStringLiteral("4️⃣ Revisão"), QStringLiteral("4️⃣ Review")),
            compat::tr(this, QStringLiteral("5️⃣ Descrição"), QStringLiteral("5️⃣ Description")),
        };
    }

    QFont font = this->font();
    if (tab_widget != nullptr && tab_widget->tabBar() != nullptr) {
        font = tab_widget->tabBar()->font();
    } else if (painel_direito_ != nullptr) {
        font = painel_direito_->font();
    }
    QFontMetrics metrics(font);

    int total = 0;
    for (const QString &text : textos) {
        total += metrics.horizontalAdvance(text);
    }
    total += textos.size() * kFeynmanTabTextPadding;
    return total;
}

void MetodoFeynman::atualizar_dimensoes_minimas_iniciais()
{
    if (splitter_principal_ == nullptr || painel_direito_ == nullptr) {
        return;
    }

    aplicar_larguras_minimas_paineis();

    const int left_width = painel_tarefas_expandido_ && painel_esquerdo_ != nullptr
                               ? std::max(painel_esquerdo_->minimumWidth(), painel_esquerdo_->minimumSizeHint().width())
                               : 0;
    const int right_width = std::max(painel_direito_->minimumWidth(), painel_direito_min_base_);
    const QSize available = available_window_size(this);
    const int required_width =
        left_width + right_width + std::max(0, splitter_principal_->handleWidth()) + largura_minima_efetiva_painel_tarefas() + 24;
    const int min_width = std::min(std::max(1260, required_width), std::max(320, available.width() - 24));
    const int min_height = std::min(520, std::max(360, available.height() - 260));
    setMinimumSize(min_width, min_height);
}

int MetodoFeynman::largura_minima_efetiva_painel_tarefas() const
{
    return rotated_button_width(btn_toggle_painel_tarefas_);
}

void MetodoFeynman::atualizar_interacao_splitter_painel_tarefas()
{
    if (splitter_principal_ == nullptr) {
        return;
    }

    splitter_principal_->setHandleWidth(
        painel_tarefas_expandido_ ? kFeynmanSplitterExpandedHandleWidth : kFeynmanSplitterCollapsedHandleWidth);
    if (QSplitterHandle *handle = splitter_principal_->handle(1)) {
        handle->setEnabled(painel_tarefas_expandido_);
    }
}

void MetodoFeynman::sincronizar_splitter_painel_tarefas(bool preservar_atual)
{
    if (splitter_principal_ == nullptr || painel_direito_ == nullptr) {
        return;
    }

    const QList<int> sizes = splitter_principal_->sizes();
    if (sizes.size() < 2) {
        return;
    }

    const int total_width = std::max(0, sizes.at(0) + sizes.at(1));
    if (total_width <= 0) {
        return;
    }

    const PanelMinimums mins = effective_panel_minimums(total_width,
                                                        painel_esquerdo_min_base_,
                                                        painel_direito_min_base_,
                                                        painel_tarefas_expandido_);
    const int right_min_width = std::max(mins.right, painel_direito_->minimumWidth());
    int target_left = 0;

    if (painel_tarefas_expandido_ && painel_esquerdo_ != nullptr) {
        const int left_min_width = std::max(mins.left, painel_esquerdo_->minimumSizeHint().width());
        target_left = preservar_atual && sizes.at(0) > 0 ? std::max(sizes.at(0), left_min_width)
                                                          : std::max(largura_painel_tarefas_, left_min_width);
        const int max_left_width = std::max(left_min_width, total_width - right_min_width);
        target_left = std::min(target_left, max_left_width);
    }

    int target_right = std::max(right_min_width, total_width - target_left);
    if (target_left + target_right != total_width) {
        target_right = std::max(right_min_width, total_width - target_left);
        target_left = std::max(0, total_width - target_right);
    }

    QSignalBlocker blocker(splitter_principal_);
    splitter_principal_->setSizes({target_left, target_right});
}

void MetodoFeynman::alternar_painel_tarefas(bool abrir_se_oculto)
{
    if (painel_esquerdo_ == nullptr || splitter_principal_ == nullptr) {
        return;
    }

    if (!painel_tarefas_expandido_ && abrir_se_oculto) {
        painel_tarefas_expandido_ = true;
    } else {
        if (painel_tarefas_expandido_) {
            largura_painel_tarefas_ =
                std::max(painel_esquerdo_->width(), painel_esquerdo_min_base_);
        }
        painel_tarefas_expandido_ = !painel_tarefas_expandido_;
    }

    painel_esquerdo_->setVisible(painel_tarefas_expandido_);
    atualizar_interacao_splitter_painel_tarefas();

    if (btn_toggle_painel_tarefas_ != nullptr) {
        btn_toggle_painel_tarefas_->setText(
            painel_tarefas_expandido_ ? collapse_task_panel_text(this) : expand_task_panel_text(this));
    }

    atualizar_largura_minima_painel_esquerdo();
    atualizar_largura_minima_painel_direito();
    atualizar_dimensoes_minimas_iniciais();
    updateGeometry();
    sincronizar_splitter_painel_tarefas(false);
    atualizar_placeholders_dinamicos();
}

void MetodoFeynman::on_task_duration_changed(const QString &task_id, double duration_days)
{
    const QString target_id = task_id.trimmed();
    if (target_id.isEmpty()) {
        return;
    }

    bool changed = false;
    for (int i = 0; i < conceitos_.size(); ++i) {
        QVariantMap conceito = conceitos_.at(i).toMap();
        const QString cid = conceito.value(QStringLiteral("id")).toString().trimmed();
        const QString c_int_id = conceito.value(QStringLiteral("integracao_task_id")).toString().trimmed();
        if (cid == target_id || c_int_id == target_id) {
            conceito.insert(QStringLiteral("duracao"), duration_days);
            conceito.insert(QStringLiteral("duration"), duration_days);
            conceitos_[i] = normalized_concept(conceito);
            changed = true;
            if (has_conceito_atual_
                && (conceito_atual_.value(QStringLiteral("id")).toString().trimmed() == target_id
                    || conceito_atual_.value(QStringLiteral("integracao_task_id")).toString().trimmed() == target_id
                    || conceito_atual_.value(QStringLiteral("titulo")).toString().compare(
                           conceito.value(QStringLiteral("titulo")).toString(), Qt::CaseInsensitive) == 0)) {
                conceito_atual_ = conceitos_.at(i).toMap();
            }
        }
    }

    if (changed) {
        atualizar_lista();
    }
}

}  // namespace source::plugin::Feynman
