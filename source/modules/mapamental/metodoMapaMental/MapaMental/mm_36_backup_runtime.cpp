#include "source/modules/mapamental/metodoMapaMental/MapaMental/mm_36_backup_runtime.hpp"

#include <QDateTime>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGraphicsItem>
#include <QJsonDocument>
#include <QLabel>
#include <QSaveFile>
#include <QStandardPaths>

#include "source/modules/mapamental/metodoMapaMental/mp_01_NoConceito.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_02_LinhaConexao.hpp"
#include "source/modules/mapamental/metodoMapaMental/mp_06_MapaScene.hpp"
#include "source/utils/CaminhoPersistenteUtils.hpp"

namespace source::plugin::MapaMental {
namespace {

QString persistent_dir()
{
    const QString path = source::utils::obter_caminho_persistente();
    QDir().mkpath(path);
    return path;
}

QString legacy_persistent_dir()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (path.isEmpty()) {
        path = QDir::home().filePath(QStringLiteral(".lumen_cpp"));
    }
    return path;
}

void migrate_legacy_backup(const QString &target_path)
{
    if (QFileInfo::exists(target_path)) {
        return;
    }

    const QString legacy_path = QDir(legacy_persistent_dir()).filePath(QStringLiteral("mapa_mental_autosave.json"));
    if (QFileInfo(legacy_path).absoluteFilePath() == QFileInfo(target_path).absoluteFilePath()
        || !QFileInfo::exists(legacy_path)) {
        return;
    }

    QDir().mkpath(QFileInfo(target_path).absolutePath());
    QFile::copy(legacy_path, target_path);
}

}  // namespace

void MapaMental::refresh_counter()
{
    if (status_label_ != nullptr) {
        status_label_->setText(QCoreApplication::translate("MapaMentalPlugin", "%1 conceito(s), %2 selecionado(s)")
                                   .arg(nos_.size())
                                   .arg(selected_nodes().size()));
    }
    emit task_counter_changed();
}

void MapaMental::salvar_backup_automatico()
{
    salvar_mapa_em_arquivo(backup_path());
}

QVariantMap MapaMental::serializar_mapa() const
{
    QVariantList nodes;
    for (NoConceito *no : nos_) {
        if (no == nullptr) {
            continue;
        }
        QVariantMap item;
        item.insert(QStringLiteral("texto"), no->texto());
        item.insert(QStringLiteral("x"), no->pos().x());
        item.insert(QStringLiteral("y"), no->pos().y());
        item.insert(QStringLiteral("cor"), no->cor().name(QColor::HexArgb));
        item.insert(QStringLiteral("notas"), no->notas());
        item.insert(QStringLiteral("arquivo_vinculado"), no->arquivo_vinculado());
        item.insert(QStringLiteral("info_associada"), no->info_associada());
        item.insert(QStringLiteral("font_config"), no->font_config());
        nodes.append(item);
    }

    QVariantList edges;
    for (LinhaConexao *linha : conexoes_) {
        if (linha == nullptr) {
            continue;
        }
        const int inicio = nos_.indexOf(linha->no_inicio());
        const int fim = nos_.indexOf(linha->no_fim());
        if (inicio >= 0 && fim >= 0) {
            QVariantMap edge;
            edge.insert(QStringLiteral("inicio"), inicio);
            edge.insert(QStringLiteral("fim"), fim);
            edges.append(edge);
        }
    }

    QVariantMap root;
    QVariantList expanded_nodes;
    if (navegacao_hierarquia_ativa_) {
        for (NoConceito *no : hierarquia_expandida_) {
            const int index = nos_.indexOf(no);
            if (index >= 0) {
                expanded_nodes.append(index);
            }
        }
    }

    root.insert(QStringLiteral("version"), 2);
    root.insert(QStringLiteral("saved_at"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    root.insert(QStringLiteral("layout_mode"), layout_mode_key());
    root.insert(QStringLiteral("hierarchy_navigation_active"), navegacao_hierarquia_ativa_);
    root.insert(QStringLiteral("hierarchy_root"), nos_.indexOf(hierarquia_raiz_));
    root.insert(QStringLiteral("hierarchy_expanded"), expanded_nodes);
    root.insert(QStringLiteral("nodes"), nodes);
    root.insert(QStringLiteral("connections"), edges);
    return root;
}

bool MapaMental::salvar_mapa_em_arquivo(const QString &caminho) const
{
    if (caminho.trimmed().isEmpty()) {
        return false;
    }

    QDir().mkpath(QFileInfo(caminho).absolutePath());
    QSaveFile file(caminho);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    const QJsonDocument doc = QJsonDocument::fromVariant(serializar_mapa());
    const QByteArray payload = doc.toJson(QJsonDocument::Indented);
    return file.write(payload) == payload.size() && file.commit();
}

QString MapaMental::backup_path() const
{
    const QString path = QDir(persistent_dir()).filePath(QStringLiteral("mapa_mental_autosave.json"));
    migrate_legacy_backup(path);
    return path;
}

QList<NoConceito *> MapaMental::selected_nodes() const
{
    QList<NoConceito *> selected;
    if (scene_ == nullptr) {
        return selected;
    }
    for (QGraphicsItem *item : scene_->selectedItems()) {
        if (auto *no = dynamic_cast<NoConceito *>(item)) {
            selected.append(no);
        }
    }
    return selected;
}

}  // namespace source::plugin::MapaMental
