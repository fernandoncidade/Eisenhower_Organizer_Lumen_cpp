#include "source/modules/feynman/Feynman/fey_01_FeynmanWorker.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>

#include "source/modules/feynman/Feynman/feynman_support.hpp"
#include "source/utils/CaminhoPersistenteUtils.hpp"
#include "source/utils/LogManager.hpp"
#include "source/utils/UnifiedTaskStore.hpp"

namespace source::plugin::Feynman {
namespace {

source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

QString legacy_concepts_path()
{
    return QDir(source::utils::obter_caminho_persistente()).filePath(QStringLiteral("conceitos_feynman.json"));
}

bool is_done_status(const QString &status)
{
    return status.compare(QStringLiteral("Done"), Qt::CaseInsensitive) == 0
        || status.compare(QStringLiteral("Completed"), Qt::CaseInsensitive) == 0;
}

QVariantMap concept_from_unified_record(const QVariantMap &record)
{
    const QVariantMap normalized = source::utils::UnifiedTaskStore::normalize_record(record);
    const QString status_val = normalized.value(QStringLiteral("integracao_status"),
                                                normalized.value(QStringLiteral("status"))).toString();
    const bool is_comp = normalized.value(QStringLiteral("integracao_completed"),
                                         normalized.value(QStringLiteral("concluida"))).toBool() || is_done_status(status_val);

    QVariantMap conceito = {
        {QStringLiteral("id"), normalized.value(QStringLiteral("id"))},
        {QStringLiteral("titulo"), normalized.value(QStringLiteral("titulo")).toString()},
        {QStringLiteral("explicacao"), normalized.value(QStringLiteral("explicacao")).toString()},
        {QStringLiteral("lacunas"), normalized.value(QStringLiteral("lacunas")).toString()},
        {QStringLiteral("revisao"), normalized.value(QStringLiteral("revisao")).toString()},
        {QStringLiteral("prioridade"), normalized.value(QStringLiteral("prioridade")).toInt()},
        {QStringLiteral("dominio"), normalized.value(QStringLiteral("dominio")).toInt()},
        {QStringLiteral("arquivo_vinculado"), normalized.value(QStringLiteral("arquivo_vinculado")).toString()},
        {QStringLiteral("cor_texto"), normalized.value(QStringLiteral("cor_texto")).toString()},
        {QStringLiteral("integracao_origem"), normalized.value(QStringLiteral("integracao_origem")).toString()},
        {QStringLiteral("integracao_task_id"), normalized.value(QStringLiteral("integracao_task_id")).toString()},
        {QStringLiteral("integracao_status"), status_val},
        {QStringLiteral("status"), status_val},
        {QStringLiteral("concluida"), is_comp},
        {QStringLiteral("integracao_date"), normalized.value(QStringLiteral("integracao_date"),
                                                             normalized.value(QStringLiteral("data"))).toString()},
        {QStringLiteral("integracao_time"), normalized.value(QStringLiteral("integracao_time"),
                                                             normalized.value(QStringLiteral("hora"))).toString()},
        {QStringLiteral("integracao_completed"), is_comp},
        {QStringLiteral("font_config"), normalized.value(QStringLiteral("font_config")).toMap()},
    };

    const int c_tot = normalized.value(QStringLiteral("ciclos_totais"),
                      normalized.value(QStringLiteral("ciclos_permitidos"),
                      normalized.value(QStringLiteral("total_ciclos"), 4))).toInt();
    const int safe_c_tot = c_tot > 0 ? c_tot : 4;
    conceito.insert(QStringLiteral("ciclos_totais"), safe_c_tot);
    conceito.insert(QStringLiteral("ciclos_permitidos"), safe_c_tot);
    conceito.insert(QStringLiteral("total_ciclos"), safe_c_tot);
    conceito.insert(QStringLiteral("ciclos_customizados"), normalized.value(QStringLiteral("ciclos_customizados"), false).toBool());

    const bool descricao_customizada = normalized.value(QStringLiteral("descricao_customizada")).toBool();
    conceito.insert(QStringLiteral("descricao_customizada"), descricao_customizada);
    if (descricao_customizada || !normalized.value(QStringLiteral("descricao")).toString().trimmed().isEmpty()) {
        conceito.insert(QStringLiteral("descricao"), normalized.value(QStringLiteral("descricao")).toString());
    }

    if (conceito.value(QStringLiteral("arquivo_vinculado")).toString().trimmed().isEmpty()) {
        conceito.insert(QStringLiteral("arquivo_vinculado"), QVariant());
    }

    double dur = 1.0;
    if (normalized.contains(QStringLiteral("duracao")) && normalized.value(QStringLiteral("duracao")).toDouble() > 0.0) {
        dur = normalized.value(QStringLiteral("duracao")).toDouble();
    } else if (normalized.contains(QStringLiteral("duration")) && normalized.value(QStringLiteral("duration")).toDouble() > 0.0) {
        dur = normalized.value(QStringLiteral("duration")).toDouble();
    } else if (normalized.contains(QStringLiteral("duracao_dias")) && normalized.value(QStringLiteral("duracao_dias")).toDouble() > 0.0) {
        dur = normalized.value(QStringLiteral("duracao_dias")).toDouble();
    }
    conceito.insert(QStringLiteral("duracao"), dur);
    conceito.insert(QStringLiteral("duration"), dur);

    int pom = 0;
    if (normalized.contains(QStringLiteral("pomodoros"))) {
        pom = normalized.value(QStringLiteral("pomodoros")).toInt();
    } else if (normalized.contains(QStringLiteral("pomodoros_completados"))) {
        pom = normalized.value(QStringLiteral("pomodoros_completados")).toInt();
    }
    conceito.insert(QStringLiteral("pomodoros"), pom);
    conceito.insert(QStringLiteral("pomodoros_completados"), pom);

    const bool in_prog = !is_comp && (normalized.value(QStringLiteral("in_progress")).toBool()
                         || normalized.value(QStringLiteral("em_progresso")).toBool()
                         || normalized.value(QStringLiteral("em_andamento")).toBool()
                         || status_val.compare(QStringLiteral("Doing"), Qt::CaseInsensitive) == 0
                         || status_val.contains(QStringLiteral("progress"), Qt::CaseInsensitive)
                         || status_val.contains(QStringLiteral("andamento"), Qt::CaseInsensitive));
    conceito.insert(QStringLiteral("in_progress"), in_prog);
    conceito.insert(QStringLiteral("em_progresso"), in_prog);
    conceito.insert(QStringLiteral("em_andamento"), in_prog);

    return conceito;
}

QVariantMap unified_record_from_concept(const QVariantMap &conceito)
{
    const QString status = conceito.value(QStringLiteral("integracao_status"),
                                          conceito.value(QStringLiteral("status"))).toString().trimmed();
    const bool completed = conceito.value(QStringLiteral("integracao_completed"),
                                          conceito.value(QStringLiteral("concluida"))).toBool() || is_done_status(status);
    const bool in_prog = !completed && (conceito.value(QStringLiteral("in_progress")).toBool()
                                        || conceito.value(QStringLiteral("em_progresso")).toBool()
                                        || conceito.value(QStringLiteral("em_andamento")).toBool()
                                        || status == QStringLiteral("Doing")
                                        || status.contains(QStringLiteral("progress"), Qt::CaseInsensitive)
                                        || status.contains(QStringLiteral("andamento"), Qt::CaseInsensitive));
    const int prioridade = conceito.value(QStringLiteral("prioridade")).toInt();
    const QString integration_id = conceito.value(QStringLiteral("integracao_task_id")).toString().trimmed();

    QVariantMap record = {
        {QStringLiteral("id"),
         conceito.value(QStringLiteral("id")).toString().trimmed().isEmpty() ? integration_id
                                                                             : conceito.value(QStringLiteral("id")).toString().trimmed()},
        {QStringLiteral("modulo"), QStringLiteral("feynman")},
        {QStringLiteral("tipo"), QStringLiteral("conceito")},
        {QStringLiteral("titulo"), conceito.value(QStringLiteral("titulo")).toString()},
        {QStringLiteral("descricao"), conceito.value(QStringLiteral("descricao")).toString()},
        {QStringLiteral("explicacao"), conceito.value(QStringLiteral("explicacao")).toString()},
        {QStringLiteral("lacunas"), conceito.value(QStringLiteral("lacunas")).toString()},
        {QStringLiteral("revisao"), conceito.value(QStringLiteral("revisao")).toString()},
        {QStringLiteral("descricao_customizada"),
         conceito.value(QStringLiteral("descricao_customizada")).toBool() || conceito.contains(QStringLiteral("descricao"))},
        {QStringLiteral("prioridade"), prioridade},
        {QStringLiteral("quadrante"), prioridade},
        {QStringLiteral("categoria"), QStringLiteral("conceito")},
        {QStringLiteral("dominio"), conceito.value(QStringLiteral("dominio")).toInt()},
        {QStringLiteral("status"), status},
        {QStringLiteral("concluida"), completed},
        {QStringLiteral("in_progress"), in_prog},
        {QStringLiteral("em_progresso"), in_prog},
        {QStringLiteral("em_andamento"), in_prog},
        {QStringLiteral("data"), conceito.value(QStringLiteral("integracao_date")).toString()},
        {QStringLiteral("hora"), conceito.value(QStringLiteral("integracao_time")).toString()},
        {QStringLiteral("arquivo_vinculado"), conceito.value(QStringLiteral("arquivo_vinculado")).toString()},
        {QStringLiteral("cor_texto"), conceito.value(QStringLiteral("cor_texto")).toString()},
        {QStringLiteral("integracao_origem"), conceito.value(QStringLiteral("integracao_origem")).toString()},
        {QStringLiteral("integracao_task_id"), integration_id},
        {QStringLiteral("integracao_status"), status},
        {QStringLiteral("integracao_date"), conceito.value(QStringLiteral("integracao_date")).toString()},
        {QStringLiteral("integracao_time"), conceito.value(QStringLiteral("integracao_time")).toString()},
        {QStringLiteral("integracao_completed"), completed},
        {QStringLiteral("font_config"), conceito.value(QStringLiteral("font_config")).toMap()},
    };

    const int c_tot = conceito.value(QStringLiteral("ciclos_totais"),
                      conceito.value(QStringLiteral("ciclos_permitidos"),
                      conceito.value(QStringLiteral("total_ciclos"), 4))).toInt();
    const int safe_c_tot = c_tot > 0 ? c_tot : 4;
    record.insert(QStringLiteral("ciclos_totais"), safe_c_tot);
    record.insert(QStringLiteral("ciclos_permitidos"), safe_c_tot);
    record.insert(QStringLiteral("total_ciclos"), safe_c_tot);
    record.insert(QStringLiteral("ciclos_customizados"), conceito.value(QStringLiteral("ciclos_customizados"), false).toBool());

    double dur = 1.0;
    if (conceito.contains(QStringLiteral("duracao")) && conceito.value(QStringLiteral("duracao")).toDouble() > 0.0) {
        dur = conceito.value(QStringLiteral("duracao")).toDouble();
    } else if (conceito.contains(QStringLiteral("duration")) && conceito.value(QStringLiteral("duration")).toDouble() > 0.0) {
        dur = conceito.value(QStringLiteral("duration")).toDouble();
    } else if (conceito.contains(QStringLiteral("duracao_dias")) && conceito.value(QStringLiteral("duracao_dias")).toDouble() > 0.0) {
        dur = conceito.value(QStringLiteral("duracao_dias")).toDouble();
    }
    record.insert(QStringLiteral("duracao"), dur);
    record.insert(QStringLiteral("duration"), dur);
    record.insert(QStringLiteral("duracao_dias"), dur);

    int pom = 0;
    if (conceito.contains(QStringLiteral("pomodoros"))) {
        pom = conceito.value(QStringLiteral("pomodoros")).toInt();
    } else if (conceito.contains(QStringLiteral("pomodoros_completados"))) {
        pom = conceito.value(QStringLiteral("pomodoros_completados")).toInt();
    }
    record.insert(QStringLiteral("pomodoros"), pom);
    record.insert(QStringLiteral("pomodoros_completados"), pom);

    return source::utils::UnifiedTaskStore::normalize_record(record);
}

QVariantList concepts_from_records(const QVariantList &records)
{
    QVariantList concepts;
    concepts.reserve(records.size());
    for (const QVariant &value : records) {
        const QVariantMap normalized = source::utils::UnifiedTaskStore::normalize_record(value.toMap());
        if (normalized.value(QStringLiteral("modulo")).toString().compare(QStringLiteral("feynman"), Qt::CaseInsensitive)
            == 0) {
            concepts.append(concept_from_unified_record(normalized));
        }
    }
    return concepts;
}

QVariantList unified_records_from_concepts(const QVariantList &conceitos)
{
    QVariantList records;
    records.reserve(conceitos.size());
    for (const QVariant &value : conceitos) {
        records.append(unified_record_from_concept(value.toMap()));
    }
    return records;
}

}  // namespace

FeynmanWorker::FeynmanWorker(QObject *parent) : QObject(parent)
{
    connect(this, &FeynmanWorker::request_load, this, &FeynmanWorker::_load);
    connect(this, &FeynmanWorker::request_save, this, &FeynmanWorker::_save);
}

QVariantList FeynmanWorker::load_file(const QString &arquivo_conceitos)
{
    if (arquivo_conceitos.trimmed().isEmpty()) {
        return {};
    }

    const QVariantList loaded_records = source::utils::UnifiedTaskStore::load_records(arquivo_conceitos);
    const bool sync_enabled = compat::host_bridge() == nullptr || compat::host_bridge()->task_sync_enabled();
    QVariantList shared_records = sync_enabled
        ? source::utils::UnifiedTaskStore::project_records_for_module(loaded_records, QStringLiteral("feynman"))
        : source::utils::UnifiedTaskStore::filter_records_by_module(loaded_records, QStringLiteral("feynman"));
    if (shared_records.isEmpty() && sync_enabled) {
        shared_records = source::utils::UnifiedTaskStore::filter_records_by_module(loaded_records, QStringLiteral("feynman"));
    }
    if (!shared_records.isEmpty()) {
        return concepts_from_records(shared_records);
    }

    const QString legacy_path = legacy_concepts_path();
    if (QFileInfo(legacy_path).exists()
        && QFileInfo(legacy_path).absoluteFilePath() != QFileInfo(arquivo_conceitos).absoluteFilePath()) {
        const QVariantList legacy_loaded = source::utils::UnifiedTaskStore::load_records(legacy_path);
        QVariantList legacy_records = sync_enabled
            ? source::utils::UnifiedTaskStore::project_records_for_module(legacy_loaded, QStringLiteral("feynman"))
            : source::utils::UnifiedTaskStore::filter_records_by_module(legacy_loaded, QStringLiteral("feynman"));
        if (legacy_records.isEmpty() && sync_enabled) {
            legacy_records =
                source::utils::UnifiedTaskStore::filter_records_by_module(legacy_loaded, QStringLiteral("feynman"));
        }
        if (!legacy_records.isEmpty()) {
            source::utils::UnifiedTaskStore::replace_module_records(arquivo_conceitos,
                                                                    QStringLiteral("feynman"),
                                                                    legacy_records);
            return concepts_from_records(legacy_records);
        }
    }

    return {};
}

bool FeynmanWorker::save_file(const QString &arquivo_conceitos, const QVariantList &conceitos)
{
    if (arquivo_conceitos.trimmed().isEmpty()) {
        return false;
    }

    if (!source::utils::UnifiedTaskStore::replace_module_records(arquivo_conceitos,
                                                                QStringLiteral("feynman"),
                                                                unified_records_from_concepts(conceitos))) {
        logger()->error(QStringLiteral("Erro ao salvar conceitos do Método Feynman em: %1").arg(arquivo_conceitos));
        return false;
    }

    return true;
}

void FeynmanWorker::_load(const QString &arquivo_conceitos)
{
    try {
        emit conceitos_loaded(load_file(arquivo_conceitos));
    } catch (...) {
        const QString message = QStringLiteral("Erro no worker (Feynman) ao carregar conceitos.");
        logger()->error(message, true);
        emit error(message);
    }
}

void FeynmanWorker::_save(const QString &arquivo_conceitos, const QVariantList &conceitos)
{
    try {
        const bool ok = save_file(arquivo_conceitos, conceitos);
        if (!ok) {
            emit error(QStringLiteral("Erro no worker (Feynman) ao salvar conceitos."));
        }
        emit save_finished(ok);
    } catch (...) {
        const QString message = QStringLiteral("Erro no worker (Feynman) ao salvar conceitos.");
        logger()->error(message, true);
        emit error(message);
        emit save_finished(false);
    }
}

}  // namespace source::plugin::Feynman
