#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_20_limpar_mapa.hpp"

#include <QCoreApplication>
#include <QGraphicsItem>
#include <QMessageBox>
#include <QPushButton>
#include <QTimer>

#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_12_atualizar_traducoes.hpp"
#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_36_backup_runtime.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_01_NoConceito.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_02_LinhaConexao.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_06_MapaScene.hpp"

namespace source::plugin::MapaMental {

void MapaMental::limpar_mapa(bool emitir_mensagem)
{
    if (emitir_mensagem && !nos_.isEmpty()) {
        QMessageBox box(this);
        box.setIcon(QMessageBox::Question);
        box.setWindowTitle(QCoreApplication::translate("MapaMentalPlugin", "Limpar mapa"));
        box.setText(QCoreApplication::translate("MapaMentalPlugin", "Remover todos os conceitos?"));
        box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        box.setDefaultButton(QMessageBox::No);
        traduzir_botoes_padrao(&box);

        if (box.exec() != QMessageBox::Yes) {
            return;
        }
    }

    ++layout_animation_generation_;
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

    if (scene_ != nullptr) {
        scene_->clear();
    }
    nos_.clear();
    conexoes_.clear();
    qDeleteAll(conexoes_removidas_);
    conexoes_removidas_.clear();
    qDeleteAll(nos_removidos_);
    nos_removidos_.clear();
    if (cinematic_timer_ != nullptr && cinematic_timer_->isActive()) {
        cinematic_timer_->stop();
    }
    cinematic_index_ = 0;

    const bool era_modo_conexao = modo_conexao_;
    modo_conexao_ = false;
    no_origem_ = nullptr;
    if (btn_conectar_ != nullptr) {
        btn_conectar_->setChecked(false);
    }

    resetar_navegacao_hierarquia();
    salvar_backup_automatico();
    if (era_modo_conexao) {
        atualizar_traducoes();
    } else {
        refresh_counter();
    }
}

void MapaMental::remover_no(NoConceito *no)
{
    remover_nos(QList<NoConceito *>{no});
}

void MapaMental::remover_nos(const QList<NoConceito *> &nos_solicitados)
{
    if (scene_ == nullptr || nos_solicitados.isEmpty()) {
        return;
    }

    QList<NoConceito *> sementes;
    for (NoConceito *no : nos_solicitados) {
        if (no != nullptr && nos_.contains(no) && !sementes.contains(no)) {
            sementes.append(no);
        }
    }
    if (sementes.isEmpty()) {
        return;
    }

    ++layout_animation_generation_;

    QSet<NoConceito *> nos_para_remover;
    const auto coletar_subarvore = [this, &nos_para_remover](NoConceito *raiz, const auto &self) -> void {
        if (raiz == nullptr || nos_para_remover.contains(raiz)) {
            return;
        }
        nos_para_remover.insert(raiz);
        const QList<NoConceito *> filhos = hierarquia_filhos_.value(raiz);
        for (NoConceito *filho : filhos) {
            self(filho, self);
        }
    };

    for (NoConceito *no : std::as_const(sementes)) {
        if (navegacao_hierarquia_ativa_) {
            coletar_subarvore(no, coletar_subarvore);
        } else {
            nos_para_remover.insert(no);
        }
    }
    if (nos_para_remover.isEmpty()) {
        return;
    }

    for (NoConceito *existente : std::as_const(nos_)) {
        if (existente != nullptr) {
            existente->set_layout_animating(false);
        }
    }

    for (NoConceito *no_removido : std::as_const(nos_para_remover)) {
        auto no_guard_it = no_animation_guards_.find(no_removido);
        if (no_guard_it != no_animation_guards_.end()) {
            if (no_guard_it.value()) {
                *no_guard_it.value() = false;
            }
            no_animation_guards_.erase(no_guard_it);
        }
    }

    if (navegacao_hierarquia_ativa_) {
        for (NoConceito *no_removido : std::as_const(nos_para_remover)) {
            hierarquia_expandida_.remove(no_removido);
            hierarquia_filhos_.remove(no_removido);
            hierarquia_pais_.remove(no_removido);
        }
        for (auto it = hierarquia_filhos_.begin(); it != hierarquia_filhos_.end(); ++it) {
            for (NoConceito *no_removido : std::as_const(nos_para_remover)) {
                it.value().removeAll(no_removido);
            }
        }
        if (nos_para_remover.contains(hierarquia_raiz_)) {
            hierarquia_raiz_ = nullptr;
        }
    }

    QSet<LinhaConexao *> linhas_para_remover;
    for (LinhaConexao *linha : std::as_const(conexoes_)) {
        if (linha == nullptr) {
            continue;
        }
        if (nos_para_remover.contains(linha->no_inicio()) || nos_para_remover.contains(linha->no_fim())) {
            linhas_para_remover.insert(linha);
        }
    }
    for (QGraphicsItem *item : scene_->items()) {
        auto *linha = dynamic_cast<LinhaConexao *>(item);
        if (linha == nullptr) {
            continue;
        }
        if (nos_para_remover.contains(linha->no_inicio()) || nos_para_remover.contains(linha->no_fim())) {
            linhas_para_remover.insert(linha);
        }
    }

    if (scene_ != nullptr) {
        scene_->clearSelection();
    }

    for (LinhaConexao *linha : std::as_const(linhas_para_remover)) {
        if (linha->no_inicio() != nullptr) {
            linha->no_inicio()->remove_conexao(linha);
        }
        if (linha->no_fim() != nullptr) {
            linha->no_fim()->remove_conexao(linha);
        }
        auto guard_it = conexao_animation_guards_.find(linha);
        if (guard_it != conexao_animation_guards_.end()) {
            if (guard_it.value()) {
                *guard_it.value() = false;
            }
            conexao_animation_guards_.erase(guard_it);
        }
        conexoes_.removeAll(linha);
        linha->setVisible(false);
        linha->setSelected(false);
        if (scene_ != nullptr) {
            scene_->removeItem(linha);
        }
        if (!conexoes_removidas_.contains(linha)) {
            conexoes_removidas_.append(linha);
        }
    }

    for (NoConceito *no_removido : std::as_const(nos_para_remover)) {
        if (no_removido == nullptr || !nos_.contains(no_removido)) {
            continue;
        }
        if (no_origem_ == no_removido) {
            no_origem_ = nullptr;
        }
        nos_.removeAll(no_removido);
        no_removido->setVisible(false);
        no_removido->setSelected(false);
        if (scene_ != nullptr) {
            scene_->removeItem(no_removido);
        }
        if (!nos_removidos_.contains(no_removido)) {
            nos_removidos_.append(no_removido);
        }
    }

    if (navegacao_hierarquia_ativa_) {
        if (nos_.isEmpty()) {
            resetar_navegacao_hierarquia();
        } else if (hierarquia_raiz_ == nullptr || !nos_.contains(hierarquia_raiz_)) {
            for (NoConceito *candidato : std::as_const(nos_)) {
                if (candidato != nullptr && !hierarquia_pais_.contains(candidato)) {
                    hierarquia_raiz_ = candidato;
                    break;
                }
            }
            if (hierarquia_raiz_ == nullptr) {
                hierarquia_raiz_ = nos_.first();
            }
        }
    }

    atualizar_visibilidade_linhas();
    salvar_backup_automatico();
    refresh_counter();
}

}  // namespace source::plugin::MapaMental
