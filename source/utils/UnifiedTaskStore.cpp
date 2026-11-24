#include "source/utils/UnifiedTaskStore.hpp"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QLockFile>
#include <QMutexLocker>
#include <QRecursiveMutex>
#include <QSaveFile>
#include <QSet>
#include <QVector>
#include <QUuid>

#include "source/utils/AppConfig.hpp"
#include "source/utils/CaminhoPersistenteUtils.hpp"
#include "source/utils/LogManager.hpp"

namespace source::utils::UnifiedTaskStore {
namespace {

QRecursiveMutex &process_wide_store_mutex()
{
    static QRecursiveMutex s_mutex;
    return s_mutex;
}

QHash<QString, int> &thread_local_lock_depths()
{
    static thread_local QHash<QString, int> s_depths;
    return s_depths;
}

class StoreFileLock {
public:
    explicit StoreFileLock(const QString &path)
        : lock_(path + QStringLiteral(".lock"))
    {
        if (path.trimmed().isEmpty()) {
            return;
        }

        canonical_path_ = QFileInfo(path).canonicalFilePath();
        if (canonical_path_.isEmpty()) {
            canonical_path_ = QDir::cleanPath(path);
        }

        auto &depths = thread_local_lock_depths();
        if (depths.value(canonical_path_, 0) > 0) {
            nested_ = true;
            locked_ = true;
            depths[canonical_path_]++;
            return;
        }

        lock_.setStaleLockTime(30000);
        locked_ = lock_.tryLock(5000);
        if (locked_) {
            depths[canonical_path_] = 1;
        }
    }

    ~StoreFileLock()
    {
        if (!locked_ || canonical_path_.isEmpty()) {
            return;
        }

        auto &depths = thread_local_lock_depths();
        auto it = depths.find(canonical_path_);
        if (it != depths.end()) {
            --(it.value());
            if (it.value() <= 0) {
                depths.erase(it);
                if (!nested_) {
                    lock_.unlock();
                }
            }
        }
    }

    bool is_locked() const { return locked_; }

private:
    QString canonical_path_;
    QLockFile lock_;
    bool locked_ = false;
    bool nested_ = false;
};

source::utils::LogManager *logger()
{
    return source::utils::LogManager::get_logger();
}

QVariantList legacy_records_from_document(const QJsonDocument &document, const QString &path);
QVariantList harmonize_linked_records(const QVariantList &records);

QString norm_text(const QVariant &value)
{
    return value.toString().trimmed();
}

QString fold_text(QString value)
{
    value = value.trimmed().toCaseFolded();
    value.replace(QStringLiteral("não"), QStringLiteral("nao"));
    value.replace(QStringLiteral("ã"), QStringLiteral("a"));
    value.replace(QStringLiteral("á"), QStringLiteral("a"));
    value.replace(QStringLiteral("â"), QStringLiteral("a"));
    value.replace(QStringLiteral("é"), QStringLiteral("e"));
    value.replace(QStringLiteral("ê"), QStringLiteral("e"));
    value.replace(QStringLiteral("í"), QStringLiteral("i"));
    value.replace(QStringLiteral("ó"), QStringLiteral("o"));
    value.replace(QStringLiteral("ô"), QStringLiteral("o"));
    value.replace(QStringLiteral("ú"), QStringLiteral("u"));
    value.replace(QLatin1Char(','), QLatin1Char(' '));
    return value.simplified();
}

int norm_priority(const QVariant &value)
{
    bool ok = false;
    const int as_int = value.toInt(&ok);
    if (ok) {
        if (as_int >= 1 && as_int <= 4) {
            return as_int;
        }
        if (as_int >= 0 && as_int <= 3) {
            return as_int + 1;
        }
    }

    const QString text = fold_text(value.toString());
    if (text.startsWith(QStringLiteral("🔴")) || text.contains(QStringLiteral("alta"))
        || (text.contains(QStringLiteral("importante")) && text.contains(QStringLiteral("urgente"))
            && !text.contains(QStringLiteral("nao urgente")))) {
        return 1;
    }
    if (text.startsWith(QStringLiteral("🟠")) || text.contains(QStringLiteral("media"))
        || (text.contains(QStringLiteral("importante")) && text.contains(QStringLiteral("nao urgente"))
            && !text.contains(QStringLiteral("nao importante")))) {
        return 2;
    }
    if (text.startsWith(QStringLiteral("🟡"))
        || (text.contains(QStringLiteral("nao importante")) && text.contains(QStringLiteral("urgente"))
            && !text.contains(QStringLiteral("nao urgente")))) {
        return 3;
    }
    if (text.startsWith(QStringLiteral("🟢")) || text.contains(QStringLiteral("baixa"))
        || (text.contains(QStringLiteral("nao importante")) && text.contains(QStringLiteral("nao urgente")))) {
        return 4;
    }
    return 1;
}

int norm_domain(const QVariant &value)
{
    bool ok = false;
    const int as_int = value.toInt(&ok);
    if (ok) {
        return qBound(0, as_int, 2);
    }
    return 0;
}

QString ui_text(const QString &text)
{
    return QCoreApplication::translate("InterfaceGrafica", text.toUtf8().constData());
}

QString priority_title_for_value(const QVariant &value)
{
    switch (norm_priority(value)) {
    case 2:
        return ui_text(QStringLiteral("🟠 Importante, mas Não Urgente"));
    case 3:
        return ui_text(QStringLiteral("🟡 Não Importante, mas Urgente"));
    case 4:
        return ui_text(QStringLiteral("🟢 Não Importante e Não Urgente"));
    default:
        return ui_text(QStringLiteral("🔴 Importante e Urgente"));
    }
}

QString domain_title_for_value(const QVariant &value)
{
    switch (norm_domain(value)) {
    case 1:
        return ui_text(QStringLiteral("🥈 Intermediário (explico com dificuldade)"));
    case 2:
        return ui_text(QStringLiteral("🥇 Avançado (explico facilmente)"));
    default:
        return ui_text(QStringLiteral("🥉 Iniciante (não consigo explicar)"));
    }
}

bool norm_bool(const QVariant &value)
{
    const QString text = fold_text(value.toString());
    if (text == QStringLiteral("true") || text == QStringLiteral("1") || text == QStringLiteral("yes")
        || text == QStringLiteral("sim") || text == QStringLiteral("done")
        || text == QStringLiteral("completed") || text == QStringLiteral("concluida")
        || text == QStringLiteral("concluido")) {
        return true;
    }
    return value.toBool();
}

qint64 norm_order(const QVariant &value)
{
    bool ok = false;
    const qint64 as_int = value.toLongLong(&ok);
    return ok ? as_int : 0;
}

QVariantList norm_steps(const QVariant &value)
{
    if (value.typeId() == QMetaType::QStringList) {
        QVariantList result;
        const QStringList entries = value.toStringList();
        result.reserve(entries.size());
        for (const QString &entry : entries) {
            result.append(entry);
        }
        return result;
    }
    return value.toList();
}

QVariantMap default_record()
{
    return {
        {QStringLiteral("id"), QString()},
        {QStringLiteral("modulo"), QString()},
        {QStringLiteral("tipo"), QString()},
        {QStringLiteral("titulo"), QString()},
        {QStringLiteral("descricao"), QString()},
        {QStringLiteral("explicacao"), QString()},
        {QStringLiteral("lacunas"), QString()},
        {QStringLiteral("revisao"), QString()},
        {QStringLiteral("descricao_customizada"), false},
        {QStringLiteral("prioridade"), 1},
        {QStringLiteral("titulo_prioridade"), priority_title_for_value(1)},
        {QStringLiteral("quadrante"), 1},
        {QStringLiteral("categoria"), QString()},
        {QStringLiteral("dominio"), 0},
        {QStringLiteral("titulo_dominio"), domain_title_for_value(0)},
        {QStringLiteral("status"), QString()},
        {QStringLiteral("concluida"), false},
        {QStringLiteral("in_progress"), false},
        {QStringLiteral("em_progresso"), false},
        {QStringLiteral("em_andamento"), false},
        {QStringLiteral("data"), QString()},
        {QStringLiteral("hora"), QString()},
        {QStringLiteral("arquivo_vinculado"), QString()},
        {QStringLiteral("cor_texto"), QString()},
        {QStringLiteral("cor_texto_original"), QString()},
        {QStringLiteral("cor_texto_customizada"), QString()},
        {QStringLiteral("data_criacao"), QString()},
        {QStringLiteral("ordem_insercao"), qint64(0)},
        {QStringLiteral("recurrence"), QString()},
        {QStringLiteral("recurrence_until"), QString()},
        {QStringLiteral("recurrence_source_id"), QString()},
        {QStringLiteral("review_due"), QString()},
        {QStringLiteral("review_source_module"), QString()},
        {QStringLiteral("review_interval_days"), 0},
        {QStringLiteral("completed_at"), QString()},
        {QStringLiteral("pomodoros"), 0},
        {QStringLiteral("pomodoros_completados"), 0},
        {QStringLiteral("ciclos_totais"), 4},
        {QStringLiteral("ciclos_permitidos"), 4},
        {QStringLiteral("total_ciclos"), 4},
        {QStringLiteral("ciclos_customizados"), false},
        {QStringLiteral("etapas"), QVariantList{}},
        {QStringLiteral("sincronizada_com_eisenhower"), false},
        {QStringLiteral("integracao_origem"), QString()},
        {QStringLiteral("integracao_task_id"), QString()},
        {QStringLiteral("integracao_status"), QString()},
        {QStringLiteral("integracao_date"), QString()},
        {QStringLiteral("integracao_time"), QString()},
        {QStringLiteral("integracao_completed"), false},
        {QStringLiteral("font_config"), QVariantMap{}},
        {QStringLiteral("duracao"), 1.0},
        {QStringLiteral("duration"), 1.0},
        {QStringLiteral("duracao_dias"), 1.0},
    };
}

bool is_unified_document(const QJsonDocument &document)
{
    if (!document.isObject()) {
        return false;
    }

    const QJsonObject object = document.object();
    const QString schema = object.value(QStringLiteral("schema")).toString();
    return (schema == AppConfig::unified_tasks_schema_name()
            || schema == AppConfig::legacy_unified_tasks_schema_name())
        && object.value(QStringLiteral("records")).isArray();
}

bool record_titles_missing(const QVariantMap &record)
{
    return norm_text(record.value(QStringLiteral("titulo_prioridade"))).isEmpty()
        || norm_text(record.value(QStringLiteral("titulo_dominio"))).isEmpty();
}

bool unified_records_need_title_backfill(const QVariantList &records)
{
    for (const QVariant &value : records) {
        if (record_titles_missing(value.toMap())) {
            return true;
        }
    }
    return false;
}

bool is_shared_store_path(const QString &path)
{
    return QFileInfo(path).absoluteFilePath() == QFileInfo(shared_tasks_path()).absoluteFilePath();
}

QString corrupt_backup_path(const QString &path)
{
    return QStringLiteral("%1.corrupt.%2")
        .arg(path,
             QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyyMMddHHmmsszzz")));
}

bool read_json_document(const QString &path, QJsonDocument *document, QString *error_message = nullptr)
{
    QFile file(path);
    if (!file.exists()) {
        if (error_message != nullptr) {
            *error_message = QStringLiteral("Arquivo inexistente.");
        }
        return false;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        if (error_message != nullptr) {
            *error_message = QStringLiteral("Nao foi possivel abrir o arquivo.");
        }
        return false;
    }

    QJsonParseError parse_error{};
    const QJsonDocument parsed = QJsonDocument::fromJson(file.readAll(), &parse_error);
    if (parse_error.error != QJsonParseError::NoError) {
        if (error_message != nullptr) {
            *error_message = parse_error.errorString();
        }
        return false;
    }

    if (document != nullptr) {
        *document = parsed;
    }
    return true;
}

QVariantList records_from_document(const QJsonDocument &document, const QString &path)
{
    if (is_unified_document(document)) {
        return harmonize_linked_records(document.object().value(QStringLiteral("records")).toArray().toVariantList());
    }

    return harmonize_linked_records(legacy_records_from_document(document, path));
}

bool write_unified_records(const QString &path, const QVariantList &records)
{
    const QFileInfo info(path);
    QDir().mkpath(info.absolutePath());

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        logger()->warning(QStringLiteral("Não foi possível salvar o arquivo unificado: %1").arg(path));
        return false;
    }

    const QVariantList normalized = harmonize_linked_records(records);
    QVariantMap document = {
        {QStringLiteral("schema"), AppConfig::unified_tasks_schema_name()},
        {QStringLiteral("schema_version"), 2},
        {QStringLiteral("records"), normalized},
    };

    const QByteArray payload = QJsonDocument::fromVariant(document).toJson(QJsonDocument::Indented);
    if (file.write(payload) != payload.size() || !file.commit()) {
        logger()->warning(QStringLiteral("Não foi possível confirmar o arquivo unificado: %1").arg(path));
        return false;
    }

    return true;
}

QString module_hint_from_path(const QString &path)
{
    const QString file_name = QFileInfo(path).fileName().toCaseFolded();
    if (file_name.contains(QStringLiteral("pomodoro"))) {
        return QStringLiteral("pomodoro");
    }
    if (file_name.contains(QStringLiteral("feynman"))) {
        return QStringLiteral("feynman");
    }
    if (file_name.contains(QStringLiteral("eisenhower")) || file_name == QStringLiteral("tasks.json")
        || file_name == QStringLiteral("tarefas.json")) {
        return QStringLiteral("eisenhower");
    }
    return {};
}

bool is_done_status(const QString &status)
{
    const QString folded = fold_text(status);
    return folded == QStringLiteral("done") || folded == QStringLiteral("completed")
        || folded == QStringLiteral("concluida") || folded == QStringLiteral("concluido");
}

QString canonical_pomodoro_status(const QString &status, bool completed)
{
    if (completed) {
        return QStringLiteral("Done");
    }

    if (is_done_status(status)) {
        return QStringLiteral("Done");
    }

    const QString folded = fold_text(status);
    if (folded == QStringLiteral("doing") || folded.contains(QStringLiteral("in progress"))
        || folded.contains(QStringLiteral("progress")) || folded.contains(QStringLiteral("progresso"))
        || folded.contains(QStringLiteral("andamento"))) {
        return QStringLiteral("Doing");
    }

    if (folded == QStringLiteral("todo") || folded == QStringLiteral("to do")
        || folded == QStringLiteral("afazer") || folded == QStringLiteral("a fazer")
        || folded == QStringLiteral("pending") || folded == QStringLiteral("pendente")) {
        return QStringLiteral("Todo");
    }

    return QStringLiteral("Todo");
}

QString infer_legacy_module(const QVariantMap &entry, const QString &hint)
{
    if (!hint.isEmpty()) {
        return hint;
    }

    if (entry.contains(QStringLiteral("explicacao")) || entry.contains(QStringLiteral("lacunas"))
        || entry.contains(QStringLiteral("revisao")) || entry.contains(QStringLiteral("arquivo_vinculado"))
        || entry.contains(QStringLiteral("descricao_customizada"))
        || entry.contains(QStringLiteral("integracao_task_id"))) {
        return QStringLiteral("feynman");
    }

    if (entry.contains(QStringLiteral("etapas")) || entry.contains(QStringLiteral("pomodoros"))
        || entry.contains(QStringLiteral("data_criacao")) || entry.contains(QStringLiteral("eisenhower_synced"))
        || entry.contains(QStringLiteral("status"))) {
        return QStringLiteral("pomodoro");
    }

    if (entry.contains(QStringLiteral("text")) || entry.contains(QStringLiteral("priority"))
        || entry.contains(QStringLiteral("insertion_order"))) {
        return QStringLiteral("eisenhower");
    }

    return {};
}

QVariantMap legacy_eisenhower_record(const QVariantMap &entry, int quadrant, bool completed)
{
    const QVariant priority_source = entry.value(QStringLiteral("priority"), entry.value(QStringLiteral("titulo_prioridade")));
    const QVariant domain_source = entry.value(QStringLiteral("dominio"), entry.value(QStringLiteral("titulo_dominio")));
    double dur = 1.0;
    if (entry.contains(QStringLiteral("duracao")) && entry.value(QStringLiteral("duracao")).toDouble() > 0.0) {
        dur = entry.value(QStringLiteral("duracao")).toDouble();
    } else if (entry.contains(QStringLiteral("duration")) && entry.value(QStringLiteral("duration")).toDouble() > 0.0) {
        dur = entry.value(QStringLiteral("duration")).toDouble();
    } else if (entry.contains(QStringLiteral("duracao_dias")) && entry.value(QStringLiteral("duracao_dias")).toDouble() > 0.0) {
        dur = entry.value(QStringLiteral("duracao_dias")).toDouble();
    }
    const QString legacy_status = norm_text(entry.value(QStringLiteral("status")));
    const bool in_prog = !completed && (entry.value(QStringLiteral("in_progress")).toBool()
                                        || entry.value(QStringLiteral("em_progresso")).toBool()
                                        || entry.value(QStringLiteral("em_andamento")).toBool()
                                        || fold_text(legacy_status) == QStringLiteral("doing")
                                        || fold_text(legacy_status).contains(QStringLiteral("andamento"))
                                        || fold_text(legacy_status).contains(QStringLiteral("progresso")));
    QVariantMap record = {
        {QStringLiteral("id"), norm_text(entry.value(QStringLiteral("id")))},
        {QStringLiteral("modulo"), QStringLiteral("eisenhower")},
        {QStringLiteral("tipo"), QStringLiteral("tarefa")},
        {QStringLiteral("titulo"), norm_text(entry.value(QStringLiteral("text")))},
        {QStringLiteral("descricao"), entry.value(QStringLiteral("description")).toString()},
        {QStringLiteral("prioridade"), norm_priority(priority_source.isValid() ? priority_source : QVariant(quadrant))},
        {QStringLiteral("quadrante"), qBound(1, quadrant, 4)},
        {QStringLiteral("categoria"), QStringLiteral("quadrant%1").arg(qBound(1, quadrant, 4))},
        {QStringLiteral("dominio"), norm_domain(domain_source)},
        {QStringLiteral("status"), completed ? QStringLiteral("Done") : (in_prog ? QStringLiteral("Doing") : QStringLiteral("Todo"))},
        {QStringLiteral("concluida"), completed},
        {QStringLiteral("in_progress"), in_prog},
        {QStringLiteral("em_progresso"), in_prog},
        {QStringLiteral("em_andamento"), in_prog},
        {QStringLiteral("data"), norm_text(entry.value(QStringLiteral("date")))},
        {QStringLiteral("hora"), norm_text(entry.value(QStringLiteral("time")))},
        {QStringLiteral("arquivo_vinculado"), norm_text(entry.value(QStringLiteral("file_path")))},
        {QStringLiteral("cor_texto"),
         norm_text(entry.value(QStringLiteral("cor_texto"), entry.value(QStringLiteral("task_color"))))},
        {QStringLiteral("cor_texto_original"),
         norm_text(entry.value(QStringLiteral("cor_texto_original"), entry.value(QStringLiteral("agenda_cor_texto"))))},
        {QStringLiteral("cor_texto_customizada"),
         norm_text(entry.value(QStringLiteral("cor_texto_customizada")))},
        {QStringLiteral("integracao_origem"), norm_text(entry.value(QStringLiteral("integracao_origem"),
                                                                     entry.value(QStringLiteral("origin"))))},
        {QStringLiteral("ordem_insercao"), norm_order(entry.value(QStringLiteral("insertion_order")))},
        {QStringLiteral("duracao"), dur},
        {QStringLiteral("duration"), dur},
        {QStringLiteral("duracao_dias"), dur},
    };
    return normalize_record(record);
}

QVariantMap legacy_pomodoro_record(const QVariantMap &entry)
{
    const QString status = norm_text(entry.value(QStringLiteral("status")));
    const bool completed = is_done_status(status);
    const bool in_prog = !completed && (entry.value(QStringLiteral("in_progress")).toBool()
                                        || entry.value(QStringLiteral("em_progresso")).toBool()
                                        || entry.value(QStringLiteral("em_andamento")).toBool()
                                        || status == QStringLiteral("Doing")
                                        || fold_text(status).contains(QStringLiteral("andamento"))
                                        || fold_text(status).contains(QStringLiteral("progresso")));
    const QVariant priority_source = entry.value(QStringLiteral("prioridade"), entry.value(QStringLiteral("titulo_prioridade")));
    const QVariant domain_source = entry.value(QStringLiteral("dominio"), entry.value(QStringLiteral("titulo_dominio")));
    const int prioridade = norm_priority(priority_source);
    double dur = 1.0;
    if (entry.contains(QStringLiteral("duracao")) && entry.value(QStringLiteral("duracao")).toDouble() > 0.0) {
        dur = entry.value(QStringLiteral("duracao")).toDouble();
    } else if (entry.contains(QStringLiteral("duration")) && entry.value(QStringLiteral("duration")).toDouble() > 0.0) {
        dur = entry.value(QStringLiteral("duration")).toDouble();
    } else if (entry.contains(QStringLiteral("duracao_dias")) && entry.value(QStringLiteral("duracao_dias")).toDouble() > 0.0) {
        dur = entry.value(QStringLiteral("duracao_dias")).toDouble();
    }

    QVariantMap record = {
        {QStringLiteral("id"), norm_text(entry.value(QStringLiteral("id")))},
        {QStringLiteral("modulo"), QStringLiteral("pomodoro")},
        {QStringLiteral("tipo"), QStringLiteral("tarefa")},
        {QStringLiteral("titulo"), norm_text(entry.value(QStringLiteral("titulo"), entry.value(QStringLiteral("text"))))},
        {QStringLiteral("descricao"), entry.value(QStringLiteral("descricao"), entry.value(QStringLiteral("description"))).toString()},
        {QStringLiteral("prioridade"), prioridade},
        {QStringLiteral("quadrante"), prioridade},
        {QStringLiteral("categoria"), QStringLiteral("kanban")},
        {QStringLiteral("dominio"), norm_domain(domain_source)},
        {QStringLiteral("status"), status.isEmpty() ? QStringLiteral("Todo") : status},
        {QStringLiteral("concluida"), completed},
        {QStringLiteral("in_progress"), in_prog},
        {QStringLiteral("em_progresso"), in_prog},
        {QStringLiteral("em_andamento"), in_prog},
        {QStringLiteral("data"), norm_text(entry.value(QStringLiteral("date"), entry.value(QStringLiteral("data"))))},
        {QStringLiteral("hora"), norm_text(entry.value(QStringLiteral("time"), entry.value(QStringLiteral("hora"))))},
        {QStringLiteral("arquivo_vinculado"),
         norm_text(entry.value(QStringLiteral("file_path"), entry.value(QStringLiteral("arquivo_vinculado"))))},
        {QStringLiteral("cor_texto"),
         norm_text(entry.value(QStringLiteral("cor_texto"), entry.value(QStringLiteral("task_color"))))},
        {QStringLiteral("cor_texto_original"),
         norm_text(entry.value(QStringLiteral("cor_texto_original"), entry.value(QStringLiteral("agenda_cor_texto"))))},
        {QStringLiteral("cor_texto_customizada"),
         norm_text(entry.value(QStringLiteral("cor_texto_customizada")))},
        {QStringLiteral("integracao_origem"), norm_text(entry.value(QStringLiteral("integracao_origem"),
                                                                     entry.value(QStringLiteral("origin"))))},
        {QStringLiteral("data_criacao"), norm_text(entry.value(QStringLiteral("data_criacao")))},
        {QStringLiteral("ordem_insercao"),
         norm_order(entry.value(QStringLiteral("ordem_insercao"), entry.value(QStringLiteral("insertion_order"))))},
        {QStringLiteral("pomodoros"), qMax(0, entry.value(QStringLiteral("pomodoros")).toInt())},
        {QStringLiteral("etapas"), norm_steps(entry.value(QStringLiteral("etapas")))},
        {QStringLiteral("sincronizada_com_eisenhower"),
         norm_bool(entry.value(QStringLiteral("sincronizada_com_eisenhower"),
                               entry.value(QStringLiteral("eisenhower_synced"))))},
        {QStringLiteral("duracao"), dur},
        {QStringLiteral("duration"), dur},
        {QStringLiteral("duracao_dias"), dur},
    };
    return normalize_record(record);
}

QVariantMap legacy_feynman_record(const QVariantMap &entry)
{
    const QString status = norm_text(entry.value(QStringLiteral("integracao_status"),
                                                 entry.value(QStringLiteral("status"))));
    const bool completed = norm_bool(entry.value(QStringLiteral("integracao_completed"),
                                                 entry.value(QStringLiteral("integracao_concluida"),
                                                             entry.value(QStringLiteral("completed")))))
                           || is_done_status(status);
    const bool in_prog = !completed && (entry.value(QStringLiteral("in_progress")).toBool()
                                        || entry.value(QStringLiteral("em_progresso")).toBool()
                                        || entry.value(QStringLiteral("em_andamento")).toBool()
                                        || status == QStringLiteral("Doing")
                                        || fold_text(status).contains(QStringLiteral("andamento"))
                                        || fold_text(status).contains(QStringLiteral("progresso")));
    const QVariant priority_source = entry.value(QStringLiteral("prioridade"), entry.value(QStringLiteral("titulo_prioridade")));
    const QVariant domain_source = entry.value(QStringLiteral("dominio"), entry.value(QStringLiteral("titulo_dominio")));
    const int prioridade = norm_priority(priority_source);
    const QString integration_id =
        norm_text(entry.value(QStringLiteral("integracao_task_id"), entry.value(QStringLiteral("id"))));
    const QString integration_date =
        norm_text(entry.value(QStringLiteral("integracao_date"),
                              entry.value(QStringLiteral("integracao_data"),
                                          entry.value(QStringLiteral("date")))));
    const QString integration_time =
        norm_text(entry.value(QStringLiteral("integracao_time"),
                              entry.value(QStringLiteral("integracao_hora"),
                                          entry.value(QStringLiteral("time")))));
    double dur = 1.0;
    if (entry.contains(QStringLiteral("duracao")) && entry.value(QStringLiteral("duracao")).toDouble() > 0.0) {
        dur = entry.value(QStringLiteral("duracao")).toDouble();
    } else if (entry.contains(QStringLiteral("duration")) && entry.value(QStringLiteral("duration")).toDouble() > 0.0) {
        dur = entry.value(QStringLiteral("duration")).toDouble();
    } else if (entry.contains(QStringLiteral("duracao_dias")) && entry.value(QStringLiteral("duracao_dias")).toDouble() > 0.0) {
        dur = entry.value(QStringLiteral("duracao_dias")).toDouble();
    }

    QVariantMap record = {
        {QStringLiteral("id"), integration_id},
        {QStringLiteral("modulo"), QStringLiteral("feynman")},
        {QStringLiteral("tipo"), QStringLiteral("conceito")},
        {QStringLiteral("titulo"), norm_text(entry.value(QStringLiteral("titulo")))},
        {QStringLiteral("descricao"), entry.value(QStringLiteral("descricao")).toString()},
        {QStringLiteral("explicacao"), entry.value(QStringLiteral("explicacao")).toString()},
        {QStringLiteral("lacunas"), entry.value(QStringLiteral("lacunas")).toString()},
        {QStringLiteral("revisao"), entry.value(QStringLiteral("revisao")).toString()},
        {QStringLiteral("descricao_customizada"),
         norm_bool(entry.value(QStringLiteral("descricao_customizada"))) || entry.contains(QStringLiteral("descricao"))},
        {QStringLiteral("prioridade"), prioridade},
        {QStringLiteral("quadrante"), prioridade},
        {QStringLiteral("categoria"), QStringLiteral("conceito")},
        {QStringLiteral("dominio"), norm_domain(domain_source)},
        {QStringLiteral("status"), status},
        {QStringLiteral("concluida"), completed},
        {QStringLiteral("in_progress"), in_prog},
        {QStringLiteral("em_progresso"), in_prog},
        {QStringLiteral("em_andamento"), in_prog},
        {QStringLiteral("data"), integration_date},
        {QStringLiteral("hora"), integration_time},
        {QStringLiteral("arquivo_vinculado"),
         norm_text(entry.value(QStringLiteral("arquivo_vinculado"),
                               entry.value(QStringLiteral("file_path"))))},
        {QStringLiteral("cor_texto"),
         norm_text(entry.value(QStringLiteral("cor_texto"), entry.value(QStringLiteral("task_color"))))},
        {QStringLiteral("cor_texto_original"),
         norm_text(entry.value(QStringLiteral("cor_texto_original"), entry.value(QStringLiteral("agenda_cor_texto"))))},
        {QStringLiteral("cor_texto_customizada"),
         norm_text(entry.value(QStringLiteral("cor_texto_customizada")))},
        {QStringLiteral("integracao_origem"), norm_text(entry.value(QStringLiteral("integracao_origem"),
                                                                     entry.value(QStringLiteral("origin"))))},
        {QStringLiteral("integracao_task_id"), integration_id},
        {QStringLiteral("integracao_status"), status},
        {QStringLiteral("integracao_date"), integration_date},
        {QStringLiteral("integracao_time"), integration_time},
        {QStringLiteral("integracao_completed"), completed},
        {QStringLiteral("duracao"), dur},
        {QStringLiteral("duration"), dur},
        {QStringLiteral("duracao_dias"), dur},
    };
    return normalize_record(record);
}

QVariantList legacy_records_from_array(const QJsonArray &array, const QString &path_hint)
{
    QVariantList records;
    records.reserve(array.size());

    const QString module_hint = module_hint_from_path(path_hint);
    for (const QJsonValue &value : array) {
        const QVariantMap entry = value.toObject().toVariantMap();
        const QString module = infer_legacy_module(entry, module_hint);
        if (module == QStringLiteral("pomodoro")) {
            records.append(legacy_pomodoro_record(entry));
        } else if (module == QStringLiteral("feynman")) {
            records.append(legacy_feynman_record(entry));
        }
    }

    return records;
}

QVariantList legacy_records_from_eisenhower_object(const QJsonObject &object)
{
    QVariantList records;
    const QList<QPair<QString, int>> mapping = {
        {QStringLiteral("quadrant1"), 1},
        {QStringLiteral("quadrant1_completed"), 1},
        {QStringLiteral("quadrant2"), 2},
        {QStringLiteral("quadrant2_completed"), 2},
        {QStringLiteral("quadrant3"), 3},
        {QStringLiteral("quadrant3_completed"), 3},
        {QStringLiteral("quadrant4"), 4},
        {QStringLiteral("quadrant4_completed"), 4},
    };

    for (const auto &entry : mapping) {
        const QString key = entry.first;
        if (!object.value(key).isArray()) {
            continue;
        }

        const bool completed = key.contains(QStringLiteral("_completed"));
        const QJsonArray items = object.value(key).toArray();
        for (const QJsonValue &value : items) {
            records.append(legacy_eisenhower_record(value.toObject().toVariantMap(), entry.second, completed));
        }
    }

    return records;
}

QVariantList legacy_records_from_document(const QJsonDocument &document, const QString &path)
{
    if (document.isArray()) {
        return legacy_records_from_array(document.array(), path);
    }

    if (!document.isObject()) {
        return {};
    }

    const QJsonObject object = document.object();
    if (object.contains(QStringLiteral("quadrant1")) || object.contains(QStringLiteral("quadrant2"))
        || object.contains(QStringLiteral("quadrant3")) || object.contains(QStringLiteral("quadrant4"))) {
        return legacy_records_from_eisenhower_object(object);
    }

    if (object.value(QStringLiteral("tarefas")).isArray()) {
        return legacy_records_from_array(object.value(QStringLiteral("tarefas")).toArray(), path);
    }

    return {};
}

QString record_key(const QVariant &value)
{
    return norm_text(value).toCaseFolded();
}

QString first_present_text(const QVariantMap &record, const QStringList &keys)
{
    for (const QString &key : keys) {
        const QString value = norm_text(record.value(key));
        if (!value.isEmpty()) {
            return value;
        }
    }
    return {};
}

QSet<QString> linked_keys_from_payload(const QVariantMap &payload)
{
    QSet<QString> keys;
    for (const QString &key : {QStringLiteral("id"),
                               QStringLiteral("integracao_task_id"),
                               QStringLiteral("integration_id"),
                               QStringLiteral("task_id")}) {
        const QString value = record_key(payload.value(key));
        if (!value.isEmpty()) {
            keys.insert(value);
        }
    }
    return keys;
}

bool record_matches_payload_for_removal(const QVariantMap &record, const QVariantMap &payload)
{
    const QSet<QString> payload_keys = linked_keys_from_payload(payload);
    if (!payload_keys.isEmpty()) {
        for (const QString &key : {record_key(record.value(QStringLiteral("id"))),
                                   record_key(record.value(QStringLiteral("integracao_task_id")))}) {
            if (!key.isEmpty() && payload_keys.contains(key)) {
                return true;
            }
        }
    }

    const QString payload_title = record_key(first_present_text(payload, {QStringLiteral("titulo"),
                                                                          QStringLiteral("text")}));
    if (payload_title.isEmpty() || record_key(record.value(QStringLiteral("titulo"))) != payload_title) {
        return false;
    }

    int compared_fields = 0;
    const QString payload_date = record_key(first_present_text(payload, {QStringLiteral("data"),
                                                                         QStringLiteral("date"),
                                                                         QStringLiteral("integracao_date")}));
    if (!payload_date.isEmpty()) {
        ++compared_fields;
        if (record_key(record.value(QStringLiteral("data"))) != payload_date
            && record_key(record.value(QStringLiteral("integracao_date"))) != payload_date) {
            return false;
        }
    }

    const QString payload_time = record_key(first_present_text(payload, {QStringLiteral("hora"),
                                                                         QStringLiteral("time"),
                                                                         QStringLiteral("integracao_time")}));
    if (!payload_time.isEmpty()) {
        ++compared_fields;
        if (record_key(record.value(QStringLiteral("hora"))) != payload_time
            && record_key(record.value(QStringLiteral("integracao_time"))) != payload_time) {
            return false;
        }
    }

    const QString payload_file = record_key(first_present_text(payload, {QStringLiteral("arquivo_vinculado"),
                                                                         QStringLiteral("file_path")}));
    if (!payload_file.isEmpty()) {
        ++compared_fields;
        if (record_key(record.value(QStringLiteral("arquivo_vinculado"))) != payload_file) {
            return false;
        }
    }

    QVariant payload_priority_source;
    bool has_payload_priority = false;
    for (const QString &key : {QStringLiteral("prioridade"),
                               QStringLiteral("priority"),
                               QStringLiteral("quadrante")}) {
        if (payload.contains(key) && !norm_text(payload.value(key)).isEmpty()) {
            payload_priority_source = payload.value(key);
            has_payload_priority = true;
            break;
        }
    }
    if (has_payload_priority) {
        ++compared_fields;
        const int payload_priority = norm_priority(payload_priority_source);
        const int record_priority = norm_priority(record.value(QStringLiteral("prioridade"),
                                                               record.value(QStringLiteral("quadrante"))));
        if (record_priority != payload_priority) {
            return false;
        }
    }

    return compared_fields > 0 || payload_keys.isEmpty();
}

QString default_type_for_module(const QString &module)
{
    const QString normalized = module.trimmed().toCaseFolded();
    if (normalized == QStringLiteral("feynman")) {
        return QStringLiteral("conceito");
    }
    return QStringLiteral("tarefa");
}

QString default_category_for_module(const QString &module, int quadrant)
{
    const QString normalized = module.trimmed().toCaseFolded();
    if (normalized == QStringLiteral("feynman")) {
        return QStringLiteral("conceito");
    }
    if (normalized == QStringLiteral("pomodoro")) {
        return QStringLiteral("kanban");
    }
    return QStringLiteral("quadrant%1").arg(qBound(1, quadrant, 4));
}

QString composed_feynman_description(const QVariantMap &record)
{
    const QString title = norm_text(record.value(QStringLiteral("titulo")));
    const QString explanation = record.value(QStringLiteral("explicacao")).toString().trimmed();
    const QString gaps = record.value(QStringLiteral("lacunas")).toString().trimmed();
    const QString review = record.value(QStringLiteral("revisao")).toString().trimmed();

    QStringList sections;
    if (!title.isEmpty()) {
        sections << QStringLiteral("1️⃣ Conceito:\n%1").arg(title);
    }
    if (!explanation.isEmpty()) {
        sections << QStringLiteral("2️⃣ Explique:\n%1").arg(explanation);
    }
    if (!gaps.isEmpty()) {
        sections << QStringLiteral("3️⃣ Lacunas:\n%1").arg(gaps);
    }
    if (!review.isEmpty()) {
        sections << QStringLiteral("4️⃣ Revisão:\n%1").arg(review);
    }

    return sections.join(QStringLiteral("\n\n")).trimmed();
}

class RecordDisjointSet {
public:
    explicit RecordDisjointSet(int size) : parent_(size), rank_(size, 0)
    {
        for (int i = 0; i < size; ++i) {
            parent_[i] = i;
        }
    }

    int find(int index)
    {
        if (parent_[index] != index) {
            parent_[index] = find(parent_[index]);
        }
        return parent_[index];
    }

    void unite(int left, int right)
    {
        left = find(left);
        right = find(right);
        if (left == right) {
            return;
        }

        if (rank_[left] < rank_[right]) {
            qSwap(left, right);
        }

        parent_[right] = left;
        if (rank_[left] == rank_[right]) {
            ++rank_[left];
        }
    }

private:
    QVector<int> parent_;
    QVector<int> rank_;
};

QVariantMap last_record_for_module(const QVector<QVariantMap> &records,
                                   const QVector<int> &group,
                                   const QString &module)
{
    const QString target = module.trimmed().toCaseFolded();
    for (auto it = group.crbegin(); it != group.crend(); ++it) {
        const QVariantMap &record = records.at(*it);
        if (record_key(record.value(QStringLiteral("modulo"))) == target) {
            return record;
        }
    }
    return {};
}

QString canonical_group_id(const QVector<QVariantMap> &records, const QVector<int> &group)
{
    for (auto it = group.crbegin(); it != group.crend(); ++it) {
        const QVariantMap &record = records.at(*it);
        const QString integration_id = norm_text(record.value(QStringLiteral("integracao_task_id")));
        if (!integration_id.isEmpty()) {
            return integration_id;
        }
    }

    for (auto it = group.crbegin(); it != group.crend(); ++it) {
        const QString id = norm_text(records.at(*it).value(QStringLiteral("id")));
        if (!id.isEmpty()) {
            return id;
        }
    }

    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString shared_origin_for_group(const QVector<QVariantMap> &records, const QVector<int> &group)
{
    for (auto it = group.crbegin(); it != group.crend(); ++it) {
        const QString origin = norm_text(records.at(*it).value(QStringLiteral("integracao_origem"),
                                                               records.at(*it).value(QStringLiteral("origin"))));
        if (!origin.isEmpty()) {
            return origin;
        }
    }
    return {};
}

QString shared_description_for_group(const QVariantMap &latest_record)
{
    QString description = latest_record.value(QStringLiteral("descricao")).toString().trimmed();
    if (record_key(latest_record.value(QStringLiteral("modulo"))) == QStringLiteral("feynman")
        && description.isEmpty()) {
        description = composed_feynman_description(latest_record);
    }
    return description;
}

bool is_external_feynman_task_record(const QVariantMap &record)
{
    if (record_key(record.value(QStringLiteral("modulo"))) != QStringLiteral("feynman")) {
        return false;
    }

    if (record_key(record.value(QStringLiteral("tipo"))) == QStringLiteral("tarefa")) {
        return true;
    }

    const QString origin = record_key(record.value(QStringLiteral("integracao_origem")));
    return !origin.isEmpty() && origin != QStringLiteral("feynman");
}

int projection_priority_for_module(const QVariantMap &record, const QString &module)
{
    const QString target = module.trimmed().toCaseFolded();
    const QString source = record_key(record.value(QStringLiteral("modulo")));

    if (source == target) {
        return 30;
    }

    if (target == QStringLiteral("feynman")) {
        if (source == QStringLiteral("pomodoro")) {
            return 20;
        }
        if (source == QStringLiteral("eisenhower")) {
            return 10;
        }
        return 0;
    }

    if (target == QStringLiteral("pomodoro") || target == QStringLiteral("eisenhower")) {
        if ((source == QStringLiteral("pomodoro") || source == QStringLiteral("eisenhower"))
            && source != target) {
            return 20;
        }
        if (source == QStringLiteral("feynman") || is_external_feynman_task_record(record)) {
            return 10;
        }
    }

    return 0;
}

QVariantMap projected_record_for_module(const QVariantMap &record, const QString &module)
{
    const QString target = module.trimmed().toCaseFolded();
    const QString source = record_key(record.value(QStringLiteral("modulo")));
    QVariantMap projected = normalize_record(record);

    projected.insert(QStringLiteral("modulo"), target);
    projected.insert(QStringLiteral("tipo"), default_type_for_module(target));
    projected.insert(QStringLiteral("categoria"),
                     default_category_for_module(target, projected.value(QStringLiteral("quadrante")).toInt()));

    if (target == QStringLiteral("feynman") && source != QStringLiteral("feynman")) {
        projected.insert(QStringLiteral("descricao_customizada"), true);
        projected.insert(QStringLiteral("descricao"), record.value(QStringLiteral("descricao")).toString());

        const QString integration_id = norm_text(projected.value(QStringLiteral("integracao_task_id"))).isEmpty()
                                           ? norm_text(projected.value(QStringLiteral("id")))
                                           : norm_text(projected.value(QStringLiteral("integracao_task_id")));
        projected.insert(QStringLiteral("integracao_task_id"), integration_id);

        if (norm_text(projected.value(QStringLiteral("integracao_origem"))).isEmpty()) {
            projected.insert(QStringLiteral("integracao_origem"), source);
        }
    }

    if ((target == QStringLiteral("pomodoro") || target == QStringLiteral("eisenhower"))
        && norm_text(projected.value(QStringLiteral("status"))).isEmpty()) {
        projected.insert(QStringLiteral("status"),
                         projected.value(QStringLiteral("concluida")).toBool() ? QStringLiteral("Done")
                                                                                : QStringLiteral("Todo"));
    }

    return normalize_record(projected);
}

QVariantList harmonize_linked_records(const QVariantList &records)
{
    QVector<QVariantMap> normalized_records;
    normalized_records.reserve(records.size());
    for (const QVariant &value : records) {
        normalized_records.append(normalize_record(value.toMap()));
    }

    if (normalized_records.isEmpty()) {
        return {};
    }

    RecordDisjointSet groups(normalized_records.size());
    QHash<QString, int> owners;
    for (int i = 0; i < normalized_records.size(); ++i) {
        const QVariantMap &record = normalized_records.at(i);
        const QStringList keys = {
            record_key(record.value(QStringLiteral("id"))),
            record_key(record.value(QStringLiteral("integracao_task_id"))),
        };

        for (const QString &key : keys) {
            if (key.isEmpty()) {
                continue;
            }

            const auto existing = owners.constFind(key);
            if (existing != owners.cend()) {
                groups.unite(i, existing.value());
            } else {
                owners.insert(key, i);
            }
        }
    }

    QHash<int, QVector<int>> grouped_indexes;
    for (int i = 0; i < normalized_records.size(); ++i) {
        grouped_indexes[groups.find(i)].append(i);
    }

    for (auto it = grouped_indexes.cbegin(); it != grouped_indexes.cend(); ++it) {
        const QVector<int> &group = it.value();
        if (group.isEmpty()) {
            continue;
        }

        const QVariantMap latest_record = normalized_records.at(group.constLast());
        const QVariantMap latest_feynman = last_record_for_module(normalized_records, group, QStringLiteral("feynman"));
        const QVariantMap latest_pomodoro = last_record_for_module(normalized_records, group, QStringLiteral("pomodoro"));
        const QVariantMap latest_eisenhower =
            last_record_for_module(normalized_records, group, QStringLiteral("eisenhower"));

        const QString latest_module = record_key(latest_record.value(QStringLiteral("modulo")));
        const QString canonical_id = canonical_group_id(normalized_records, group);
        const QString integration_origin = shared_origin_for_group(normalized_records, group);
        const QString title = norm_text(latest_record.value(QStringLiteral("titulo")));
        const QString description = shared_description_for_group(latest_record);
        const int priority = norm_priority(latest_record.value(QStringLiteral("prioridade")));
        const int quadrant = norm_priority(latest_record.value(QStringLiteral("quadrante"), priority));
        const int domain = norm_domain(latest_record.value(QStringLiteral("dominio")));
        QString status = norm_text(latest_record.value(QStringLiteral("status")));
        const bool completed = norm_bool(latest_record.value(QStringLiteral("concluida")));
        const QString date = norm_text(latest_record.value(QStringLiteral("data")));
        const QString time = norm_text(latest_record.value(QStringLiteral("hora")));
        const QString file_path = norm_text(latest_record.value(QStringLiteral("arquivo_vinculado")));
        const QString text_color = norm_text(latest_record.value(QStringLiteral("cor_texto")));
        QString original_text_color;
        for (auto group_it = group.crbegin(); group_it != group.crend(); ++group_it) {
            original_text_color =
                norm_text(normalized_records.at(*group_it).value(QStringLiteral("cor_texto_original"),
                                                                 normalized_records.at(*group_it)
                                                                     .value(QStringLiteral("agenda_cor_texto"))));
            if (!original_text_color.isEmpty()) {
                break;
            }
        }
        QString custom_text_color;
        for (auto group_it = group.crbegin(); group_it != group.crend(); ++group_it) {
            custom_text_color =
                norm_text(normalized_records.at(*group_it).value(QStringLiteral("cor_texto_customizada")));
            if (!custom_text_color.isEmpty()) {
                break;
            }
        }

        bool group_in_progress = false;
        if (!completed && !is_done_status(status)) {
            const bool latest_is_doing = (status.compare(QStringLiteral("Doing"), Qt::CaseInsensitive) == 0
                || latest_record.value(QStringLiteral("in_progress")).toBool()
                || latest_record.value(QStringLiteral("em_progresso")).toBool()
                || latest_record.value(QStringLiteral("em_andamento")).toBool()
                || fold_text(status).contains(QStringLiteral("andamento"))
                || fold_text(status).contains(QStringLiteral("progresso")));

            const bool latest_has_explicit_status = !status.isEmpty()
                || latest_record.contains(QStringLiteral("in_progress"))
                || latest_record.contains(QStringLiteral("em_progresso"))
                || latest_record.contains(QStringLiteral("em_andamento"));

            if (latest_is_doing) {
                group_in_progress = true;
            } else if (!latest_has_explicit_status) {
                for (auto group_it = group.crbegin(); group_it != group.crend(); ++group_it) {
                    const QVariantMap &rec = normalized_records.at(*group_it);
                    const QString s = norm_text(rec.value(QStringLiteral("status")));
                    if (!norm_bool(rec.value(QStringLiteral("concluida"))) && !is_done_status(s)) {
                        if (s.compare(QStringLiteral("Doing"), Qt::CaseInsensitive) == 0
                            || rec.value(QStringLiteral("in_progress")).toBool()
                            || rec.value(QStringLiteral("em_progresso")).toBool()
                            || rec.value(QStringLiteral("em_andamento")).toBool()
                            || fold_text(s).contains(QStringLiteral("andamento"))
                            || fold_text(s).contains(QStringLiteral("progresso"))) {
                            group_in_progress = true;
                            break;
                        }
                    }
                }
            } else {
                group_in_progress = false;
            }
        }

        const bool is_global_done = completed || is_done_status(status);
        const QString global_status = is_global_done
            ? QStringLiteral("Done")
            : (group_in_progress ? QStringLiteral("Doing") : QStringLiteral("Todo"));
        const QString resolved_pomodoro_status = global_status;

        double duration = 1.0;
        if (latest_record.contains(QStringLiteral("duracao")) && latest_record.value(QStringLiteral("duracao")).toDouble() > 0.0) {
            duration = latest_record.value(QStringLiteral("duracao")).toDouble();
        } else if (latest_record.contains(QStringLiteral("duration")) && latest_record.value(QStringLiteral("duration")).toDouble() > 0.0) {
            duration = latest_record.value(QStringLiteral("duration")).toDouble();
        } else if (latest_record.contains(QStringLiteral("duracao_dias")) && latest_record.value(QStringLiteral("duracao_dias")).toDouble() > 0.0) {
            duration = latest_record.value(QStringLiteral("duracao_dias")).toDouble();
        } else {
            for (auto group_it = group.crbegin(); group_it != group.crend(); ++group_it) {
                const QVariantMap &rec = normalized_records.at(*group_it);
                if (rec.contains(QStringLiteral("duracao")) && rec.value(QStringLiteral("duracao")).toDouble() > 0.0) {
                    duration = rec.value(QStringLiteral("duracao")).toDouble();
                    break;
                } else if (rec.contains(QStringLiteral("duration")) && rec.value(QStringLiteral("duration")).toDouble() > 0.0) {
                    duration = rec.value(QStringLiteral("duration")).toDouble();
                    break;
                } else if (rec.contains(QStringLiteral("duracao_dias")) && rec.value(QStringLiteral("duracao_dias")).toDouble() > 0.0) {
                    duration = rec.value(QStringLiteral("duracao_dias")).toDouble();
                    break;
                }
            }
        }

        int pomodoros = 0;
        if (latest_record.contains(QStringLiteral("pomodoros")) && latest_record.value(QStringLiteral("pomodoros")).toInt() >= 0) {
            pomodoros = latest_record.value(QStringLiteral("pomodoros")).toInt();
        } else if (latest_record.contains(QStringLiteral("pomodoros_completados")) && latest_record.value(QStringLiteral("pomodoros_completados")).toInt() >= 0) {
            pomodoros = latest_record.value(QStringLiteral("pomodoros_completados")).toInt();
        } else if (!latest_pomodoro.isEmpty()) {
            pomodoros = qMax(0, latest_pomodoro.value(QStringLiteral("pomodoros"), latest_pomodoro.value(QStringLiteral("pomodoros_completados"))).toInt());
        } else {
            for (auto group_it = group.crbegin(); group_it != group.crend(); ++group_it) {
                const QVariantMap &rec = normalized_records.at(*group_it);
                if (rec.contains(QStringLiteral("pomodoros"))) {
                    pomodoros = rec.value(QStringLiteral("pomodoros")).toInt();
                    break;
                } else if (rec.contains(QStringLiteral("pomodoros_completados"))) {
                    pomodoros = rec.value(QStringLiteral("pomodoros_completados")).toInt();
                    break;
                }
            }
        }

        int custom_ciclos_totais = 0;
        bool custom_ciclos = false;
        if (latest_record.value(QStringLiteral("ciclos_customizados")).toBool()) {
            custom_ciclos = true;
            custom_ciclos_totais = latest_record.value(QStringLiteral("ciclos_totais"),
                                   latest_record.value(QStringLiteral("ciclos_permitidos"),
                                   latest_record.value(QStringLiteral("total_ciclos"), 0))).toInt();
        }

        int ciclos_totais = 0;
        if (custom_ciclos && custom_ciclos_totais > 0) {
            ciclos_totais = custom_ciclos_totais;
        } else {
            if (latest_record.contains(QStringLiteral("ciclos_totais")) && latest_record.value(QStringLiteral("ciclos_totais")).toInt() > 0) {
                ciclos_totais = latest_record.value(QStringLiteral("ciclos_totais")).toInt();
            } else if (latest_record.contains(QStringLiteral("ciclos_permitidos")) && latest_record.value(QStringLiteral("ciclos_permitidos")).toInt() > 0) {
                ciclos_totais = latest_record.value(QStringLiteral("ciclos_permitidos")).toInt();
            } else if (!latest_pomodoro.isEmpty()) {
                ciclos_totais = latest_pomodoro.value(QStringLiteral("ciclos_totais"), latest_pomodoro.value(QStringLiteral("ciclos_permitidos"), 0)).toInt();
            }
            if (ciclos_totais <= 0) {
                for (auto group_it = group.crbegin(); group_it != group.crend(); ++group_it) {
                    const QVariantMap &rec = normalized_records.at(*group_it);
                    if (rec.contains(QStringLiteral("ciclos_totais")) && rec.value(QStringLiteral("ciclos_totais")).toInt() > 0) {
                        ciclos_totais = rec.value(QStringLiteral("ciclos_totais")).toInt();
                        break;
                    } else if (rec.contains(QStringLiteral("ciclos_permitidos")) && rec.value(QStringLiteral("ciclos_permitidos")).toInt() > 0) {
                        ciclos_totais = rec.value(QStringLiteral("ciclos_permitidos")).toInt();
                        break;
                    }
                }
            }
            if (ciclos_totais <= 0) {
                ciclos_totais = 4;
            }
        }

        for (int index : group) {
            QVariantMap updated = normalized_records.at(index);
            const QString module = record_key(updated.value(QStringLiteral("modulo")));

            updated.insert(QStringLiteral("id"), canonical_id);
            updated.insert(QStringLiteral("titulo"), title);
            updated.insert(QStringLiteral("prioridade"), priority);
            updated.insert(QStringLiteral("quadrante"), quadrant);
            updated.insert(QStringLiteral("dominio"), domain);
            updated.insert(QStringLiteral("duracao"), duration);
            updated.insert(QStringLiteral("duration"), duration);
            updated.insert(QStringLiteral("duracao_dias"), duration);
            updated.insert(QStringLiteral("pomodoros"), pomodoros);
            updated.insert(QStringLiteral("pomodoros_completados"), pomodoros);
            updated.insert(QStringLiteral("ciclos_totais"), ciclos_totais);
            updated.insert(QStringLiteral("ciclos_permitidos"), ciclos_totais);
            updated.insert(QStringLiteral("total_ciclos"), ciclos_totais);
            updated.insert(QStringLiteral("ciclos_customizados"), custom_ciclos);
            const QString status_for_module =
                (module == QStringLiteral("pomodoro") || module == QStringLiteral("eisenhower"))
                ? (module == QStringLiteral("pomodoro") ? resolved_pomodoro_status : global_status)
                : global_status;
            updated.insert(QStringLiteral("status"), status_for_module);
            updated.insert(QStringLiteral("concluida"), is_global_done);
            updated.insert(QStringLiteral("in_progress"), group_in_progress);
            updated.insert(QStringLiteral("em_progresso"), group_in_progress);
            updated.insert(QStringLiteral("em_andamento"), group_in_progress);
            updated.insert(QStringLiteral("data"), date);
            updated.insert(QStringLiteral("hora"), time);
            updated.insert(QStringLiteral("arquivo_vinculado"), file_path);
            updated.insert(QStringLiteral("cor_texto"), norm_text(updated.value(QStringLiteral("cor_texto"))));
            updated.insert(QStringLiteral("cor_texto_original"), original_text_color);
            updated.insert(QStringLiteral("cor_texto_customizada"), custom_text_color);
            if (!integration_origin.isEmpty()) {
                updated.insert(QStringLiteral("integracao_origem"), integration_origin);
            }
            updated.insert(QStringLiteral("integracao_task_id"), canonical_id);
            updated.insert(QStringLiteral("integracao_status"), global_status);
            updated.insert(QStringLiteral("integracao_date"), date);
            updated.insert(QStringLiteral("integracao_time"), time);
            updated.insert(QStringLiteral("integracao_completed"), completed);
            updated.insert(QStringLiteral("tipo"),
                           norm_text(updated.value(QStringLiteral("tipo"))).isEmpty()
                                ? default_type_for_module(module)
                                : norm_text(updated.value(QStringLiteral("tipo"))));
            updated.insert(QStringLiteral("categoria"), default_category_for_module(module, quadrant));

            if (!latest_feynman.isEmpty()) {
                updated.insert(QStringLiteral("explicacao"), latest_feynman.value(QStringLiteral("explicacao")).toString());
                updated.insert(QStringLiteral("lacunas"), latest_feynman.value(QStringLiteral("lacunas")).toString());
                updated.insert(QStringLiteral("revisao"), latest_feynman.value(QStringLiteral("revisao")).toString());
            }

            if (!latest_pomodoro.isEmpty()) {
                updated.insert(QStringLiteral("data_criacao"), latest_pomodoro.value(QStringLiteral("data_criacao")).toString());
                updated.insert(QStringLiteral("etapas"), norm_steps(latest_pomodoro.value(QStringLiteral("etapas"))));
                updated.insert(QStringLiteral("sincronizada_com_eisenhower"),
                               norm_bool(latest_pomodoro.value(QStringLiteral("sincronizada_com_eisenhower"))));
            }

            if (!latest_eisenhower.isEmpty()) {
                updated.insert(QStringLiteral("ordem_insercao"),
                               norm_order(latest_eisenhower.value(QStringLiteral("ordem_insercao"))));
            }

            if (module == QStringLiteral("feynman")) {
                if (latest_module == QStringLiteral("feynman")) {
                    const bool custom_description =
                        latest_feynman.value(QStringLiteral("descricao_customizada")).toBool();
                    updated.insert(QStringLiteral("descricao_customizada"), custom_description);
                    updated.insert(QStringLiteral("descricao"),
                                   custom_description
                                       ? latest_feynman.value(QStringLiteral("descricao")).toString()
                                       : QString());
                } else {
                    updated.insert(QStringLiteral("descricao_customizada"), true);
                    updated.insert(QStringLiteral("descricao"), description);
                }
            } else {
                updated.insert(QStringLiteral("descricao"), description);
            }

            normalized_records[index] = normalize_record(updated);
        }
    }

    QVariantList harmonized;
    harmonized.reserve(normalized_records.size());
    for (const QVariantMap &record : normalized_records) {
        harmonized.append(record);
    }
    return harmonized;
}

}  // namespace

QString shared_tasks_path()
{
    return QDir(source::utils::obter_caminho_persistente()).filePath(QStringLiteral("tarefas.json"));
}

QVariantMap normalize_record(const QVariantMap &record)
{
    QVariantMap normalized = default_record();
    const QVariant priority_source = record.contains(QStringLiteral("prioridade"))
                                        ? record.value(QStringLiteral("prioridade"))
                                        : record.value(QStringLiteral("titulo_prioridade"));
    const QVariant quadrant_source = record.contains(QStringLiteral("quadrante"))
                                        ? record.value(QStringLiteral("quadrante"))
                                        : priority_source;
    const QVariant domain_source = record.contains(QStringLiteral("dominio"))
                                      ? record.value(QStringLiteral("dominio"))
                                      : record.value(QStringLiteral("titulo_dominio"));
    normalized.insert(QStringLiteral("id"),
                      norm_text(record.value(QStringLiteral("id"))).isEmpty()
                          ? QUuid::createUuid().toString(QUuid::WithoutBraces)
                          : norm_text(record.value(QStringLiteral("id"))));
    normalized.insert(QStringLiteral("modulo"), norm_text(record.value(QStringLiteral("modulo"))));
    normalized.insert(QStringLiteral("tipo"), norm_text(record.value(QStringLiteral("tipo"))));
    normalized.insert(QStringLiteral("titulo"), norm_text(record.value(QStringLiteral("titulo"))));
    normalized.insert(QStringLiteral("descricao"), record.value(QStringLiteral("descricao")).toString());
    normalized.insert(QStringLiteral("explicacao"), record.value(QStringLiteral("explicacao")).toString());
    normalized.insert(QStringLiteral("lacunas"), record.value(QStringLiteral("lacunas")).toString());
    normalized.insert(QStringLiteral("revisao"), record.value(QStringLiteral("revisao")).toString());
    normalized.insert(QStringLiteral("descricao_customizada"),
                      norm_bool(record.value(QStringLiteral("descricao_customizada"))));
    normalized.insert(QStringLiteral("prioridade"), norm_priority(priority_source));
    normalized.insert(QStringLiteral("titulo_prioridade"),
                      priority_title_for_value(record.value(QStringLiteral("titulo_prioridade"), priority_source)));
    normalized.insert(QStringLiteral("quadrante"), norm_priority(quadrant_source));
    normalized.insert(QStringLiteral("categoria"), norm_text(record.value(QStringLiteral("categoria"))));
    normalized.insert(QStringLiteral("dominio"), norm_domain(domain_source));
    normalized.insert(QStringLiteral("titulo_dominio"),
                      domain_title_for_value(record.value(QStringLiteral("titulo_dominio"), domain_source)));
    const QString status_str = norm_text(record.value(QStringLiteral("status")));
    const bool is_done = norm_bool(record.value(QStringLiteral("concluida"))) || is_done_status(status_str);
    const QString folded_status = fold_text(status_str);
    const bool is_in_prog = !is_done && (norm_bool(record.value(QStringLiteral("in_progress")))
                                         || norm_bool(record.value(QStringLiteral("em_progresso")))
                                         || norm_bool(record.value(QStringLiteral("em_andamento")))
                                         || folded_status == QStringLiteral("doing")
                                         || folded_status.contains(QStringLiteral("progress"))
                                         || folded_status.contains(QStringLiteral("andamento")));

    normalized.insert(QStringLiteral("status"), norm_text(record.value(QStringLiteral("status"))));
    normalized.insert(QStringLiteral("concluida"), is_done);
    normalized.insert(QStringLiteral("in_progress"), is_in_prog);
    normalized.insert(QStringLiteral("em_progresso"), is_in_prog);
    normalized.insert(QStringLiteral("em_andamento"), is_in_prog);
    normalized.insert(QStringLiteral("data"), norm_text(record.value(QStringLiteral("data"))));
    normalized.insert(QStringLiteral("hora"), norm_text(record.value(QStringLiteral("hora"))));
    normalized.insert(QStringLiteral("arquivo_vinculado"),
                      norm_text(record.value(QStringLiteral("arquivo_vinculado"))));
    normalized.insert(QStringLiteral("cor_texto"),
                      norm_text(record.value(QStringLiteral("cor_texto"),
                                             record.value(QStringLiteral("task_color"),
                                                          record.value(QStringLiteral("foreground_color"))))));
    normalized.insert(QStringLiteral("cor_texto_original"),
                      norm_text(record.value(QStringLiteral("cor_texto_original"),
                                             record.value(QStringLiteral("agenda_cor_texto")))));
    normalized.insert(QStringLiteral("cor_texto_customizada"),
                      norm_text(record.value(QStringLiteral("cor_texto_customizada"))));
    normalized.insert(QStringLiteral("data_criacao"), norm_text(record.value(QStringLiteral("data_criacao"))));
    normalized.insert(QStringLiteral("ordem_insercao"), norm_order(record.value(QStringLiteral("ordem_insercao"))));
    normalized.insert(QStringLiteral("recurrence"), norm_text(record.value(QStringLiteral("recurrence"))));
    normalized.insert(QStringLiteral("recurrence_until"), norm_text(record.value(QStringLiteral("recurrence_until"))));
    normalized.insert(QStringLiteral("recurrence_source_id"), norm_text(record.value(QStringLiteral("recurrence_source_id"))));
    normalized.insert(QStringLiteral("review_due"), norm_text(record.value(QStringLiteral("review_due"))));
    normalized.insert(QStringLiteral("review_source_module"),
                      norm_text(record.value(QStringLiteral("review_source_module"))));
    normalized.insert(QStringLiteral("review_target_module"),
                      norm_text(record.value(QStringLiteral("review_target_module"))));
    normalized.insert(QStringLiteral("review_interval_days"),
                      qMax(0, record.value(QStringLiteral("review_interval_days")).toInt()));
    normalized.insert(QStringLiteral("completed_at"), norm_text(record.value(QStringLiteral("completed_at"))));
    const int pom_val = qMax(0, record.value(QStringLiteral("pomodoros"), record.value(QStringLiteral("pomodoros_completados"), 0)).toInt());
    normalized.insert(QStringLiteral("pomodoros"), pom_val);
    normalized.insert(QStringLiteral("pomodoros_completados"), pom_val);

    int c_totais = record.value(QStringLiteral("ciclos_totais"),
                   record.value(QStringLiteral("ciclos_permitidos"),
                   record.value(QStringLiteral("total_ciclos"),
                   record.value(QStringLiteral("ciclos_planejados"),
                   record.value(QStringLiteral("ciclos_alvo"), 4))))).toInt();
    if (c_totais <= 0) {
        c_totais = 4;
    }
    normalized.insert(QStringLiteral("ciclos_totais"), c_totais);
    normalized.insert(QStringLiteral("ciclos_permitidos"), c_totais);
    normalized.insert(QStringLiteral("total_ciclos"), c_totais);
    normalized.insert(QStringLiteral("ciclos_customizados"),
                      norm_bool(record.value(QStringLiteral("ciclos_customizados"))));
    normalized.insert(QStringLiteral("etapas"), norm_steps(record.value(QStringLiteral("etapas"))));
    normalized.insert(QStringLiteral("sincronizada_com_eisenhower"),
                      norm_bool(record.value(QStringLiteral("sincronizada_com_eisenhower"),
                                             record.value(QStringLiteral("eisenhower_synced")))));
    normalized.insert(QStringLiteral("integracao_origem"),
                      norm_text(record.value(QStringLiteral("integracao_origem"),
                                             record.value(QStringLiteral("origin")))));
    normalized.insert(QStringLiteral("integracao_task_id"),
                      norm_text(record.value(QStringLiteral("integracao_task_id"))));
    normalized.insert(QStringLiteral("integracao_status"),
                      norm_text(record.value(QStringLiteral("integracao_status"))));
    normalized.insert(QStringLiteral("integracao_date"),
                      norm_text(record.value(QStringLiteral("integracao_date"),
                                             record.value(QStringLiteral("integracao_data")))));
    normalized.insert(QStringLiteral("integracao_time"),
                      norm_text(record.value(QStringLiteral("integracao_time"),
                                             record.value(QStringLiteral("integracao_hora")))));
    normalized.insert(QStringLiteral("integracao_completed"),
                      norm_bool(record.value(QStringLiteral("integracao_completed"),
                                             record.value(QStringLiteral("integracao_concluida")))));
    normalized.insert(QStringLiteral("font_config"),
                      record.value(QStringLiteral("font_config")).toMap());
    double dur = 1.0;
    if (record.contains(QStringLiteral("duracao")) && record.value(QStringLiteral("duracao")).toDouble() > 0.0) {
        dur = record.value(QStringLiteral("duracao")).toDouble();
    } else if (record.contains(QStringLiteral("duration")) && record.value(QStringLiteral("duration")).toDouble() > 0.0) {
        dur = record.value(QStringLiteral("duration")).toDouble();
    } else if (record.contains(QStringLiteral("duracao_dias")) && record.value(QStringLiteral("duracao_dias")).toDouble() > 0.0) {
        dur = record.value(QStringLiteral("duracao_dias")).toDouble();
    }
    normalized.insert(QStringLiteral("duracao"), dur);
    normalized.insert(QStringLiteral("duration"), dur);
    normalized.insert(QStringLiteral("duracao_dias"), dur);
    return normalized;
}

QVariantList normalize_records(const QVariantList &records)
{
    QVariantList normalized;
    normalized.reserve(records.size());
    for (const QVariant &value : records) {
        normalized.append(normalize_record(value.toMap()));
    }
    return normalized;
}

QVariantList load_records(const QString &path)
{
    if (path.trimmed().isEmpty()) {
        return {};
    }

    QMutexLocker process_locker(&process_wide_store_mutex());

    if (!QFileInfo::exists(path)) {
        return {};
    }

    QString parse_error;
    QJsonDocument document;
    if (!read_json_document(path, &document, &parse_error)) {
        logger()->warning(QStringLiteral("Falha ao interpretar JSON unificado em %1: %2")
                              .arg(path, parse_error));
        const QString backup_path = path + QStringLiteral(".bak");
        QJsonDocument backup_document;
        QString backup_error;
        if (read_json_document(backup_path, &backup_document, &backup_error)) {
            const QVariantList backup_records = records_from_document(backup_document, backup_path);
            if (!backup_records.isEmpty() || is_unified_document(backup_document)) {
                const QString corrupt_path = corrupt_backup_path(path);
                QFile::copy(path, corrupt_path);
                if (write_unified_records(path, backup_records)) {
                    logger()->warning(QStringLiteral("Arquivo %1 restaurado automaticamente a partir de %2. Cópia corrompida: %3")
                                          .arg(path, backup_path, corrupt_path));
                }
                return backup_records;
            }
        }

        logger()->warning(QStringLiteral("Backup %1 indisponível ou inválido: %2").arg(backup_path, backup_error));
        return {};
    }

    if (is_unified_document(document)) {
        const QVariantList raw_records = document.object().value(QStringLiteral("records")).toArray().toVariantList();
        const QVariantList normalized_records = harmonize_linked_records(raw_records);
        if (is_shared_store_path(path) && unified_records_need_title_backfill(raw_records)) {
            save_records(path, normalized_records);
        }
        return normalized_records;
    }

    return harmonize_linked_records(legacy_records_from_document(document, path));
}

bool save_records(const QString &path, const QVariantList &records)
{
    if (path.trimmed().isEmpty()) {
        return false;
    }

    QMutexLocker process_locker(&process_wide_store_mutex());

    StoreFileLock store_lock(path);
    if (!store_lock.is_locked()) {
        logger()->warning(QStringLiteral("Tempo esgotado ao aguardar acesso exclusivo ao arquivo unificado: %1")
                              .arg(path));
        return false;
    }

    const QFileInfo info(path);
    QDir().mkpath(info.absolutePath());

    if (QFile::exists(path)) {
        QJsonDocument current_document;
        QString current_error;
        if (!read_json_document(path, &current_document, &current_error)) {
            const QString corrupt_path = corrupt_backup_path(path);
            QFile::copy(path, corrupt_path);
            logger()->warning(QStringLiteral("Salvamento bloqueado para evitar sobrescrever JSON inválido em %1. Cópia: %2. Erro: %3")
                                  .arg(path, corrupt_path, current_error));
            return false;
        }
        QFile::remove(path + QStringLiteral(".bak"));
        QFile::copy(path, path + QStringLiteral(".bak"));
    }

    if (!write_unified_records(path, records)) {
        return false;
    }

    return true;
}

QVariantList project_records_for_module(const QVariantList &records, const QString &module)
{
    const QString target = module.trimmed().toCaseFolded();
    if (target.isEmpty()) {
        return {};
    }

    const QVariantList harmonized = harmonize_linked_records(records);
    QVariantList projected;
    QHash<QString, QVariantMap> by_key;
    QHash<QString, int> by_key_priority;
    QStringList ordered_keys;
    int synthetic_index = 0;

    for (const QVariant &value : harmonized) {
        const QVariantMap record = normalize_record(value.toMap());
        const int priority = projection_priority_for_module(record, target);
        if (priority <= 0) {
            continue;
        }

        QString key = record_key(record.value(QStringLiteral("integracao_task_id")));
        if (key.isEmpty()) {
            key = record_key(record.value(QStringLiteral("id")));
        }
        if (key.isEmpty()) {
            key = QStringLiteral("__projected_%1_%2").arg(target).arg(++synthetic_index);
        }

        if (!by_key.contains(key) || priority >= by_key_priority.value(key)) {
            by_key.insert(key, projected_record_for_module(record, target));
            by_key_priority.insert(key, priority);
            if (!ordered_keys.contains(key)) {
                ordered_keys.append(key);
            }
        }
    }

    projected.reserve(ordered_keys.size());
    for (const QString &key : ordered_keys) {
        projected.append(by_key.value(key));
    }

    return projected;
}

QVariantList filter_records_by_module(const QVariantList &records, const QString &module)
{
    QVariantList filtered;
    const QString target = module.trimmed().toCaseFolded();
    for (const QVariant &value : records) {
        const QVariantMap record = normalize_record(value.toMap());
        if (record.value(QStringLiteral("modulo")).toString().trimmed().toCaseFolded() == target) {
            filtered.append(record);
        }
    }
    return filtered;
}

QVariantList merge_module_records(const QVariantList &records,
                                  const QString &module,
                                  const QVariantList &module_records)
{
    QVariantList merged;
    const QString target = module.trimmed().toCaseFolded();
    for (const QVariant &value : records) {
        const QVariantMap record = normalize_record(value.toMap());
        if (record.value(QStringLiteral("modulo")).toString().trimmed().toCaseFolded() != target) {
            merged.append(record);
        }
    }

    const QVariantList normalized_module_records = normalize_records(module_records);
    for (const QVariant &value : normalized_module_records) {
        merged.append(value);
    }
    return merged;
}

bool replace_module_records(const QString &path,
                            const QString &module,
                            const QVariantList &module_records)
{
    if (path.trimmed().isEmpty() || module.trimmed().isEmpty()) {
        return false;
    }

    StoreFileLock store_lock(path);
    if (!store_lock.is_locked()) {
        logger()->warning(QStringLiteral("Tempo esgotado ao atualizar o módulo %1 no arquivo unificado: %2")
                              .arg(module, path));
        return false;
    }

    const QVariantList records = load_records(path);
    return save_records(path, merge_module_records(records, module, module_records));
}

QVariantList remove_linked_records(const QVariantList &records,
                                   const QVariantMap &task,
                                   const QStringList &modules)
{
    QSet<QString> target_modules;
    for (const QString &module : modules) {
        const QString normalized = module.trimmed().toCaseFolded();
        if (!normalized.isEmpty()) {
            target_modules.insert(normalized);
        }
    }

    if (target_modules.isEmpty()) {
        return records;
    }

    QVariantList remaining;
    remaining.reserve(records.size());
    for (const QVariant &value : records) {
        const QVariantMap record = normalize_record(value.toMap());
        const QString module = record.value(QStringLiteral("modulo")).toString().trimmed().toCaseFolded();
        if (!target_modules.contains(module) || !record_matches_payload_for_removal(record, task)) {
            remaining.append(record);
        }
    }

    return harmonize_linked_records(remaining);
}

}  // namespace source::utils::UnifiedTaskStore
