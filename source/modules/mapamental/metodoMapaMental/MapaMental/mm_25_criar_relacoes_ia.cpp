#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_25_criar_relacoes_ia.hpp"

#include "source/modules/mapamental/metodoMapaMental/mp_01_NoConceito.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_02_LinhaConexao.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_06_MapaScene.hpp"

#include <memory>

#include <QEasingCurve>
#include <QVariantAnimation>

namespace source::plugin::MapaMental {

void MapaMental::criar_conexao(NoConceito *inicio, NoConceito *fim)
{
    if (scene_ == nullptr || inicio == nullptr || fim == nullptr || conexao_existe(inicio, fim)) {
        return;
    }

    auto *linha = new LinhaConexao(inicio, fim);
    linha->set_reveal_progress(0.0);
    scene_->addItem(linha);
    conexoes_.append(linha);
    inicio->add_conexao(linha);
    fim->add_conexao(linha);

    auto animation_guard = std::make_shared<bool>(true);
    conexao_animation_guards_.insert(linha, animation_guard);

    auto *animacao = new QVariantAnimation(this);
    animacao->setDuration(360);
    animacao->setEasingCurve(QEasingCurve::OutCubic);
    animacao->setStartValue(0.0);
    animacao->setEndValue(1.0);
    connect(animacao, &QVariantAnimation::valueChanged, this, [linha, animation_guard](const QVariant &valor) {
        if (!*animation_guard) {
            return;
        }
        linha->set_reveal_progress(valor.toReal());
    });
    connect(animacao, &QVariantAnimation::finished, this, [this, linha, animation_guard, animacao]() {
        if (*animation_guard) {
            conexao_animation_guards_.remove(linha);
        }
        animacao->deleteLater();
    });
    animacao->start();
}

bool MapaMental::conexao_existe(NoConceito *inicio, NoConceito *fim) const
{
    for (LinhaConexao *linha : conexoes_) {
        if (linha == nullptr) {
            continue;
        }
        const bool same = linha->no_inicio() == inicio && linha->no_fim() == fim;
        const bool inverse = linha->no_inicio() == fim && linha->no_fim() == inicio;
        if (same || inverse) {
            return true;
        }
    }
    return false;
}

}  // namespace source::plugin::MapaMental
