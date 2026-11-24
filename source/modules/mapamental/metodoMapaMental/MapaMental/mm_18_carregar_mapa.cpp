#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_18_carregar_mapa.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_13_adicionar_no.hpp"
#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_20_limpar_mapa.hpp"
#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_25_criar_relacoes_ia.hpp"
#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_36_backup_runtime.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_01_NoConceito.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_02_LinhaConexao.hpp"

namespace source::plugin::MapaMental {
namespace {

bool conexoes_formam_arvore_enraizada(const QList<NoConceito *> &nos,
                                      const QList<LinhaConexao *> &conexoes,
                                      NoConceito *&raiz)
{
    raiz = nullptr;
    if (nos.size() < 2 || conexoes.isEmpty()) {
        return false;
    }

    QSet<NoConceito *> validos;
    for (NoConceito *no : nos) {
        if (no != nullptr) {
            validos.insert(no);
        }
    }
    if (validos.size() < 2) {
        return false;
    }

    QHash<NoConceito *, NoConceito *> pais;
    QHash<NoConceito *, QList<NoConceito *>> filhos;
    int arestas_validas = 0;

    for (LinhaConexao *linha : conexoes) {
        if (linha == nullptr || linha->no_inicio() == nullptr || linha->no_fim() == nullptr) {
            return false;
        }

        NoConceito *pai = linha->no_inicio();
        NoConceito *filho = linha->no_fim();
        if (pai == filho || !validos.contains(pai) || !validos.contains(filho) || pais.contains(filho)) {
            return false;
        }

        pais.insert(filho, pai);
        filhos[pai].append(filho);
        ++arestas_validas;
    }

    if (arestas_validas != validos.size() - 1) {
        return false;
    }

    QList<NoConceito *> raizes;
    for (NoConceito *no : validos) {
        if (!pais.contains(no)) {
            raizes.append(no);
        }
    }
    if (raizes.size() != 1) {
        return false;
    }

    QSet<NoConceito *> visitados;
    QSet<NoConceito *> pilha;
    bool sem_ciclo = true;
    const auto visitar = [&filhos, &visitados, &pilha, &sem_ciclo](NoConceito *no, const auto &self) -> void {
        if (no == nullptr || !sem_ciclo) {
            return;
        }
        if (pilha.contains(no)) {
            sem_ciclo = false;
            return;
        }
        if (visitados.contains(no)) {
            return;
        }

        pilha.insert(no);
        visitados.insert(no);
        const QList<NoConceito *> descendentes = filhos.value(no);
        for (NoConceito *filho : descendentes) {
            self(filho, self);
        }
        pilha.remove(no);
    };

    visitar(raizes.first(), visitar);
    if (!sem_ciclo || visitados.size() != validos.size()) {
        return false;
    }

    raiz = raizes.first();
    return true;
}

}  // namespace

void MapaMental::carregar_mapa()
{
    const QString caminho = QFileDialog::getOpenFileName(
        this,
        QCoreApplication::translate("MapaMentalPlugin", "Carregar mapa"),
        QDir::homePath(),
        QCoreApplication::translate("MapaMentalPlugin", "JSON (*.json)"));
    if (!caminho.isEmpty()) {
        carregar_mapa_de_arquivo(caminho);
    }
}

void MapaMental::carregar_backup_automatico()
{
    const QString caminho = backup_path();
    if (QFileInfo::exists(caminho)) {
        carregar_mapa_de_arquivo(caminho);
    }
}

bool MapaMental::carregar_mapa_de_arquivo(const QString &caminho)
{
    QFile file(caminho);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return false;
    }

    limpar_mapa(false);
    const QVariantMap root = doc.object().toVariantMap();
    if (root.contains(QStringLiteral("layout_mode"))) {
        set_layout_mode_key(root.value(QStringLiteral("layout_mode")).toString(), false);
    }

    const bool prev_batch = carregando_em_lote_;
    carregando_em_lote_ = true;
    const auto restore_batch = [this, prev_batch]() {
        carregando_em_lote_ = prev_batch;
    };
    struct BatchGuard {
        decltype(restore_batch) fn;
        ~BatchGuard() { fn(); }
    } batch_guard{restore_batch};

    const QVariantList nodes = root.value(QStringLiteral("nodes")).toList();
    for (const QVariant &value : nodes) {
        const QVariantMap item = value.toMap();
        auto *no = adicionar_no(item.value(QStringLiteral("texto")).toString(),
                                QPointF(item.value(QStringLiteral("x")).toDouble(),
                                        item.value(QStringLiteral("y")).toDouble()),
                                QColor(item.value(QStringLiteral("cor"), QStringLiteral("#0078d4")).toString()));
        if (no != nullptr) {
            no->set_notas(item.value(QStringLiteral("notas")).toString());
            no->set_arquivo_vinculado(item.value(QStringLiteral("arquivo_vinculado")).toString());
            no->set_info_associada(item.value(QStringLiteral("info_associada")).toStringList());
            no->set_font_config(item.value(QStringLiteral("font_config")).toMap());
        }
    }

    const QVariantList edges = root.value(QStringLiteral("connections")).toList();
    for (const QVariant &value : edges) {
        const QVariantMap edge = value.toMap();
        const int inicio = edge.value(QStringLiteral("inicio"), -1).toInt();
        const int fim = edge.value(QStringLiteral("fim"), -1).toInt();
        if (inicio >= 0 && inicio < nos_.size() && fim >= 0 && fim < nos_.size()) {
            criar_conexao(nos_.at(inicio), nos_.at(fim));
        }
    }

    NoConceito *raiz_inferida = nullptr;
    const bool estrutura_hierarquica = conexoes_formam_arvore_enraizada(nos_, conexoes_, raiz_inferida);
    const bool tem_estado_hierarquia = root.contains(QStringLiteral("hierarchy_navigation_active"));
    const bool restaurar_hierarquia = estrutura_hierarquica
                                      && (root.value(QStringLiteral("hierarchy_navigation_active"), false).toBool()
                                          || !tem_estado_hierarquia);

    if (restaurar_hierarquia) {
        NoConceito *raiz = raiz_inferida;
        const int raiz_index = root.value(QStringLiteral("hierarchy_root"), -1).toInt();
        if (raiz_index >= 0 && raiz_index < nos_.size()) {
            raiz = nos_.at(raiz_index);
        }

        habilitar_navegacao_hierarquia(raiz);

        QSet<NoConceito *> expandidos;
        if (tem_estado_hierarquia) {
            const QVariantList expanded_nodes = root.value(QStringLiteral("hierarchy_expanded")).toList();
            for (const QVariant &expanded_value : expanded_nodes) {
                bool ok = false;
                const int index = expanded_value.toInt(&ok);
                if (ok && index >= 0 && index < nos_.size()) {
                    expandidos.insert(nos_.at(index));
                }
            }
        } else {
            for (auto it = hierarquia_filhos_.constBegin(); it != hierarquia_filhos_.constEnd(); ++it) {
                if (it.key() != nullptr && !it.value().isEmpty()) {
                    expandidos.insert(it.key());
                }
            }
        }

        QSet<NoConceito *> expandidos_validos;
        for (NoConceito *no : expandidos) {
            if (no != nullptr && hierarquia_filhos_.contains(no)) {
                expandidos_validos.insert(no);
            }
        }
        hierarquia_expandida_ = expandidos_validos;

        for (NoConceito *no : nos_) {
            if (no != nullptr) {
                no->setVisible(false);
            }
        }

        const auto mostrar_subarvore_expandida = [this](NoConceito *no, const auto &self) -> void {
            if (no == nullptr) {
                return;
            }

            no->setVisible(true);
            if (!hierarquia_expandida_.contains(no)) {
                return;
            }

            const QList<NoConceito *> filhos = hierarquia_filhos_.value(no);
            for (NoConceito *filho : filhos) {
                self(filho, self);
            }
        };
        mostrar_subarvore_expandida(hierarquia_raiz_, mostrar_subarvore_expandida);
        atualizar_visibilidade_linhas();
        aplicar_layout_arvore();
    }

    refresh_counter();
    salvar_backup_automatico();
    return true;
}

}  // namespace source::plugin::MapaMental
