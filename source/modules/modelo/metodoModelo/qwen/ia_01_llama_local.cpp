#include "source/modules/modelo/metodoModelo/qwen/ia_01_llama_local.hpp"

#include <algorithm>

#include <QDir>
#include <QDirIterator>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLibrary>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSet>
#include <QStringConverter>
#include <QTemporaryDir>
#include <QTextStream>
#include <QThread>
#include <QTimer>

#include "source/utils/ApplicationPathUtils.hpp"
#include "source/utils/CaminhoPersistenteUtils.hpp"

namespace source::plugin::Modelo::ProcessadorIAFns {
namespace {

constexpr int kMaxInputChars = 18000;
constexpr int kDefaultTimeoutMs = 300000;
constexpr int kRuntimeProbeTimeoutMs = 12000;

struct LlamaRuntime {
    QString cli;
    QString label;
    bool gpu = false;
};

struct OutlineSection {
    int id = 0;
    int level = 1;
    int parent_id = 0;
    QString title;
    QString body;
};

struct OutlinePlan {
    QString document_title;
    QList<OutlineSection> sections;
    int max_depth = 1;
    bool usable = false;
};

QString resumo_fonte(const OutlineSection &section);

QString env_value(const QString &name)
{
    return QProcessEnvironment::systemEnvironment().value(name).trimmed();
}

int env_int(const QString &name, int fallback, int minimum, int maximum)
{
    bool ok = false;
    const int value = env_value(name).toInt(&ok);
    return ok ? qBound(minimum, value, maximum) : fallback;
}

bool env_enabled(const QString &name)
{
    const QString value = env_value(name).toCaseFolded();
    return value == QStringLiteral("1") || value == QStringLiteral("true")
           || value == QStringLiteral("yes") || value == QStringLiteral("on");
}

bool valid_gguf_file(const QFileInfo &info)
{
    if (!info.isFile() || info.suffix().compare(QStringLiteral("gguf"), Qt::CaseInsensitive) != 0) {
        return false;
    }
    QFile file(info.absoluteFilePath());
    return file.open(QIODevice::ReadOnly) && file.read(4) == QByteArrayLiteral("GGUF");
}

int model_score(const QFileInfo &model)
{
    const QString name = model.fileName().toCaseFolded();
    if (name.contains(QStringLiteral("mmproj")) || name.contains(QStringLiteral("projector"))
        || name.contains(QStringLiteral("embedding")) || name.contains(QStringLiteral("reranker"))) {
        return -1000;
    }

    int score = 0;
    if (name.contains(QStringLiteral("instruct"))) {
        score += 8;
    }
    if (name.contains(QStringLiteral("chat"))) {
        score += 5;
    }
    if (name.contains(QStringLiteral("3b")) || name.contains(QStringLiteral("4b"))) {
        score += 6;
    } else if (name.contains(QStringLiteral("1.5b")) || name.contains(QStringLiteral("1b"))) {
        score += 2;
    }
    if (name.contains(QStringLiteral("q4_k_m")) || name.contains(QStringLiteral("q5_k_m"))) {
        score += 4;
    } else if (name.contains(QStringLiteral("q4")) || name.contains(QStringLiteral("q5"))) {
        score += 2;
    }
    return score;
}

QString best_gguf_in_dir(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) {
        return {};
    }

    QFileInfo best;
    int best_score = -1000;
    QDirIterator iterator(path,
                          {QStringLiteral("*.gguf")},
                          QDir::Files | QDir::Readable,
                          QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        const QFileInfo candidate(iterator.next());
        if (!valid_gguf_file(candidate)) {
            continue;
        }
        const int score = model_score(candidate);
        if (score > best_score
            || (score == best_score && (!best.exists() || candidate.size() < best.size()))) {
            best = candidate;
            best_score = score;
        }
    }
    return best_score < 0 || !best.exists() ? QString() : best.absoluteFilePath();
}

QString descobrir_modelo()
{
    for (const QString &configured : {env_value(QStringLiteral("MODELO_QWEN_MODEL")),
                                      env_value(QStringLiteral("MAPAMENTAL_LLAMA_MODEL")),
                                      env_value(QStringLiteral("LUMEN_LLAMA_MODEL"))}) {
        const QFileInfo info(configured);
        if (valid_gguf_file(info)) {
            return info.absoluteFilePath();
        }
        if (info.isDir()) {
            const QString model = best_gguf_in_dir(info.absoluteFilePath());
            if (!model.isEmpty()) {
                return model;
            }
        }
    }

    const QString app_base = source::utils::get_app_base_path();
    const QString persistent_dir = source::utils::obter_caminho_persistente();

    const QStringList model_dirs = {
        QDir(persistent_dir).filePath(QStringLiteral("models")),
        QDir(persistent_dir).filePath(QStringLiteral("models/modelo")),
        QDir(persistent_dir).filePath(QStringLiteral("modelo/models")),
        QDir(app_base).filePath(QStringLiteral("models")),
        QDir(app_base).filePath(QStringLiteral("plugins/modelo/models")),
        QDir(app_base).filePath(QStringLiteral("plugins/modelo/llama/models")),
        QDir(app_base).filePath(QStringLiteral("models/modelo")),
    };

    for (const QString &dir : model_dirs) {
        const QString model = best_gguf_in_dir(dir);
        if (!model.isEmpty()) {
            return model;
        }
    }

    return {};
}

QProcessEnvironment runtime_environment(const QString &runtime_dir)
{
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    const QChar separator = QDir::listSeparator();
    const QString current_path = environment.value(QStringLiteral("PATH"));
    environment.insert(QStringLiteral("PATH"),
                       QDir::toNativeSeparators(runtime_dir) + separator + current_path);
    environment.insert(QStringLiteral("GGML_BACKEND_PATH"), QDir::toNativeSeparators(runtime_dir));
    return environment;
}

bool has_required_runtime_files(const QString &dir, bool gpu)
{
    if (dir.trimmed().isEmpty()) {
        return false;
    }

    const QStringList common_files = {
        QStringLiteral("llama-cli.exe"),
        QStringLiteral("llama-common.dll"),
        QStringLiteral("llama.dll"),
        QStringLiteral("ggml.dll"),
        QStringLiteral("ggml-base.dll"),
        QStringLiteral("ggml-cpu-x64.dll"),
        QStringLiteral("libomp140.x86_64.dll"),
        QStringLiteral("mtmd.dll"),
    };
    for (const QString &file_name : common_files) {
        if (!QFileInfo::exists(QDir(dir).filePath(file_name))) {
            return false;
        }
    }

    return !gpu || QFileInfo::exists(QDir(dir).filePath(QStringLiteral("ggml-vulkan.dll")));
}

QString run_runtime_probe(const QString &cli)
{
    QProcess process;
    process.setProgram(cli);
    process.setArguments({QStringLiteral("--help")});
    process.setWorkingDirectory(QFileInfo(cli).absolutePath());
    process.setProcessEnvironment(runtime_environment(QFileInfo(cli).absolutePath()));
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start();
    if (!process.waitForStarted(5000) || !process.waitForFinished(kRuntimeProbeTimeoutMs)) {
        process.kill();
        process.waitForFinished(3000);
        return {};
    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        return {};
    }

    return QString::fromUtf8(process.readAllStandardOutput());
}

bool vulkan_runtime_available(const QString &dir)
{
    if (!has_required_runtime_files(dir, true)) {
        return false;
    }

    QLibrary vulkan_loader(QStringLiteral("vulkan-1"));
    if (!vulkan_loader.load()) {
        return false;
    }
    vulkan_loader.unload();

    const QString output = run_runtime_probe(QDir(dir).filePath(QStringLiteral("llama-cli.exe")));
    return output.contains(QStringLiteral("loaded Vulkan backend"), Qt::CaseInsensitive);
}

bool cpu_runtime_available(const QString &dir)
{
    if (!has_required_runtime_files(dir, false)) {
        return false;
    }

    const QString output = run_runtime_probe(QDir(dir).filePath(QStringLiteral("llama-cli.exe")));
    return output.contains(QStringLiteral("loaded CPU backend"), Qt::CaseInsensitive) || !output.trimmed().isEmpty();
}

void append_runtime_if_ready(QList<LlamaRuntime> &runtimes, const QString &dir, bool gpu, const QString &label)
{
    const QString absolute_dir = QFileInfo(dir).absoluteFilePath();
    const bool ready = gpu ? vulkan_runtime_available(absolute_dir) : cpu_runtime_available(absolute_dir);
    if (!ready) {
        return;
    }

    const QString cli = QFileInfo(QDir(absolute_dir).filePath(QStringLiteral("llama-cli.exe"))).absoluteFilePath();
    for (const LlamaRuntime &runtime : runtimes) {
        if (QFileInfo(runtime.cli).absoluteFilePath() == cli) {
            return;
        }
    }

    runtimes.append(LlamaRuntime{cli, label, gpu});
}

QList<LlamaRuntime> descobrir_runtimes_uncached()
{
    QList<LlamaRuntime> runtimes;
    const QString explicit_gpu_dir = env_value(QStringLiteral("MAPAMENTAL_LLAMA_GPU_DIR"));
    const QString explicit_cpu_dir = env_value(QStringLiteral("MAPAMENTAL_LLAMA_CPU_DIR"));
    const QString explicit_dir = env_value(QStringLiteral("MAPAMENTAL_LLAMA_DIR"));
    const QString lumen_dir = env_value(QStringLiteral("LUMEN_LLAMA_DIR"));
    const QDir source_qwen_dir = QFileInfo(QString::fromUtf8(__FILE__)).absoluteDir();
    const QString app_base = source::utils::get_app_base_path();
    const QString app_root = QFileInfo(app_base).dir().absolutePath();
    QString backend_preference = env_value(QStringLiteral("MAPAMENTAL_LLAMA_BACKEND")).toCaseFolded();
    if (backend_preference != QStringLiteral("cpu") && backend_preference != QStringLiteral("vulkan")) {
        backend_preference = QStringLiteral("auto");
    }
    const bool allow_gpu = backend_preference == QStringLiteral("auto")
                           || backend_preference == QStringLiteral("vulkan");
    const bool allow_cpu = backend_preference == QStringLiteral("auto")
                           || backend_preference == QStringLiteral("cpu");

    const QStringList gpu_dirs = {
        explicit_gpu_dir,
        explicit_dir,
        lumen_dir,
        QDir(app_base).filePath(QStringLiteral("plugins/modelo/llama/vulkan")),
        QDir(app_base).filePath(QStringLiteral("plugins/modelo/llama-gpu")),
        QDir(app_base).filePath(QStringLiteral("llama/vulkan")),
        QDir(app_root).filePath(QStringLiteral("source/modules/modelo/metodoModelo/llama/vulkan")),
        QDir(app_root).filePath(QStringLiteral("source/modules/modelo/metodoModelo/llama-b8987-bin-win-vulkan-x64")),
        source_qwen_dir.filePath(QStringLiteral("llama-b8987-bin-win-vulkan-x64")),
    };
    if (allow_gpu) {
        for (const QString &dir : gpu_dirs) {
            append_runtime_if_ready(runtimes, dir, true, QStringLiteral("llama.cpp Vulkan"));
        }
    }

    const QStringList cpu_dirs = {
        explicit_cpu_dir,
        explicit_dir,
        lumen_dir,
        QDir(app_base).filePath(QStringLiteral("plugins/modelo/llama/cpu")),
        QDir(app_base).filePath(QStringLiteral("plugins/modelo/llama")),
        QDir(app_base).filePath(QStringLiteral("llama/cpu")),
        QDir(app_root).filePath(QStringLiteral("source/modules/modelo/metodoModelo/llama/cpu")),
        QDir(app_root).filePath(QStringLiteral("source/modules/modelo/metodoModelo/llama-b8987-bin-win-cpu-x64")),
        source_qwen_dir.filePath(QStringLiteral("llama-b8987-bin-win-cpu-x64")),
    };
    if (allow_cpu) {
        for (const QString &dir : cpu_dirs) {
            append_runtime_if_ready(runtimes, dir, false, QStringLiteral("llama.cpp CPU"));
        }
    }

    return runtimes;
}

QList<LlamaRuntime> descobrir_runtimes()
{
    // Os binários implantados e o backend selecionado não mudam durante a
    // execução do aplicativo. O cache evita iniciar dois probes --help para
    // cada documento e também atende ao diagnóstico exibido na interface.
    static const QList<LlamaRuntime> cached = descobrir_runtimes_uncached();
    return cached;
}

QString amostra_documento(const QString &texto, int max_input_chars)
{
    QString cleaned = texto;
    cleaned.replace(QRegularExpression(QStringLiteral("[\\t ]+")), QStringLiteral(" "));
    cleaned.replace(QRegularExpression(QStringLiteral("\\n{3,}")), QStringLiteral("\n\n"));
    cleaned = cleaned.trimmed();

    const int limit = qBound(3000, max_input_chars, kMaxInputChars);
    if (cleaned.size() <= limit) {
        return cleaned;
    }

    constexpr int kSegments = 8;
    const int segment_size = qMax(300, (limit - 320) / kSegments);
    QStringList samples;
    samples.reserve(kSegments);
    for (int index = 0; index < kSegments; ++index) {
        int start = index == 0
                        ? 0
                        : ((cleaned.size() - segment_size) * index) / (kSegments - 1);
        if (start > 0) {
            const int next_line = cleaned.indexOf(QLatin1Char('\n'), start);
            if (next_line >= 0 && next_line - start <= 240) {
                start = next_line + 1;
            }
        }
        const QString sample = cleaned.mid(start, segment_size).trimmed();
        if (!sample.isEmpty()) {
            samples.append(QStringLiteral("[TRECHO %1/%2]\n%3").arg(index + 1).arg(kSegments).arg(sample));
        }
    }
    return samples.join(QStringLiteral("\n\n"));
}

const QRegularExpression &regex_titulo_estrutural()
{
    static const QRegularExpression expression(
        QStringLiteral("^(?:#{1,6}\\s+.+|(?:parte|part|cap[ií]tulo|chapter)\\s+[ivxlcdm0-9]+.*)$"),
        QRegularExpression::CaseInsensitiveOption);
    return expression;
}

bool titulo_editorial_relevante(const QString &title)
{
    const QString key = title.toCaseFolded();
    static const QStringList excluded = {
        QStringLiteral("crédito"),
        QStringLiteral("credito"),
        QStringLiteral("folha de rosto"),
        QStringLiteral("sumário"),
        QStringLiteral("sumario"),
        QStringLiteral("nota do autor"),
        QStringLiteral("prefácio"),
        QStringLiteral("prefacio"),
        QStringLiteral("perguntas frequentes"),
        QStringLiteral("frequently asked"),
        QStringLiteral("guia de recursos"),
        QStringLiteral("resource guide"),
        QStringLiteral("agradecimento"),
        QStringLiteral("acknowledg"),
        QStringLiteral("sobre os autores"),
        QStringLiteral("about the author"),
    };
    for (const QString &term : excluded) {
        if (key.contains(term)) {
            return false;
        }
    }
    return true;
}

OutlinePlan construir_plano_outline(const QString &texto)
{
    OutlinePlan plan;
    const QStringList lines = texto.split(QLatin1Char('\n'));

    for (const QString &raw_line : lines) {
        const QString line = raw_line.simplified().trimmed();
        if (line.startsWith(QStringLiteral("NOME DO ARQUIVO/FONTE:"))) {
            plan.document_title = line.mid(line.indexOf(QLatin1Char(':')) + 1).simplified().trimmed();
            const int publisher_suffix = plan.document_title.indexOf(QStringLiteral(" -- "));
            if (publisher_suffix > 0) {
                plan.document_title = plan.document_title.left(publisher_suffix).trimmed();
            }
            plan.document_title.replace(
                QRegularExpression(QStringLiteral("(?<=\\p{L})-(?=\\s)")),
                QStringLiteral(" -"));
            break;
        }
        if (line.isEmpty() || line.startsWith(QLatin1Char('#')) || line.size() > 180) {
            continue;
        }
        plan.document_title = line;
        break;
    }

    OutlineSection current;
    bool has_current = false;
    QStringList body;
    const auto flush = [&]() {
        if (has_current) {
            current.body = body.join(QLatin1Char(' ')).simplified().trimmed();
            plan.sections.append(current);
        }
        body.clear();
        has_current = false;
    };

    for (const QString &raw_line : lines) {
        const QString line = raw_line.simplified().trimmed();
        if (line.isEmpty()) {
            continue;
        }
        if (line.size() <= 180 && line.startsWith(QLatin1Char('#'))
            && regex_titulo_estrutural().match(line).hasMatch()) {
            flush();
            int level = 1;
            while (level < line.size() && line.at(level) == QLatin1Char('#')) {
                ++level;
            }
            QString title = line;
            if (line.startsWith(QLatin1Char('#'))) {
                title = line.mid(level).simplified().trimmed();
            } else {
                level = line.startsWith(QStringLiteral("cap"), Qt::CaseInsensitive) ? 2 : 1;
            }
            if (titulo_editorial_relevante(title)) {
                current = OutlineSection{0, qBound(1, level, 6), 0, title, {}};
                has_current = true;
            }
        } else if (has_current) {
            body.append(line);
        }
    }
    flush();

    int root_count = 0;
    int child_count = 0;
    int id = 1;
    QHash<int, int> last_id_by_level;
    for (OutlineSection &section : plan.sections) {
        section.id = id++;
        for (auto it = last_id_by_level.begin(); it != last_id_by_level.end();) {
            if (it.key() >= section.level) {
                it = last_id_by_level.erase(it);
            } else {
                ++it;
            }
        }
        int parent_level = section.level - 1;
        while (parent_level > 0 && !last_id_by_level.contains(parent_level)) {
            --parent_level;
        }
        section.parent_id = parent_level > 0 ? last_id_by_level.value(parent_level) : 0;
        if (section.parent_id == 0) {
            section.level = 1;
        }
        last_id_by_level.insert(section.level, section.id);
        plan.max_depth = qMax(plan.max_depth, section.level);
        if (section.level == 1) {
            ++root_count;
        } else {
            ++child_count;
        }
    }
    plan.usable = root_count >= 1 && child_count >= 1 && plan.sections.size() <= 240;
    return plan;
}

QString dossie_secoes_documento(const QString &texto, int max_input_chars)
{
    struct Section {
        QString heading;
        QString body;
    };

    QList<Section> sections;
    QString current_heading;
    QStringList current_body;
    const auto flush = [&]() {
        const QString body = current_body.join(QLatin1Char(' ')).simplified().trimmed();
        if (!current_heading.isEmpty() && !body.isEmpty()) {
            sections.append(Section{current_heading, body});
        }
        current_body.clear();
    };

    const QStringList lines = texto.split(QLatin1Char('\n'));
    for (const QString &raw_line : lines) {
        const QString line = raw_line.simplified().trimmed();
        if (line.isEmpty()) {
            continue;
        }
        if (line.size() <= 180 && regex_titulo_estrutural().match(line).hasMatch()) {
            flush();
            current_heading = line;
        } else if (!current_heading.isEmpty()) {
            current_body.append(line);
        }
    }
    flush();

    if (sections.size() < 4) {
        return amostra_documento(texto, max_input_chars);
    }

    constexpr int kMaxSections = 20;
    QList<Section> selected;
    const int selected_count = qMin(kMaxSections, sections.size());
    selected.reserve(selected_count);
    for (int index = 0; index < selected_count; ++index) {
        const int source_index = selected_count == 1
                                     ? 0
                                     : (index * (sections.size() - 1)) / (selected_count - 1);
        selected.append(sections.at(source_index));
    }

    const int limit = qBound(3000, max_input_chars, kMaxInputChars);
    const int excerpt_size = qBound(240, (limit - selected_count * 80) / selected_count, 720);
    QStringList dossier;
    dossier.reserve(selected.size());
    for (int index = 0; index < selected.size(); ++index) {
        const Section &section = selected.at(index);
        dossier.append(QStringLiteral("[SECAO %1/%2] %3\n%4")
                           .arg(index + 1)
                           .arg(selected.size())
                           .arg(section.heading, section.body.left(excerpt_size)));
    }
    return dossier.join(QStringLiteral("\n\n")).left(limit);
}

QString extrair_outline_documento(const QString &texto)
{
    QStringList headings;
    QString last_key;
    const QStringList lines = texto.split(QLatin1Char('\n'));
    for (const QString &line : lines) {
        const QString heading = line.simplified().trimmed();
        if (heading.isEmpty() || heading.size() > 180
            || !regex_titulo_estrutural().match(heading).hasMatch()) {
            continue;
        }
        const QString key = heading.toCaseFolded();
        if (key == last_key) {
            continue;
        }
        headings.append(heading);
        last_key = key;
    }

    constexpr int kMaxOutlineItems = 64;
    if (headings.size() > kMaxOutlineItems) {
        QStringList sampled;
        sampled.reserve(kMaxOutlineItems);
        for (int index = 0; index < kMaxOutlineItems; ++index) {
            const int source_index = (index * (headings.size() - 1)) / (kMaxOutlineItems - 1);
            sampled.append(headings.at(source_index));
        }
        headings = sampled;
    }

    QString outline = headings.join(QLatin1Char('\n'));
    return outline.left(5200);
}

QString json_schema()
{
    return QStringLiteral(R"({
  "type": "object",
  "properties": {
    "titulo": {"type": "string", "minLength": 1, "maxLength": 100},
    "resumo": {"type": "string", "minLength": 1, "maxLength": 300},
    "topicos": {
      "type": "array",
      "minItems": 5,
      "maxItems": 6,
      "items": {
        "type": "object",
        "properties": {
          "titulo": {"type": "string", "minLength": 1, "maxLength": 100},
          "resumo": {"type": "string", "minLength": 1, "maxLength": 220},
          "subtopicos": {
            "type": "array",
            "minItems": 2,
            "maxItems": 3,
            "items": {
              "type": "object",
              "properties": {
                "titulo": {"type": "string", "minLength": 1, "maxLength": 100},
                "resumo": {"type": "string", "minLength": 1, "maxLength": 180}
              },
              "required": ["titulo", "resumo"]
            }
          }
        },
        "required": ["titulo", "resumo", "subtopicos"]
      }
    }
  },
  "required": ["titulo", "resumo", "topicos"]
})");
}

QString json_schema_lote(const OutlinePlan &plan, const QList<int> &ids, bool include_root = false)
{
    QJsonObject properties;
    QJsonArray required;
    for (const int id : ids) {
        if (id < 1 || id > plan.sections.size()) {
            continue;
        }
        const OutlineSection &section = plan.sections.at(id - 1);
        const int max_length = section.level <= 1 ? 900 : (section.level == 2 ? 700 : 500);
        const QString key = QStringLiteral("s%1").arg(id);
        properties.insert(key,
                          QJsonObject{{QStringLiteral("type"), QStringLiteral("string")},
                                      {QStringLiteral("minLength"), 80},
                                      {QStringLiteral("maxLength"), max_length}});
        required.append(key);
    }
    if (include_root) {
        properties.insert(QStringLiteral("resumo_obra"),
                          QJsonObject{{QStringLiteral("type"), QStringLiteral("string")},
                                      {QStringLiteral("minLength"), 400},
                                      {QStringLiteral("maxLength"), 3000}});
        required.append(QStringLiteral("resumo_obra"));
    }
    return QString::fromUtf8(QJsonDocument(QJsonObject{
                                                {QStringLiteral("type"), QStringLiteral("object")},
                                                {QStringLiteral("properties"), properties},
                                                {QStringLiteral("required"), required},
                                            })
                                 .toJson(QJsonDocument::Compact));
}

QString json_schema_resumo_obra()
{
    QJsonObject summary_property;
    summary_property.insert(QStringLiteral("type"), QStringLiteral("string"));
    summary_property.insert(QStringLiteral("minLength"), 500);
    summary_property.insert(QStringLiteral("maxLength"), 3000);
    QJsonObject properties;
    properties.insert(QStringLiteral("resumo"), summary_property);
    QJsonObject schema;
    schema.insert(QStringLiteral("type"), QStringLiteral("object"));
    schema.insert(QStringLiteral("properties"), properties);
    schema.insert(QStringLiteral("required"), QJsonArray{QStringLiteral("resumo")});
    return QString::fromUtf8(QJsonDocument(schema).toJson(QJsonDocument::Compact));
}

QString compact_json(const QVariantList &items, int max_chars)
{
    const QByteArray json = QJsonDocument::fromVariant(items).toJson(QJsonDocument::Compact);
    return QString::fromUtf8(json).left(max_chars);
}

QString prompt_para_mapa(const QString &texto,
                         const QString &idioma,
                         const QVariantList &conceitos,
                         const QVariantList &relacoes,
                         int max_input_chars)
{
    Q_UNUSED(relacoes);
    const QString outline = extrair_outline_documento(texto);
    const int document_budget = qMax(6000, max_input_chars - outline.size() - 1200);
    return QStringLiteral(
               "Voce e um analista editorial senior. Converta o documento em um mapa mental executivo, claro e profissional.\n"
               "Retorne exclusivamente JSON valido no schema solicitado.\n"
               "Use o idioma detectado: %1.\n"
               "Regras:\n"
               "- produza uma sintese curada, nao uma transcricao do indice.\n"
               "- titulo: nome curto, correto e especifico do livro/documento.\n"
               "- consolide o conteudo em 5 ou 6 macrotemas CONCEITUAIS que representem o argumento inteiro; nao use apenas Parte, Introducao ou Epilogo.\n"
               "- para cada macrotema, crie 2 ou 3 subtopicos concretos e realmente decisivos.\n"
               "- todos os titulos de topicos e subtopicos devem ser semanticamente distintos em todo o mapa.\n"
               "- e proibido repetir ou parafrasear o mesmo conjunto de subtopicos em macrotemas diferentes.\n"
               "- vincule cada subtopico a uma secao ou ideia concreta do dossie; nunca crie conselhos genericos no imperativo.\n"
               "- use o outline global para cobrir inicio, meio e fim; elimine duplicatas de sumario e cabecalhos.\n"
               "- ignore creditos, ficha catalografica, avisos legais e indices quando nao forem parte do argumento central.\n"
               "- nao atribua autoria com base em tradutores, editores, catalogadores ou nomes isolados da ficha editorial.\n"
               "- todo no deve ter resumo concreto, informativo e especifico; nunca use frases genericas como 'topico relacionado'.\n"
               "- resumos devem ter uma ou duas frases, explicar a ideia e sua relevancia e permanecer dentro do limite do schema.\n"
               "- preserve conceitos, teses, evidencias, metodos, contrastes e conclusoes apresentados pelo autor.\n"
               "- nao invente fatos, capitulos ou conclusoes fora do documento.\n"
               "- nao use markdown, comentarios ou texto fora do JSON.\n\n"
               "OUTLINE GLOBAL EXTRAIDO DO LIVRO:\n%2\n\n"
               "TERMOS RELEVANTES (apenas pistas):\n%3\n\n"
               "DOSSIE DE SECOES DISTRIBUIDAS PELO DOCUMENTO:\n%4")
        .arg(idioma.trimmed().isEmpty() ? QStringLiteral("pt_BR") : idioma,
             outline.isEmpty() ? QStringLiteral("(outline indisponivel)") : outline,
             compact_json(conceitos, 900),
             dossie_secoes_documento(texto, document_budget));
}

QString trecho_distribuido(const QString &body, int limit)
{
    const QString cleaned = body.simplified().trimmed();
    if (cleaned.size() <= limit) {
        return cleaned;
    }
    const int segment = qMax(100, (limit - 12) / 3);
    const int middle = qMax(0, (cleaned.size() - segment) / 2);
    return QStringLiteral("%1 [...] %2 [...] %3")
        .arg(cleaned.left(segment), cleaned.mid(middle, segment), cleaned.right(segment))
        .left(limit);
}

QString caminho_secao(const OutlinePlan &plan, int id)
{
    QStringList path;
    QSet<int> visited;
    int current = id;
    while (current > 0 && current <= plan.sections.size() && !visited.contains(current)) {
        visited.insert(current);
        const OutlineSection &section = plan.sections.at(current - 1);
        path.prepend(section.title);
        current = section.parent_id;
    }
    return path.join(QStringLiteral(" > "));
}

QString prompt_para_lote_hierarquico(const OutlinePlan &plan,
                                     const QList<int> &ids,
                                     const QHash<int, QString> &summaries,
                                     const QString &idioma,
                                     int max_input_chars,
                                     bool include_root = false)
{
    QStringList items;
    const int item_budget = qBound(500, (max_input_chars - 2600) / qMax(1, ids.size()), 1500);
    for (const int id : ids) {
        if (id < 1 || id > plan.sections.size()) {
            continue;
        }
        const OutlineSection &section = plan.sections.at(id - 1);
        QStringList child_context;
        for (const OutlineSection &candidate : plan.sections) {
            int ancestor = candidate.parent_id;
            while (ancestor > 0 && ancestor != id && ancestor <= plan.sections.size()) {
                ancestor = plan.sections.at(ancestor - 1).parent_id;
            }
            if (ancestor == id && summaries.contains(candidate.id)) {
                child_context.append(QStringLiteral("- %1: %2")
                                         .arg(caminho_secao(plan, candidate.id),
                                              summaries.value(candidate.id)));
                if (child_context.size() == 12) {
                    break;
                }
            }
        }
        if (child_context.isEmpty()) {
            for (const OutlineSection &candidate : plan.sections) {
                if (candidate.parent_id == id) {
                    child_context.append(QStringLiteral("- %1: %2")
                                             .arg(candidate.title, resumo_fonte(candidate)));
                }
            }
        }
        const QString evidence = trecho_distribuido(section.body, item_budget);
        items.append(QStringLiteral(
                         "[CAMPO s%1]\nNIVEL: %2\nCAMINHO: %3\nTEXTO DA SECAO: %4\n"
                         "SINTESES DOS FILHOS: %5")
                         .arg(id)
                         .arg(section.level)
                         .arg(caminho_secao(plan, id),
                              evidence.isEmpty() ? QStringLiteral("(sem texto direto; sintetize os filhos)")
                                                 : evidence,
                              child_context.isEmpty() ? QStringLiteral("(nenhum)")
                                                      : child_context.join(QLatin1Char('\n'))));
    }

    QString prompt = QStringLiteral(
               "Voce e um analista editorial senior. Resuma fielmente cada secao solicitada no idioma %1.\n"
               "Retorne somente o objeto JSON do schema, preenchendo exatamente os campos indicados.\n"
               "Criterios obrigatorios:\n"
               "- preserve a hierarquia e nunca misture conteudo de campos diferentes.\n"
               "- diferencie tese, argumentos, conceitos, evidencias e exemplos secundarios.\n"
               "- mantenha fidelidade: nao invente autoria, fatos, conclusoes ou relacoes causais.\n"
               "- escreva com clareza, coesao, objetividade e proporcionalidade; nao faca resenha critica.\n"
               "- para nivel 1, explique funcao, ideia central, argumentos, conceitos, conclusao e conexao com a obra.\n"
               "- para nivel 2, resuma o capitulo em 1 a 3 paragrafos, incluindo funcao e conclusao parcial.\n"
               "- para nivel 3 ou maior, identifique microtema, contribuicao ao pai, conceito indispensavel e funcao logica.\n"
               "- use as sinteses dos filhos para consolidar secoes-pai sem simplesmente concatena-las.\n"
               "- evite repeticoes, frases genericas, listas soltas e transcricoes extensas.\n\n"
               "SECOES A PROCESSAR:\n%2")
                         .arg(idioma.trimmed().isEmpty() ? QStringLiteral("pt_BR") : idioma,
                              items.join(QStringLiteral("\n\n")).left(max_input_chars));
    if (include_root) {
        prompt += QStringLiteral(
                      "\n\n[CAMPO resumo_obra]\nConsolide as secoes acima em um resumo integral coeso de varios "
                      "paragrafos. Inclua tema central, tese/proposito, organizacao, argumentos, conceitos "
                      "indispensaveis e conclusao geral; explique as conexoes entre as partes.");
    }
    return prompt;
}

QString prompt_para_resumo_obra(const OutlinePlan &plan,
                                const QHash<int, QString> &summaries,
                                const QVariantList &conceitos,
                                const QString &idioma,
                                int max_input_chars)
{
    QStringList structure;
    for (const OutlineSection &section : plan.sections) {
        if (section.level == 1) {
            structure.append(QStringLiteral("- %1: %2")
                                 .arg(section.title,
                                      summaries.value(section.id, resumo_fonte(section))));
        }
    }
    return QStringLiteral(
               "Voce e um analista editorial senior. Produza o resumo integral da obra no idioma %1.\n"
               "Retorne somente JSON valido no schema. Titulo: %2.\n"
               "O resumo deve ser coeso e fiel, com varios paragrafos, e responder: tema central; tese ou proposito;"
               " organizacao da obra; argumentos principais; conceitos indispensaveis; conclusao do autor.\n"
               "Mostre como as partes se conectam, preserve proporcionalidade e separe fatos do autor de qualquer"
               " avaliacao. Nao invente autoria, dados ou conclusoes e nao escreva uma resenha.\n\n"
               "SINTESE DAS PARTES/CAPITULOS PRINCIPAIS:\n%3\n\n"
               "CONCEITOS-CHAVE EXTRAIDOS:\n%4")
        .arg(idioma.trimmed().isEmpty() ? QStringLiteral("pt_BR") : idioma,
             plan.document_title.isEmpty() ? QStringLiteral("Documento") : plan.document_title,
             structure.join(QStringLiteral("\n\n")).left(max_input_chars - 1400),
             compact_json(conceitos, 1200));
}

QString limpar_saida_llama(QString output)
{
    output.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    output.remove(QRegularExpression(QStringLiteral("(?m)^load_backend:.*\\n?")));
    output.remove(QRegularExpression(QStringLiteral("(?m)^llama_.*\\n?")));
    output.remove(QRegularExpression(QStringLiteral("(?m)^main:.*\\n?")));
    output.remove(QRegularExpression(QStringLiteral("(?m)^system_info:.*\\n?")));
    output.remove(QRegularExpression(QStringLiteral("(?m)^sampler seed:.*\\n?")));
    output.remove(QRegularExpression(QStringLiteral("(?m)^generate:.*\\n?")));
    output.remove(QRegularExpression(QStringLiteral("(?m)^prompt eval.*\\n?")));
    output.remove(QRegularExpression(QStringLiteral("(?m)^eval time.*\\n?")));
    output.remove(QRegularExpression(QStringLiteral("(?m)^total time.*\\n?")));

    const int first = output.indexOf(QLatin1Char('{'));
    const int last = output.lastIndexOf(QLatin1Char('}'));
    if (first >= 0 && last > first) {
        output = output.mid(first, last - first + 1);
    }
    return output.trimmed();
}

QVariantMap normalizar_item(const QVariantMap &item, int nivel)
{
    QVariantMap normalized;
    QString titulo = item.value(QStringLiteral("titulo")).toString().simplified().trimmed();
    if (titulo.isEmpty()) {
        titulo = nivel == 0 ? QStringLiteral("Mapa Mental") : QStringLiteral("Conceito");
    }

    normalized.insert(QStringLiteral("titulo"), titulo.left(100));
    normalized.insert(QStringLiteral("nivel"), nivel);

    QVariantList filhos;
    const QVariantList raw_children = item.value(QStringLiteral("filhos")).toList();
    const int max_children = nivel == 0 ? 6 : (nivel == 1 ? 3 : 0);
    for (const QVariant &raw_child : raw_children) {
        if (filhos.size() >= max_children) {
            break;
        }
        const QVariantMap child = raw_child.toMap();
        if (!child.isEmpty()) {
            filhos.append(normalizar_item(child, nivel + 1));
        }
    }
    normalized.insert(QStringLiteral("filhos"), filhos);

    QString resumo = item.value(QStringLiteral("resumo")).toString().simplified().trimmed();
    if (resumo.isEmpty()) {
        QStringList child_summaries;
        for (const QVariant &child_value : filhos) {
            const QVariantMap child = child_value.toMap();
            const QString child_summary = child.value(QStringLiteral("resumo")).toString().simplified().trimmed();
            const QString child_title = child.value(QStringLiteral("titulo")).toString().simplified().trimmed();
            if (!child_summary.isEmpty()) {
                child_summaries.append(child_summary.left(180));
            } else if (!child_title.isEmpty()) {
                child_summaries.append(child_title.left(90));
            }
            if (child_summaries.size() >= 4) {
                break;
            }
        }

        if (!child_summaries.isEmpty()) {
            resumo = QStringLiteral("Este topico organiza: %1.").arg(child_summaries.join(QStringLiteral(" ")));
        } else {
            resumo = QStringLiteral("Topico do documento relacionado a %1.").arg(titulo.left(100));
        }
    }
    const int summary_limit = nivel == 0 ? 300 : (nivel == 1 ? 220 : 180);
    normalized.insert(QStringLiteral("resumo"), resumo.left(summary_limit));
    return normalized;
}

QVariantMap normalizar_mapa_compacto(const QVariantMap &raw_root)
{
    QVariantMap legacy_root;
    legacy_root.insert(QStringLiteral("titulo"), raw_root.value(QStringLiteral("titulo")));
    legacy_root.insert(QStringLiteral("resumo"), raw_root.value(QStringLiteral("resumo")));

    QVariantList legacy_topics;
    const QVariantList raw_topics = raw_root.value(QStringLiteral("topicos")).toList();
    for (const QVariant &raw_topic_value : raw_topics) {
        if (legacy_topics.size() >= 6) {
            break;
        }

        const QVariantMap raw_topic = raw_topic_value.toMap();
        if (raw_topic.isEmpty()) {
            continue;
        }

        QVariantMap legacy_topic;
        legacy_topic.insert(QStringLiteral("titulo"), raw_topic.value(QStringLiteral("titulo")));
        legacy_topic.insert(QStringLiteral("resumo"), raw_topic.value(QStringLiteral("resumo")));

        QVariantList legacy_subtopics;
        const QVariantList raw_subtopics = raw_topic.value(QStringLiteral("subtopicos")).toList();
        for (const QVariant &raw_subtopic_value : raw_subtopics) {
            if (legacy_subtopics.size() >= 3) {
                break;
            }

            const QVariantMap raw_subtopic = raw_subtopic_value.toMap();
            if (raw_subtopic.isEmpty()) {
                continue;
            }

            QVariantMap legacy_subtopic;
            legacy_subtopic.insert(QStringLiteral("titulo"),
                                   raw_subtopic.value(QStringLiteral("titulo")));
            legacy_subtopic.insert(QStringLiteral("resumo"),
                                   raw_subtopic.value(QStringLiteral("resumo")));
            legacy_subtopic.insert(QStringLiteral("filhos"), QVariantList{});
            legacy_subtopics.append(legacy_subtopic);
        }

        legacy_topic.insert(QStringLiteral("filhos"), legacy_subtopics);
        legacy_topics.append(legacy_topic);
    }

    legacy_root.insert(QStringLiteral("filhos"), legacy_topics);
    return normalizar_item(legacy_root, 0);
}

QString resumo_fonte(const OutlineSection &section)
{
    QString theme = section.title;
    theme.remove(QRegularExpression(QStringLiteral("^CAP[IÍ]TULO\\s+\\d+\\s*[/.:–—-]?\\s*"),
                                    QRegularExpression::CaseInsensitiveOption));
    theme = theme.simplified().trimmed();
    if (theme.isEmpty()) {
        return {};
    }

    const QString key = theme.toCaseFolded();
    if (key.startsWith(QStringLiteral("introdu"))) {
        return QStringLiteral("Apresenta o contexto, a perspectiva e as questões centrais que orientam a análise desenvolvida ao longo do documento.");
    }
    if (key.startsWith(QStringLiteral("epílogo"))) {
        return QStringLiteral("Consolida as conclusões do documento e relaciona seus principais aprendizados às perspectivas apresentadas para o futuro.");
    }
    return QStringLiteral("Examina %1 no contexto do argumento central, conectando o tema às experiências, evidências e implicações discutidas nesta seção.")
        .arg(theme.toLower());
}

int id_secao_mencionada(const QString &summary, const OutlinePlan &plan)
{
    const QString normalized = summary.toCaseFolded();
    const QRegularExpression chapter_re(QStringLiteral("cap[ií]tulo\\s*(\\d{1,3})"),
                                        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch chapter_match = chapter_re.match(normalized);
    if (chapter_match.hasMatch()) {
        const int chapter = chapter_match.captured(1).toInt();
        for (const OutlineSection &section : plan.sections) {
            const QRegularExpressionMatch title_match = chapter_re.match(section.title);
            if (title_match.hasMatch() && title_match.captured(1).toInt() == chapter) {
                return section.id;
            }
        }
    }

    const QRegularExpression part_re(QStringLiteral("parte\\s+([ivxlcdm]+|\\d+)"),
                                     QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch part_match = part_re.match(normalized);
    if (part_match.hasMatch()) {
        const QString part = part_match.captured(1).toCaseFolded();
        for (const OutlineSection &section : plan.sections) {
            const QRegularExpressionMatch title_match = part_re.match(section.title);
            if (title_match.hasMatch() && title_match.captured(1).toCaseFolded() == part) {
                return section.id;
            }
        }
    }

    for (const OutlineSection &section : plan.sections) {
        const QString title = section.title.toCaseFolded();
        if ((title.contains(QStringLiteral("introdu")) && normalized.startsWith(QStringLiteral("introdu")))
            || (title.contains(QStringLiteral("epílogo"))
                && normalized.startsWith(QStringLiteral("epílogo")))) {
            return section.id;
        }
    }

    for (const OutlineSection &section : plan.sections) {
        QString theme = section.title.toCaseFolded();
        theme.remove(QRegularExpression(QStringLiteral("^cap[ií]tulo\\s+\\d+\\s*[/.:–—-]?\\s*"),
                                        QRegularExpression::CaseInsensitiveOption));
        theme.replace(QRegularExpression(QStringLiteral("[^\\p{L}\\p{N}]+")), QStringLiteral(" "));
        theme = theme.simplified().trimmed();
        QString comparable = normalized;
        comparable.replace(QRegularExpression(QStringLiteral("[^\\p{L}\\p{N}]+")),
                           QStringLiteral(" "));
        if (theme.size() >= 8 && comparable.contains(theme)) {
            return section.id;
        }
    }
    return 0;
}

QString limpar_resumo_ia(QString summary, int summary_limit = 900)
{
    summary.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    QStringList paragraphs;
    for (const QString &paragraph : summary.split(QRegularExpression(QStringLiteral("\n+")))) {
        const QString cleaned = paragraph.simplified().trimmed();
        if (!cleaned.isEmpty()) {
            paragraphs.append(cleaned);
        }
    }
    summary = paragraphs.join(QStringLiteral("\n\n")).trimmed();
    summary.remove(QRegularExpression(
        QStringLiteral("^Cap[ií]tulo\\s+\\d+\\s*:[^-–—]{1,100}[-–—]\\s*"),
        QRegularExpression::CaseInsensitiveOption));
    summary = summary.trimmed();
    if (summary.size() > summary_limit) {
        const QString candidate = summary.left(summary_limit);
        const int sentence_end = candidate.lastIndexOf(
            QRegularExpression(QStringLiteral("[.!?](?:\\s|$)")));
        summary = sentence_end >= 80 ? candidate.left(sentence_end + 1) : candidate;
    }
    return summary;
}

int incorporar_resumos_lote(const OutlinePlan &plan,
                            const QList<int> &ids,
                            const QVariantMap &raw,
                            QHash<int, QString> &summaries)
{
    QSet<int> allowed(ids.cbegin(), ids.cend());
    QSet<QString> used_summaries;
    for (auto it = summaries.cbegin(); it != summaries.cend(); ++it) {
        used_summaries.insert(it.value().toCaseFolded());
    }
    int accepted = 0;
    for (const int id : ids) {
        QString summary = raw.value(QStringLiteral("s%1").arg(id)).toString().trimmed();
        int target_id = id_secao_mencionada(summary, plan);
        if (target_id == 0 || !allowed.contains(target_id)) {
            target_id = id;
        }
        const QString section_title = plan.sections.at(target_id - 1).title.simplified().toCaseFolded();
        const bool only_title = summary.toCaseFolded() == section_title
                                || (summary.size() <= section_title.size() + 16
                                    && summary.toCaseFolded().contains(section_title));
        const int level = plan.sections.at(target_id - 1).level;
        const int limit = level <= 1 ? 900 : (level == 2 ? 700 : 500);
        summary = limpar_resumo_ia(summary, limit);
        const QString key = summary.toCaseFolded();
        const int word_count = summary.split(QLatin1Char(' '), Qt::SkipEmptyParts).size();
        const int minimum_words = level <= 2 ? 12 : 8;
        if (!only_title && summary.size() >= 60 && word_count >= minimum_words
            && !used_summaries.contains(key)) {
            const QString existing = summaries.value(target_id);
            if (existing.isEmpty() || summary.size() > existing.size()) {
                summaries.insert(target_id, summary);
                ++accepted;
            }
            used_summaries.insert(key);
        }
    }
    return accepted;
}

QString resumo_hierarquico_fallback(const OutlinePlan &plan,
                                    int id,
                                    const QHash<int, QString> &summaries)
{
    if (id < 1 || id > plan.sections.size()) {
        return {};
    }
    QStringList children;
    for (const OutlineSection &candidate : plan.sections) {
        if (candidate.parent_id == id) {
            const QString child_summary = summaries.value(candidate.id, resumo_fonte(candidate));
            if (!child_summary.isEmpty()) {
                children.append(child_summary);
            }
            if (children.size() == 3) {
                break;
            }
        }
    }
    if (!children.isEmpty()) {
        return QStringLiteral("Esta seção consolida os seguintes eixos do argumento: %1")
            .arg(children.join(QStringLiteral(" ")))
            .left(1000);
    }
    return resumo_fonte(plan.sections.at(id - 1));
}

QVariantMap construir_no_hierarquico(const OutlinePlan &plan,
                                     int id,
                                     const QHash<int, QString> &summaries,
                                     QSet<int> &visiting)
{
    if (id < 1 || id > plan.sections.size() || visiting.contains(id)) {
        return {};
    }
    visiting.insert(id);
    const OutlineSection &section = plan.sections.at(id - 1);
    QVariantList children;
    for (const OutlineSection &candidate : plan.sections) {
        if (candidate.parent_id == id) {
            const QVariantMap child = construir_no_hierarquico(plan, candidate.id, summaries, visiting);
            if (!child.isEmpty()) {
                children.append(child);
            }
        }
    }
    visiting.remove(id);

    QVariantMap node;
    node.insert(QStringLiteral("titulo"), section.title.left(160));
    node.insert(QStringLiteral("nivel"), section.level);
    node.insert(QStringLiteral("filhos"), children);
    node.insert(QStringLiteral("resumo"),
                summaries.value(id, resumo_hierarquico_fallback(plan, id, summaries)));
    return node;
}

QVariantMap construir_resultado_hierarquico(const OutlinePlan &plan,
                                            const QHash<int, QString> &summaries,
                                            QString root_summary,
                                            int accepted_summaries,
                                            int batch_count)
{
    QVariantList root_children;
    QSet<int> visiting;
    for (const OutlineSection &section : plan.sections) {
        if (section.parent_id == 0) {
            const QVariantMap child = construir_no_hierarquico(plan, section.id, summaries, visiting);
            if (!child.isEmpty()) {
                root_children.append(child);
            }
        }
    }

    if (root_summary.split(QLatin1Char(' '), Qt::SkipEmptyParts).size() < 40) {
        QStringList overview;
        for (const QVariant &child_value : root_children) {
            const QString child_summary = child_value.toMap().value(QStringLiteral("resumo")).toString();
            if (!child_summary.isEmpty()) {
                overview.append(child_summary);
            }
        }
        root_summary = overview.join(QStringLiteral("\n\n")).left(3000);
    }
    QVariantMap root;
    root.insert(QStringLiteral("titulo"), plan.document_title.left(160));
    root.insert(QStringLiteral("nivel"), 0);
    root.insert(QStringLiteral("filhos"), root_children);
    root.insert(QStringLiteral("resumo"), root_summary.left(3000));
    root.insert(QStringLiteral("llama_outline_mode"), true);
    root.insert(QStringLiteral("llama_hierarchical_mode"), true);
    root.insert(QStringLiteral("llama_summary_count"), accepted_summaries);
    root.insert(QStringLiteral("llama_summary_total"), plan.sections.size());
    root.insert(QStringLiteral("llama_batch_count"), batch_count);
    root.insert(QStringLiteral("llama_max_depth"), plan.max_depth);
    return root;
}

QVariantList normalizar_conceitos(const QVariantList &items, const QVariantList &fallback)
{
    QVariantList result;
    QSet<QString> seen;
    for (const QVariant &value : items) {
        const QVariantMap raw = value.toMap();
        const QString texto = raw.value(QStringLiteral("texto")).toString().simplified().trimmed().left(100);
        const QString key = texto.toCaseFolded();
        if (texto.isEmpty() || seen.contains(key)) {
            continue;
        }

        bool ok = false;
        double peso = raw.value(QStringLiteral("relevancia")).toDouble(&ok);
        if (!ok) {
            peso = raw.value(QStringLiteral("peso"), 0.5).toDouble();
        }
        QVariantMap concept_item;
        concept_item.insert(QStringLiteral("texto"), texto);
        concept_item.insert(QStringLiteral("peso"), qBound(0.0, peso, 1.0));
        result.append(concept_item);
        seen.insert(key);
        if (result.size() >= 20) {
            break;
        }
    }
    return result.isEmpty() ? fallback : result;
}

QVariantList normalizar_relacoes(const QVariantList &items, const QVariantList &fallback)
{
    QVariantList result;
    QSet<QString> seen;
    for (const QVariant &value : items) {
        const QVariantMap raw = value.toMap();
        const QString origem = raw.value(QStringLiteral("origem")).toString().simplified().trimmed().left(100);
        const QString destino = raw.value(QStringLiteral("destino")).toString().simplified().trimmed().left(100);
        QString tipo = raw.value(QStringLiteral("tipo")).toString().simplified().trimmed().left(60);
        if (origem.isEmpty() || destino.isEmpty() || origem.compare(destino, Qt::CaseInsensitive) == 0) {
            continue;
        }
        if (tipo.isEmpty()) {
            tipo = QStringLiteral("associado");
        }
        const QString key = origem.toCaseFolded() + QLatin1Char('|') + destino.toCaseFolded()
                            + QLatin1Char('|') + tipo.toCaseFolded();
        if (seen.contains(key)) {
            continue;
        }

        QVariantMap relation;
        relation.insert(QStringLiteral("origem"), origem);
        relation.insert(QStringLiteral("destino"), destino);
        relation.insert(QStringLiteral("tipo"), tipo);
        result.append(relation);
        seen.insert(key);
        if (result.size() >= 16) {
            break;
        }
    }
    return result.isEmpty() ? fallback : result;
}

struct JsonInferenceResult {
    QVariantMap data;
    bool normal_process = false;
};

JsonInferenceResult executar_json_llama(const LlamaRuntime &runtime,
                                        const QString &model,
                                        const QString &prompt,
                                        const QString &schema,
                                        int context_size,
                                        int predict_tokens,
                                        int seed)
{
    QTemporaryDir temp_dir;
    if (!temp_dir.isValid()) {
        return {};
    }
    const QString prompt_path = QDir(temp_dir.path()).filePath(QStringLiteral("mapamental_prompt.txt"));
    const QString schema_path = QDir(temp_dir.path()).filePath(QStringLiteral("mapamental_schema.json"));
    QFile prompt_file(prompt_path);
    QFile schema_file(schema_path);
    if (!prompt_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)
        || !schema_file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        return {};
    }
    QTextStream prompt_stream(&prompt_file);
    prompt_stream.setEncoding(QStringConverter::Utf8);
    prompt_stream << prompt;
    prompt_file.close();
    QTextStream schema_stream(&schema_file);
    schema_stream.setEncoding(QStringConverter::Utf8);
    schema_stream << schema;
    schema_file.close();

    const int thread_count = qMax(1, QThread::idealThreadCount() - 1);
    QStringList args;
    args << QStringLiteral("--model") << model << QStringLiteral("--file") << prompt_path
         << QStringLiteral("--json-schema-file") << schema_path << QStringLiteral("--ctx-size")
         << QString::number(context_size) << QStringLiteral("--predict")
         << QString::number(predict_tokens) << QStringLiteral("--threads")
         << QString::number(thread_count) << QStringLiteral("--threads-batch")
         << QString::number(thread_count) << QStringLiteral("--batch-size") << QStringLiteral("1024")
         << QStringLiteral("--ubatch-size") << QStringLiteral("256") << QStringLiteral("--temp")
         << QStringLiteral("0.10") << QStringLiteral("--top-p") << QStringLiteral("0.85")
         << QStringLiteral("--seed") << QString::number(seed) << QStringLiteral("--gpu-layers")
         << (runtime.gpu ? QStringLiteral("auto") : QStringLiteral("0"))
         << QStringLiteral("--flash-attn") << QStringLiteral("auto") << QStringLiteral("--no-warmup")
         << QStringLiteral("--single-turn") << QStringLiteral("--no-display-prompt")
         << QStringLiteral("--no-show-timings") << QStringLiteral("--simple-io");

    QProcess process;
    process.setProgram(runtime.cli);
    process.setArguments(args);
    process.setWorkingDirectory(QFileInfo(runtime.cli).absolutePath());
    process.setProcessEnvironment(runtime_environment(QFileInfo(runtime.cli).absolutePath()));
    process.setProcessChannelMode(QProcess::SeparateChannels);
    process.start();
    if (!process.waitForStarted(10000)) {
        return {};
    }

    bool timed_out = false;
    QEventLoop wait_loop;
    QTimer timeout_timer;
    timeout_timer.setSingleShot(true);
    QObject::connect(&process,
                     qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
                     &wait_loop,
                     &QEventLoop::quit);
    QObject::connect(&process, &QProcess::errorOccurred, &wait_loop, [&wait_loop](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart || error == QProcess::Crashed) {
            wait_loop.quit();
        }
    });
    QObject::connect(&timeout_timer, &QTimer::timeout, &wait_loop, [&]() {
        timed_out = true;
        process.kill();
        wait_loop.quit();
    });
    if (process.state() != QProcess::NotRunning) {
        timeout_timer.start(env_int(QStringLiteral("MAPAMENTAL_LLAMA_TIMEOUT_MS"),
                                    kDefaultTimeoutMs,
                                    30000,
                                    900000));
        wait_loop.exec();
    }
    timeout_timer.stop();
    if (process.state() != QProcess::NotRunning) {
        process.kill();
        process.waitForFinished(5000);
    }
    if (timed_out || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        return {};
    }

    JsonInferenceResult result;
    result.normal_process = true;
    const QString output = limpar_saida_llama(QString::fromUtf8(process.readAllStandardOutput()));
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(output.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError && document.isObject()) {
        result.data = document.object().toVariantMap();
    }
    return result;
}

QVariantMap executar_llama_com_runtime(const LlamaRuntime &runtime,
                                       const QString &model,
                                       const QString &texto,
                                       const QString &idioma,
                                       const QVariantList &conceitos,
                                       const QVariantList &relacoes,
                                       bool *retry_next_runtime)
{
    if (retry_next_runtime != nullptr) {
        *retry_next_runtime = true;
    }
    if (runtime.cli.isEmpty() || model.isEmpty()) {
        return {};
    }

    const int context_size = env_int(QStringLiteral("MAPAMENTAL_LLAMA_CONTEXT"), 8192, 4096, 32768);
    const int configured_predict = env_int(QStringLiteral("MAPAMENTAL_LLAMA_PREDICT"), 2000, 512, 4096);
    const int predict_tokens = qMin(configured_predict, context_size / 2);
    const int max_input_chars = qBound(3000, (context_size - predict_tokens - 1800) * 3, kMaxInputChars);
    const OutlinePlan outline_plan = construir_plano_outline(texto);
    const int seed = env_int(QStringLiteral("MAPAMENTAL_LLAMA_SEED"), 7, 0, 2147483647);
    QVariantMap root;
    QVariantMap raw_root;
    bool any_normal_process = false;

    if (outline_plan.usable) {
        QHash<int, QString> summaries;
        int accepted_summaries = 0;
        int batch_count = 0;
        const int batch_size = env_int(QStringLiteral("MAPAMENTAL_LLAMA_HIERARCHICAL_BATCH"),
                                       14,
                                       4,
                                       24);
        if (outline_plan.max_depth > 2 && outline_plan.sections.size() <= batch_size) {
            QList<int> all_ids;
            for (const OutlineSection &section : outline_plan.sections) {
                all_ids.append(section.id);
            }
            const JsonInferenceResult inference = executar_json_llama(
                runtime,
                model,
                prompt_para_lote_hierarquico(
                    outline_plan, all_ids, summaries, idioma, max_input_chars, false),
                json_schema_lote(outline_plan, all_ids, false),
                context_size,
                qMin(predict_tokens, 2000),
                seed + batch_count);
            ++batch_count;
            any_normal_process = inference.normal_process;
            if (!inference.data.isEmpty()) {
                accepted_summaries +=
                    incorporar_resumos_lote(outline_plan, all_ids, inference.data, summaries);
            }
        } else {
            for (int level = outline_plan.max_depth; level >= 1; --level) {
                QList<int> level_ids;
                for (const OutlineSection &section : outline_plan.sections) {
                    if (section.level == level) {
                        level_ids.append(section.id);
                    }
                }
                for (int offset = 0; offset < level_ids.size(); offset += batch_size) {
                    const QList<int> ids = level_ids.mid(offset, batch_size);
                    const JsonInferenceResult inference = executar_json_llama(
                        runtime,
                        model,
                        prompt_para_lote_hierarquico(
                            outline_plan, ids, summaries, idioma, max_input_chars, false),
                        json_schema_lote(outline_plan, ids, false),
                        context_size,
                        qMin(predict_tokens, 2000),
                        seed + batch_count);
                    ++batch_count;
                    any_normal_process = any_normal_process || inference.normal_process;
                    if (!inference.data.isEmpty()) {
                        accepted_summaries +=
                            incorporar_resumos_lote(outline_plan, ids, inference.data, summaries);
                    }
                }
            }
        }

        QString root_summary;
        const JsonInferenceResult root_inference = executar_json_llama(
            runtime,
            model,
            prompt_para_resumo_obra(outline_plan, summaries, conceitos, idioma, max_input_chars),
            json_schema_resumo_obra(),
            context_size,
            qMin(predict_tokens, 1800),
            seed + batch_count);
        ++batch_count;
        any_normal_process = any_normal_process || root_inference.normal_process;
        if (!root_inference.data.isEmpty()) {
            root_summary = limpar_resumo_ia(
                root_inference.data.value(QStringLiteral("resumo")).toString(), 3000);
        }
        if (!any_normal_process) {
            return {};
        }
        root = construir_resultado_hierarquico(
            outline_plan, summaries, root_summary, accepted_summaries, batch_count);
    } else {
        const JsonInferenceResult inference = executar_json_llama(
            runtime,
            model,
            prompt_para_mapa(texto, idioma, conceitos, relacoes, max_input_chars),
            json_schema(),
            context_size,
            predict_tokens,
            seed);
        any_normal_process = inference.normal_process;
        if (inference.data.isEmpty()) {
            if (retry_next_runtime != nullptr) {
                *retry_next_runtime = !inference.normal_process;
            }
            return {};
        }
        raw_root = inference.data;
        root = raw_root.contains(QStringLiteral("topicos")) ? normalizar_mapa_compacto(raw_root)
                                                            : normalizar_item(raw_root, 0);
    }

    if (retry_next_runtime != nullptr) {
        *retry_next_runtime = !any_normal_process;
    }
    root.insert(QStringLiteral("conceitos"),
                normalizar_conceitos(raw_root.value(QStringLiteral("conceitos")).toList(), conceitos));
    root.insert(QStringLiteral("relacoes"),
                normalizar_relacoes(raw_root.value(QStringLiteral("relacoes")).toList(), relacoes));
    root.insert(QStringLiteral("llama_runtime"), runtime.label);
    root.insert(QStringLiteral("llama_backend"), runtime.gpu ? QStringLiteral("vulkan") : QStringLiteral("cpu"));
    root.insert(QStringLiteral("llama_model"), QFileInfo(model).fileName());
    return root;
}

QVariantMap executar_llama(const QString &texto,
                           const QString &idioma,
                           const QVariantList &conceitos,
                           const QVariantList &relacoes,
                           const QString &custom_model_path = QString())
{
    const QString model = !custom_model_path.trimmed().isEmpty() ? custom_model_path : descobrir_modelo();
    if (model.isEmpty() || !QFile::exists(model)) {
        return {};
    }

    const QList<LlamaRuntime> runtimes = descobrir_runtimes();
    for (const LlamaRuntime &runtime : runtimes) {
        bool retry_next_runtime = true;
        QVariantMap result = executar_llama_com_runtime(
            runtime, model, texto, idioma, conceitos, relacoes, &retry_next_runtime);
        if (!result.isEmpty()) {
            return result;
        }
        if (!retry_next_runtime) {
            break;
        }
    }

    return {};
}

}  // namespace

QVariantMap diagnosticar_llama_local()
{
    QVariantMap result;
    const QString model = descobrir_modelo();
    result.insert(QStringLiteral("modelo_disponivel"), !model.isEmpty());
    result.insert(QStringLiteral("modelo"), QFileInfo(model).fileName());
    result.insert(QStringLiteral("caminho_modelo"), model);
    result.insert(QStringLiteral("caminho_esperado"), caminho_destino_modelo_gguf());
    result.insert(QStringLiteral("tamanho_esperado"), tamanho_esperado_modelo_gguf());
    result.insert(QStringLiteral("url_download"), url_download_modelo_gguf());

    QVariantList runtime_items;
    if (!model.isEmpty()) {
        const QList<LlamaRuntime> runtimes = descobrir_runtimes();
        for (const LlamaRuntime &runtime : runtimes) {
            QVariantMap item;
            item.insert(QStringLiteral("nome"), runtime.label);
            item.insert(QStringLiteral("backend"), runtime.gpu ? QStringLiteral("vulkan") : QStringLiteral("cpu"));
            item.insert(QStringLiteral("executavel"), runtime.cli);
            runtime_items.append(item);
        }
    }
    result.insert(QStringLiteral("runtimes"), runtime_items);
    result.insert(QStringLiteral("runtime_disponivel"), !runtime_items.isEmpty());
    return result;
}

QVariantMap analisar_estrutura_com_llama(const QString &texto,
                                         const QString &idioma,
                                         const QVariantList &conceitos,
                                         const QVariantList &relacoes,
                                         const QString &custom_model_path)
{
    if (env_enabled(QStringLiteral("MAPAMENTAL_LLAMA_DISABLE"))) {
        return {};
    }

    QVariantMap raiz = executar_llama(texto, idioma, conceitos, relacoes, custom_model_path);
    if (raiz.isEmpty()) {
        return {};
    }

    raiz.insert(QStringLiteral("idioma"), idioma);
    raiz.insert(QStringLiteral("motor_ia"), QStringLiteral("llama.cpp"));
    return raiz;
}

QString diretorio_modelos_persistente()
{
    const QString persistent_dir = source::utils::obter_caminho_persistente();
    return QDir(persistent_dir).filePath(QStringLiteral("models"));
}

QString caminho_destino_modelo_gguf()
{
    return QDir(diretorio_modelos_persistente()).filePath(nome_arquivo_modelo_gguf());
}

QString nome_arquivo_modelo_gguf()
{
    return QStringLiteral("qwen2.5-3b-instruct-q4_k_m.gguf");
}

QString url_download_modelo_gguf()
{
    return QStringLiteral("https://huggingface.co/Qwen/Qwen2.5-3B-Instruct-GGUF/resolve/main/qwen2.5-3b-instruct-q4_k_m.gguf");
}

qint64 tamanho_esperado_modelo_gguf()
{
    return 2104932768LL;
}

bool modelo_gguf_instalado()
{
    const QString model = descobrir_modelo();
    if (model.isEmpty()) {
        return false;
    }
    const QFileInfo info(model);
    return valid_gguf_file(info);
}

qint64 tamanho_modelo_gguf_instalado()
{
    const QString model = descobrir_modelo();
    if (model.isEmpty()) {
        return 0;
    }
    return QFileInfo(model).size();
}

bool remover_modelo_gguf_instalado()
{
    const QString dest = caminho_destino_modelo_gguf();
    const QString part = dest + QStringLiteral(".part");
    bool removed = false;
    if (QFile::exists(dest)) {
        removed = QFile::remove(dest) || removed;
    }
    if (QFile::exists(part)) {
        QFile::remove(part);
    }

    const QString dir = diretorio_modelos_persistente();
    QDir models_dir(dir);
    if (models_dir.exists()) {
        const QStringList files = models_dir.entryList({QStringLiteral("*.gguf*")}, QDir::Files);
        for (const QString &f : files) {
            if (models_dir.remove(f)) {
                removed = true;
            }
        }
    }
    return removed;
}

}  // namespace source::plugin::Modelo::ProcessadorIAFns
