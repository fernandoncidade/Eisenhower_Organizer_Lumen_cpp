#include "source/modules/mapamental/metodoMapaMental/mp_03_MapaMental.hpp"

#include <QAbstractButton>
#include <QApplication>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>

#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_11_setup_ui.hpp"
#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_12_atualizar_traducoes.hpp"
#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_18_carregar_mapa.hpp"
#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_21_atualizar_tema.hpp"
#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_36_backup_runtime.hpp"
#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_38_modo_layout.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_01_NoConceito.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_02_LinhaConexao.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_04_ProcessadorIA.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_07_SmoothGraphicsView.hpp"

namespace source::plugin::MapaMental {

MapaMental::MapaMental(QWidget *parent) : QWidget(parent), processador_ia_(new ProcessadorIA)
{
    setup_ui();
    if (qApp != nullptr) {
        qApp->installEventFilter(this);
    }
    carregar_preferencia_layout();
    atualizar_tema();
    carregar_backup_automatico();

    backup_timer_ = new QTimer(this);
    backup_timer_->setInterval(8000);
    connect(backup_timer_, &QTimer::timeout, this, &MapaMental::salvar_backup_automatico);
    backup_timer_->start();
}

MapaMental::~MapaMental()
{
    if (qApp != nullptr) {
        qApp->removeEventFilter(this);
    }

    if (cinematic_timer_ != nullptr) {
        cinematic_timer_->stop();
    }
    if (backup_timer_ != nullptr) {
        backup_timer_->stop();
    }

    salvar_backup_automatico();

    for (auto it = no_animation_guards_.begin(); it != no_animation_guards_.end(); ++it) {
        if (it.value()) {
            *it.value() = false;
        }
    }
    no_animation_guards_.clear();
    for (auto it = conexao_animation_guards_.begin(); it != conexao_animation_guards_.end(); ++it) {
        if (it.value()) {
            *it.value() = false;
        }
    }
    conexao_animation_guards_.clear();

    for (LinhaConexao *linha : std::as_const(conexoes_removidas_)) {
        delete linha;
    }
    conexoes_removidas_.clear();
    for (NoConceito *no : std::as_const(nos_removidos_)) {
        delete no;
    }
    nos_removidos_.clear();

    delete processador_ia_;
}

void MapaMental::traduzir_botoes_padrao(QObject *dialogo) const
{
    if (dialogo == nullptr) {
        return;
    }

    struct StandardButtonText {
        int button;
        const char *text;
    };

    if (auto *message_box = qobject_cast<QMessageBox *>(dialogo)) {
        const StandardButtonText buttons[] = {
            {QMessageBox::Ok, QT_TRANSLATE_NOOP("MapaMentalPlugin", "OK")},
            {QMessageBox::Cancel, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Cancelar")},
            {QMessageBox::Yes, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Sim")},
            {QMessageBox::No, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Nao")},
            {QMessageBox::Abort, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Abortar")},
            {QMessageBox::Retry, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Tentar Novamente")},
            {QMessageBox::Ignore, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Ignorar")},
            {QMessageBox::Close, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Fechar")},
            {QMessageBox::Help, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Ajuda")},
            {QMessageBox::Apply, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Aplicar")},
            {QMessageBox::Reset, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Redefinir")},
            {QMessageBox::RestoreDefaults, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Restaurar Padroes")},
            {QMessageBox::Save, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Salvar")},
            {QMessageBox::SaveAll, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Salvar Tudo")},
            {QMessageBox::Open, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Abrir")},
        };

        for (const StandardButtonText &button : buttons) {
            if (QAbstractButton *widget = message_box->button(static_cast<QMessageBox::StandardButton>(button.button))) {
                widget->setText(QCoreApplication::translate("MapaMentalPlugin", button.text));
            }
        }
    }

    const auto traduzir_button_box = [](QDialogButtonBox *button_box) {
        if (button_box == nullptr) {
            return;
        }

        const StandardButtonText buttons[] = {
            {QDialogButtonBox::Ok, QT_TRANSLATE_NOOP("MapaMentalPlugin", "OK")},
            {QDialogButtonBox::Cancel, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Cancelar")},
            {QDialogButtonBox::Yes, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Sim")},
            {QDialogButtonBox::No, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Nao")},
            {QDialogButtonBox::Abort, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Abortar")},
            {QDialogButtonBox::Retry, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Tentar Novamente")},
            {QDialogButtonBox::Ignore, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Ignorar")},
            {QDialogButtonBox::Close, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Fechar")},
            {QDialogButtonBox::Help, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Ajuda")},
            {QDialogButtonBox::Apply, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Aplicar")},
            {QDialogButtonBox::Reset, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Redefinir")},
            {QDialogButtonBox::RestoreDefaults, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Restaurar Padroes")},
            {QDialogButtonBox::Save, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Salvar")},
            {QDialogButtonBox::SaveAll, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Salvar Tudo")},
            {QDialogButtonBox::Open, QT_TRANSLATE_NOOP("MapaMentalPlugin", "Abrir")},
        };

        for (const StandardButtonText &button : buttons) {
            if (QPushButton *widget = button_box->button(static_cast<QDialogButtonBox::StandardButton>(button.button))) {
                widget->setText(QCoreApplication::translate("MapaMentalPlugin", button.text));
            }
        }
    };

    traduzir_button_box(qobject_cast<QDialogButtonBox *>(dialogo));
    const auto button_boxes = dialogo->findChildren<QDialogButtonBox *>();
    for (QDialogButtonBox *button_box : button_boxes) {
        traduzir_button_box(button_box);
    }
}

int MapaMental::total_task_count() const
{
    return nos_.size();
}

int MapaMental::selected_task_count() const
{
    return selected_nodes().size();
}

void MapaMental::clicar_no_para_expandir_hierarquia(NoConceito *no)
{
    if (!navegacao_hierarquia_ativa_ || no == nullptr) {
        return;
    }

    auto it = hierarquia_filhos_.constFind(no);
    if (it == hierarquia_filhos_.constEnd()) {
        return;
    }

    if (hierarquia_expandida_.contains(no)) {
        const auto esconder_subarvore = [this](NoConceito *raiz, const auto &self) -> void {
            auto child_it = hierarquia_filhos_.constFind(raiz);
            if (child_it == hierarquia_filhos_.constEnd()) {
                return;
            }
            const QList<NoConceito *> filhos = child_it.value();
            for (NoConceito *filho : filhos) {
                if (filho == nullptr) {
                    continue;
                }
                self(filho, self);
                filho->setVisible(false);
                hierarquia_expandida_.remove(filho);
            }
        };

        esconder_subarvore(no, esconder_subarvore);
        hierarquia_expandida_.remove(no);
        atualizar_visibilidade_linhas();
        aplicar_layout_arvore();
        salvar_backup_automatico();
        return;
    }

    hierarquia_expandida_.insert(no);
    const QList<NoConceito *> filhos = it.value();
    for (NoConceito *filho : filhos) {
        if (filho != nullptr) {
            if (!filho->isVisible()) {
                filho->set_layout_animating(true);
                filho->setPos(no->pos());
                filho->set_layout_animating(false);
            }
            filho->setVisible(true);
        }
    }
    atualizar_visibilidade_linhas();
    aplicar_layout_arvore();
    salvar_backup_automatico();
}

void MapaMental::habilitar_navegacao_hierarquia(NoConceito *raiz_no)
{
    resetar_navegacao_hierarquia();
    navegacao_hierarquia_ativa_ = true;

    for (LinhaConexao *linha : conexoes_) {
        if (linha == nullptr || linha->no_inicio() == nullptr || linha->no_fim() == nullptr) {
            continue;
        }
        NoConceito *pai = linha->no_inicio();
        NoConceito *filho = linha->no_fim();
        QList<NoConceito *> &lista = hierarquia_filhos_[pai];
        if (!lista.contains(filho)) {
            lista.append(filho);
        }
        if (!hierarquia_pais_.contains(filho)) {
            hierarquia_pais_.insert(filho, pai);
        }
    }

    hierarquia_raiz_ = raiz_no;
    if (hierarquia_raiz_ == nullptr) {
        for (NoConceito *no : nos_) {
            if (no != nullptr && !hierarquia_pais_.contains(no)) {
                hierarquia_raiz_ = no;
                break;
            }
        }
    }
    if (hierarquia_raiz_ == nullptr && !nos_.isEmpty()) {
        hierarquia_raiz_ = nos_.first();
    }

    for (NoConceito *no : nos_) {
        if (no != nullptr) {
            no->setVisible(no == hierarquia_raiz_);
        }
    }

    atualizar_visibilidade_linhas();
    aplicar_layout_arvore();
}

void MapaMental::atualizar_visibilidade_linhas()
{
    for (LinhaConexao *linha : conexoes_) {
        if (linha == nullptr) {
            continue;
        }
        NoConceito *inicio = linha->no_inicio();
        NoConceito *fim = linha->no_fim();
        const bool visivel = inicio != nullptr && fim != nullptr && inicio->isVisible() && fim->isVisible();
        linha->setVisible(visivel);
    }
}

void MapaMental::resetar_navegacao_hierarquia()
{
    navegacao_hierarquia_ativa_ = false;
    hierarquia_filhos_.clear();
    hierarquia_pais_.clear();
    hierarquia_expandida_.clear();
    hierarquia_raiz_ = nullptr;
}

void MapaMental::refresh_fonts()
{
    for (NoConceito *no : nos_) {
        if (no != nullptr) {
            no->update_font();
        }
    }
}

}  // namespace source::plugin::MapaMental
