#include "source/modules/modelo/metodoModelo/ollama/ia_02_ollama_local.hpp"

#include <algorithm>

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QEventLoop>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSet>
#include <QStringConverter>
#include <QThread>
#include <QTimer>
#include <QUrl>

#include "source/modules/modelo/metodoModelo/qwen/ia_prompts_mapamental.hpp"
#include "source/modules/modelo/metodoModelo/qwen/ia_01_llama_local.hpp"

namespace source::plugin::Modelo::ProcessadorIAFns {
namespace {

constexpr auto kOllamaBaseUrl = "http://127.0.0.1:11434";
constexpr int kDefaultApiTimeoutMs = 300000;      // 5 minutos para inferência
constexpr int kDefaultProbeTimeoutMs = 1500;       // 1.5s para checagem de status

QString formatar_tamanho_bytes(qint64 bytes)
{
    if (bytes <= 0) {
        return QString();
    }
    const double gb = static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0);
    if (gb >= 1.0) {
        return QStringLiteral("%1 GB").arg(QString::number(gb, 'f', 1));
    }
    const double mb = static_cast<double>(bytes) / (1024.0 * 1024.0);
    return QStringLiteral("%1 MB").arg(QString::number(mb, 'f', 0));
}

QString caminho_base_ollama()
{
    const QString env = QProcessEnvironment::systemEnvironment().value(QStringLiteral("OLLAMA_MODELS")).trimmed();
    if (!env.isEmpty() && QDir(env).exists()) {
        return env;
    }
    const QString home = QDir::homePath();
    const QString default_path = QDir(home).filePath(QStringLiteral(".ollama/models"));
    return default_path;
}

bool verificar_cabecalho_gguf(const QString &caminho_blob)
{
    QFile file(caminho_blob);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    const QByteArray magic = file.read(4);
    return magic == QByteArrayLiteral("GGUF");
}

QList<OllamaModelInfo> varrer_manifests_disco()
{
    QList<OllamaModelInfo> models;
    const QString base_dir = caminho_base_ollama();
    const QString manifests_dir = QDir(base_dir).filePath(QStringLiteral("manifests"));
    const QString blobs_dir = QDir(base_dir).filePath(QStringLiteral("blobs"));

    if (!QDir(manifests_dir).exists()) {
        return models;
    }

    QDirIterator it(manifests_dir, QDir::Files | QDir::Readable, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        const QFileInfo file_info = it.fileInfo();
        const QString relative_path = QDir(manifests_dir).relativeFilePath(file_info.absoluteFilePath());

        // O padrão geralmente é: registry.ollama.ai/library/<modelo>/<tag>
        // Ou registry.ollama.ai/<autor>/<modelo>/<tag>
        QStringList parts = relative_path.split(QLatin1Char('/'), Qt::SkipEmptyParts);
        if (parts.size() < 2) {
            continue;
        }

        // Remover prefixo de registro (ex: "registry.ollama.ai")
        if (parts.first().contains(QLatin1Char('.'))) {
            parts.removeFirst();
        }

        QString model_name;
        if (parts.size() == 3 && parts.at(0) == QStringLiteral("library")) {
            model_name = QStringLiteral("%1:%2").arg(parts.at(1), parts.at(2));
        } else if (parts.size() >= 3) {
            model_name = QStringLiteral("%1/%2:%3").arg(parts.at(0), parts.at(1), parts.at(2));
        } else if (parts.size() == 2) {
            model_name = QStringLiteral("%1:%2").arg(parts.at(0), parts.at(1));
        } else {
            model_name = file_info.fileName();
        }

        QFile manifest_file(file_info.absoluteFilePath());
        if (!manifest_file.open(QIODevice::ReadOnly)) {
            continue;
        }

        const QJsonDocument doc = QJsonDocument::fromJson(manifest_file.readAll());
        if (!doc.isObject()) {
            continue;
        }

        const QJsonObject root_obj = doc.object();
        const QJsonArray layers = root_obj.value(QStringLiteral("layers")).toArray();

        QString model_digest;
        qint64 model_size = 0;
        for (const QJsonValue &layer_val : layers) {
            const QJsonObject layer = layer_val.toObject();
            if (layer.value(QStringLiteral("mediaType")).toString() == QStringLiteral("application/vnd.ollama.image.model")) {
                model_digest = layer.value(QStringLiteral("digest")).toString();
                model_size = layer.value(QStringLiteral("size")).toInteger(0);
                break;
            }
        }

        if (model_digest.isEmpty()) {
            continue;
        }

        // Digest formato: "sha256:abcd..." -> arquivo blob: "sha256-abcd..."
        QString blob_filename = model_digest;
        blob_filename.replace(QLatin1Char(':'), QLatin1Char('-'));
        const QString blob_full_path = QDir(blobs_dir).filePath(blob_filename);

        OllamaModelInfo info;
        info.name = model_name;
        info.digest = model_digest;
        info.blobPath = blob_full_path;
        info.sizeBytes = model_size > 0 ? model_size : QFileInfo(blob_full_path).size();
        info.isBlobGguf = verificar_cabecalho_gguf(blob_full_path);

        const QString size_str = formatar_tamanho_bytes(info.sizeBytes);
        info.displayName = size_str.isEmpty() ? model_name : QStringLiteral("%1 (%2)").arg(model_name, size_str);

        models.append(info);
    }

    return models;
}

QList<OllamaModelInfo> consultar_api_tags(int timeout_ms = kDefaultProbeTimeoutMs)
{
    QList<OllamaModelInfo> api_models;
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(QStringLiteral("%1/api/tags").arg(kOllamaBaseUrl)));
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);

    QNetworkReply *reply = manager.get(request);
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, [&]() {
        if (reply->isRunning()) {
            reply->abort();
        }
        loop.quit();
    });

    timer.start(timeout_ms);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return api_models;
    }

    const QByteArray response_bytes = reply->readAll();
    reply->deleteLater();

    const QJsonDocument doc = QJsonDocument::fromJson(response_bytes);
    if (!doc.isObject()) {
        return api_models;
    }

    const QJsonArray models_arr = doc.object().value(QStringLiteral("models")).toArray();
    for (const QJsonValue &val : models_arr) {
        const QJsonObject obj = val.toObject();
        OllamaModelInfo info;
        info.name = obj.value(QStringLiteral("name")).toString();
        if (info.name.isEmpty()) {
            info.name = obj.value(QStringLiteral("model")).toString();
        }
        if (info.name.isEmpty()) {
            continue;
        }

        info.sizeBytes = obj.value(QStringLiteral("size")).toInteger(0);
        info.digest = obj.value(QStringLiteral("digest")).toString();

        const QJsonObject details = obj.value(QStringLiteral("details")).toObject();
        info.family = details.value(QStringLiteral("family")).toString();
        info.parameterSize = details.value(QStringLiteral("parameter_size")).toString();

        const QString size_str = formatar_tamanho_bytes(info.sizeBytes);
        if (!info.parameterSize.isEmpty() && !size_str.isEmpty()) {
            info.displayName = QStringLiteral("%1 [%2] (%3)").arg(info.name, info.parameterSize, size_str);
        } else if (!size_str.isEmpty()) {
            info.displayName = QStringLiteral("%1 (%2)").arg(info.name, size_str);
        } else {
            info.displayName = info.name;
        }

        api_models.append(info);
    }

    return api_models;
}

JsonInferenceResult requisitar_inferencia_ollama_api(
    const QString &model_name,
    const QString &prompt,
    const QString &schema,
    int context_size,
    int predict_tokens,
    int seed,
    int timeout_ms = kDefaultApiTimeoutMs)
{
    JsonInferenceResult result;

    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(QStringLiteral("%1/api/generate").arg(kOllamaBaseUrl)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);

    QJsonObject request_obj;
    request_obj.insert(QStringLiteral("model"), model_name);
    request_obj.insert(QStringLiteral("prompt"), prompt);
    request_obj.insert(QStringLiteral("stream"), false);

    // Se temos um schema JSON, convertê-lo em objeto para o parâmetro format
    const QJsonDocument schema_doc = QJsonDocument::fromJson(schema.toUtf8());
    if (schema_doc.isObject()) {
        request_obj.insert(QStringLiteral("format"), schema_doc.object());
    } else {
        request_obj.insert(QStringLiteral("format"), QStringLiteral("json"));
    }

    QJsonObject options_obj;
    options_obj.insert(QStringLiteral("temperature"), 0.1);
    options_obj.insert(QStringLiteral("top_p"), 0.85);
    options_obj.insert(QStringLiteral("seed"), seed);
    options_obj.insert(QStringLiteral("num_predict"), predict_tokens);
    options_obj.insert(QStringLiteral("num_ctx"), context_size);
    request_obj.insert(QStringLiteral("options"), options_obj);

    const QByteArray request_body = QJsonDocument(request_obj).toJson(QJsonDocument::Compact);

    QNetworkReply *reply = manager.post(request, request_body);
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, [&]() {
        if (reply->isRunning()) {
            reply->abort();
        }
        loop.quit();
    });

    timer.start(timeout_ms);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return result;
    }

    const QByteArray response_bytes = reply->readAll();
    reply->deleteLater();

    const QJsonDocument resp_doc = QJsonDocument::fromJson(response_bytes);
    if (!resp_doc.isObject()) {
        return result;
    }

    result.normal_process = true;
    const QString response_text = resp_doc.object().value(QStringLiteral("response")).toString();
    const QString cleaned = limpar_saida_llama(response_text);

    QJsonParseError error;
    const QJsonDocument json_data = QJsonDocument::fromJson(cleaned.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError && json_data.isObject()) {
        result.data = json_data.object().toVariantMap();
    }

    return result;
}

}  // namespace

bool testar_conexao_ollama_api(int timeout_ms)
{
    QNetworkAccessManager manager;
    QNetworkRequest request(QUrl(QStringLiteral("%1/api/version").arg(kOllamaBaseUrl)));
    request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);

    QNetworkReply *reply = manager.get(request);
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, [&]() {
        if (reply->isRunning()) {
            reply->abort();
        }
        loop.quit();
    });

    timer.start(timeout_ms);
    loop.exec();

    const bool ok = reply->error() == QNetworkReply::NoError;
    reply->deleteLater();
    return ok;
}

QList<OllamaModelInfo> descobrir_modelos_ollama(bool forcar_recarga)
{
    static QList<OllamaModelInfo> cached_models;
    static bool cached = false;

    if (cached && !forcar_recarga) {
        return cached_models;
    }

    // 1. Varrer manifests locais no disco
    QList<OllamaModelInfo> disk_models = varrer_manifests_disco();
    QHash<QString, OllamaModelInfo> merged_map;
    for (const auto &info : disk_models) {
        merged_map.insert(info.name, info);
    }

    // 2. Consultar API caso o serviço esteja rodando
    QList<OllamaModelInfo> api_models = consultar_api_tags(kDefaultProbeTimeoutMs);
    for (const auto &api_info : api_models) {
        if (merged_map.contains(api_info.name)) {
            // Enriquecer informações de família e parâmetros
            auto &existing = merged_map[api_info.name];
            existing.family = api_info.family;
            existing.parameterSize = api_info.parameterSize;
            if (!api_info.displayName.isEmpty()) {
                existing.displayName = api_info.displayName;
            }
        } else {
            // Se o modelo apareceu na API mas não foi lido pelo parser de manifest,
            // tentar associar o blob
            OllamaModelInfo enriched = api_info;
            if (!enriched.digest.isEmpty()) {
                QString blob_name = enriched.digest;
                blob_name.replace(QLatin1Char(':'), QLatin1Char('-'));
                const QString base_dir = caminho_base_ollama();
                const QString blob_path = QDir(base_dir).filePath(QStringLiteral("blobs/%1").arg(blob_name));
                enriched.blobPath = blob_path;
                enriched.isBlobGguf = verificar_cabecalho_gguf(blob_path);
            }
            merged_map.insert(enriched.name, enriched);
        }
    }

    cached_models = merged_map.values();
    std::sort(cached_models.begin(), cached_models.end(), [](const OllamaModelInfo &a, const OllamaModelInfo &b) {
        return a.name.localeAwareCompare(b.name) < 0;
    });

    cached = true;
    return cached_models;
}

OllamaModelInfo obter_info_modelo_ollama(const QString &model_name)
{
    const QList<OllamaModelInfo> models = descobrir_modelos_ollama(false);
    for (const auto &model : models) {
        if (model.name.compare(model_name, Qt::CaseInsensitive) == 0) {
            return model;
        }
    }
    // Caso tenha sido instalado recentemente, forçar recarga
    const QList<OllamaModelInfo> reloaded = descobrir_modelos_ollama(true);
    for (const auto &model : reloaded) {
        if (model.name.compare(model_name, Qt::CaseInsensitive) == 0) {
            return model;
        }
    }
    return OllamaModelInfo();
}

QVariantMap diagnosticar_ollama()
{
    QVariantMap diag;
    const bool api_ok = testar_conexao_ollama_api(1000);
    const QList<OllamaModelInfo> models = descobrir_modelos_ollama(true);

    diag.insert(QStringLiteral("api_conectada"), api_ok);
    diag.insert(QStringLiteral("caminho_base"), caminho_base_ollama());
    diag.insert(QStringLiteral("total_modelos"), models.size());

    QVariantList list;
    for (const auto &m : models) {
        QVariantMap item;
        item.insert(QStringLiteral("nome"), m.name);
        item.insert(QStringLiteral("display"), m.displayName);
        item.insert(QStringLiteral("tamanho_bytes"), m.sizeBytes);
        item.insert(QStringLiteral("tamanho_formatado"), formatar_tamanho_bytes(m.sizeBytes));
        item.insert(QStringLiteral("familia"), m.family);
        item.insert(QStringLiteral("parametros"), m.parameterSize);
        item.insert(QStringLiteral("blob_gguf_valido"), m.isBlobGguf);
        item.insert(QStringLiteral("caminho_blob"), m.blobPath);
        list.append(item);
    }
    diag.insert(QStringLiteral("modelos"), list);
    return diag;
}

QVariantMap analisar_estrutura_com_ollama(const QString &model_name,
                                         const QString &texto,
                                         const QString &idioma,
                                         const QVariantList &conceitos,
                                         const QVariantList &relacoes)
{
    if (texto.trimmed().isEmpty() || model_name.trimmed().isEmpty()) {
        return {};
    }

    const OllamaModelInfo model_info = obter_info_modelo_ollama(model_name);
    const bool api_online = testar_conexao_ollama_api(1500);

    // Se a API não está respondendo e não temos o blob local GGUF, não é possível processar
    if (!api_online && (!model_info.isBlobGguf || !QFile::exists(model_info.blobPath))) {
        return {};
    }

    const int context_size = 8192;
    const int predict_tokens = 2000;
    const int seed = 7;
    const int batch_size = 14;

    bool used_api = false;
    auto executor = [&](const QString &prompt,
                        const QString &schema,
                        int ctx,
                        int predict,
                        int seed_val) -> JsonInferenceResult {
        // Tentativa primária: API REST do Ollama
        if (api_online) {
            JsonInferenceResult res = requisitar_inferencia_ollama_api(
                model_name, prompt, schema, ctx, predict, seed_val);
            if (res.normal_process && !res.data.isEmpty()) {
                used_api = true;
                return res;
            }
        }

        // Tentativa secundária / fallback: se o blob GGUF existir, usar llama-cli local
        if (model_info.isBlobGguf && QFile::exists(model_info.blobPath)) {
            // Usar analisar_estrutura_com_llama com o caminho do blob customizado
            // mas aqui precisamos de um json individual para cada lote
            // Se a API falhou, podemos tentar chamar o llama-cli com esse blob
            // O pipeline em ia_01_llama_local já suporta llama-cli
        }

        return {};
    };

    bool any_normal_process = false;
    QVariantMap raiz = executar_pipeline_hierarquico(
        texto, idioma, conceitos, relacoes, executor, &any_normal_process,
        context_size, predict_tokens, seed, batch_size);

    // Se o executor HTTP não conseguiu completar e temos o blob local GGUF,
    // fazer o fallback completo para o motor llama-cli passando o blob do modelo Ollama
    if (raiz.isEmpty() && model_info.isBlobGguf && QFile::exists(model_info.blobPath)) {
        raiz = analisar_estrutura_com_llama(texto, idioma, conceitos, relacoes, model_info.blobPath);
    }

    if (raiz.isEmpty()) {
        return {};
    }

    raiz.insert(QStringLiteral("idioma"), idioma);
    raiz.insert(QStringLiteral("motor_ia"), QStringLiteral("ollama:%1").arg(model_name));
    raiz.insert(QStringLiteral("modelo_ollama"), model_name);
    raiz.insert(QStringLiteral("metodo_inferencia"), used_api ? QStringLiteral("ollama_http_api") : QStringLiteral("llama_cli_blob"));
    raiz.insert(QStringLiteral("plugin_modelo"), true);
    return raiz;
}

}  // namespace source::plugin::Modelo::ProcessadorIAFns
