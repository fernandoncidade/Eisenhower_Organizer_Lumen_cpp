#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_24_gerar_mapa_de_hierarquia.hpp"

#include <QCoreApplication>
#include <QWidget>

#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_13_adicionar_no.hpp"
#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_20_limpar_mapa.hpp"
#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_29_aplicar_layout_arvore.hpp"
#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_25_criar_relacoes_ia.hpp"
#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_36_backup_runtime.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_01_NoConceito.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_06_MapaScene.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_07_SmoothGraphicsView.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_08_ProgressoMapaMental.hpp"

namespace source::plugin::MapaMental {
namespace {

QString resumo_obrigatorio_para_no(const QVariantMap &item, const QString &titulo)
{
    QString resumo = item.value(QStringLiteral("resumo")).toString().trimmed();
    if (!resumo.isEmpty()) {
        return resumo;
    }

    QStringList filhos;
    const QVariantList raw_children = item.value(QStringLiteral("filhos")).toList();
    for (const QVariant &child_value : raw_children) {
        const QVariantMap child = child_value.toMap();
        const QString child_summary = child.value(QStringLiteral("resumo")).toString().simplified().trimmed();
        const QString child_title = child.value(QStringLiteral("titulo")).toString().simplified().trimmed();
        if (!child_summary.isEmpty()) {
            filhos.append(child_summary.left(160));
        } else if (!child_title.isEmpty()) {
            filhos.append(child_title.left(90));
        }
        if (filhos.size() >= 4) {
            break;
        }
    }

    if (!filhos.isEmpty()) {
        return QCoreApplication::translate("MapaMentalPlugin", "Este topico organiza: %1.").arg(filhos.join(QStringLiteral(" "))).left(500);
    }
    return QCoreApplication::translate("MapaMentalPlugin", "Topico do documento relacionado a %1.").arg(titulo.left(100));
}

int contar_nos_hierarquia(const QVariantMap &item)
{
    if (item.isEmpty()) {
        return 0;
    }

    int total = 1;
    const QVariantList filhos = item.value(QStringLiteral("filhos")).toList();
    for (const QVariant &child : filhos) {
        total += contar_nos_hierarquia(child.toMap());
    }
    return total;
}

class SuspensaoAtualizacaoMapa final {
public:
    explicit SuspensaoAtualizacaoMapa(SmoothGraphicsView *view)
        : view_(view),
          viewport_(view_ != nullptr ? view_->viewport() : nullptr),
          view_updates_enabled_(view_ == nullptr || view_->updatesEnabled()),
          viewport_updates_enabled_(viewport_ == nullptr || viewport_->updatesEnabled())
    {
        if (view_ != nullptr) {
            view_->setUpdatesEnabled(false);
        }
        if (viewport_ != nullptr) {
            viewport_->setUpdatesEnabled(false);
        }
    }

    ~SuspensaoAtualizacaoMapa()
    {
        if (viewport_ != nullptr) {
            viewport_->setUpdatesEnabled(viewport_updates_enabled_);
            viewport_->update();
        }
        if (view_ != nullptr) {
            view_->setUpdatesEnabled(view_updates_enabled_);
            view_->update();
        }
    }

private:
    SmoothGraphicsView *view_ = nullptr;
    QWidget *viewport_ = nullptr;
    bool view_updates_enabled_ = true;
    bool viewport_updates_enabled_ = true;
};

}  // namespace

void MapaMental::gerar_mapa_de_hierarquia(const QVariantMap &hierarquia)
{
    gerar_mapa_de_hierarquia(hierarquia, nullptr);
}

void MapaMental::gerar_mapa_de_hierarquia(const QVariantMap &hierarquia, ProgressoMapaMental *progresso)
{
    if (hierarquia.isEmpty()) {
        return;
    }

    limpar_mapa(false);
    SuspensaoAtualizacaoMapa suspensao(progresso != nullptr ? view_ : nullptr);

    const int total_nos = contar_nos_hierarquia(hierarquia);
    if (progresso != nullptr) {
        progresso->configurar_nos(total_nos);
    }

    const bool prev_batch = carregando_em_lote_;
    carregando_em_lote_ = true;
    NoConceito *raiz = nullptr;
    {
        struct BatchGuard {
            bool &flag;
            bool previous;
            ~BatchGuard() { flag = previous; }
        } batch_guard{carregando_em_lote_, prev_batch};

        int linha = 0;
        int nos_gerados = 0;
        raiz = gerar_no_hierarquia(hierarquia, 0, linha, nullptr, progresso, total_nos, nos_gerados);
    }
    habilitar_navegacao_hierarquia(raiz);
    refresh_counter();
    salvar_backup_automatico();
    if (progresso != nullptr) {
        progresso->concluir_nos(QCoreApplication::translate("MapaMentalPlugin", "Todos os nos foram gerados."));
    }
}

NoConceito *MapaMental::gerar_no_hierarquia(const QVariantMap &item, int nivel, int &linha, NoConceito *pai)
{
    int nos_gerados = 0;
    return gerar_no_hierarquia(item, nivel, linha, pai, nullptr, contar_nos_hierarquia(item), nos_gerados);
}

NoConceito *MapaMental::gerar_no_hierarquia(const QVariantMap &item,
                                            int nivel,
                                            int &linha,
                                            NoConceito *pai,
                                            ProgressoMapaMental *progresso,
                                            int total_nos,
                                            int &nos_gerados)
{
    const QString titulo =
        item.value(QStringLiteral("titulo"), QCoreApplication::translate("MapaMentalPlugin", "Conceito"))
            .toString()
            .trimmed();
    const QPointF pos(nivel * 260.0, linha * 130.0);
    ++linha;

    QColor color = QColor("#0078d4");
    if (nivel == 1) {
        color = QColor("#2e7d32");
    } else if (nivel >= 2) {
        color = QColor("#8e44ad");
    }

    NoConceito *no = adicionar_no(titulo, pos, color);
    if (no != nullptr) {
        no->set_notas(resumo_obrigatorio_para_no(item, titulo));
    }
    if (pai != nullptr && no != nullptr) {
        criar_conexao(pai, no);
    }
    ++nos_gerados;
    if (progresso != nullptr) {
        progresso->atualizar_no(nos_gerados, total_nos, titulo);
    }

    const QVariantList filhos = item.value(QStringLiteral("filhos")).toList();
    for (const QVariant &child : filhos) {
        gerar_no_hierarquia(child.toMap(), nivel + 1, linha, no, progresso, total_nos, nos_gerados);
    }

    return no;
}

}  // namespace source::plugin::MapaMental
