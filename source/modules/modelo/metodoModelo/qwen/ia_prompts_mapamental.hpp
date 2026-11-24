#pragma once

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QRegularExpression>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <functional>

namespace source::plugin::Modelo::ProcessadorIAFns {

constexpr int kMaxInputChars = 40000;

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

struct JsonInferenceResult {
    QVariantMap data;
    bool normal_process = false;
};

using JsonInferenceExecutor = std::function<JsonInferenceResult(
    const QString &prompt, const QString &schema, int context_size, int predict_tokens, int seed)>;

inline const QRegularExpression &regex_titulo_estrutural()
{
    static const QRegularExpression expression(
        QStringLiteral("^(?:#{1,6}\\s+.+|(?:parte|part|cap[ií]tulo|chapter)\\s+[ivxlcdm0-9]+.*)$"),
        QRegularExpression::CaseInsensitiveOption);
    return expression;
}

inline bool titulo_editorial_relevante(const QString &title)
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

inline QString resumo_fonte(const OutlineSection &section)
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

inline OutlinePlan construir_plano_outline(const QString &texto)
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

inline QString amostra_documento(const QString &texto, int max_input_chars)
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

inline QString dossie_secoes_documento(const QString &texto, int max_input_chars)
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

inline QString extrair_outline_documento(const QString &texto)
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

inline QString compact_json(const QVariantList &items, int max_chars)
{
    const QByteArray json = QJsonDocument::fromVariant(items).toJson(QJsonDocument::Compact);
    return QString::fromUtf8(json).left(max_chars);
}

inline QString prompt_para_mapa(const QString &texto,
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

inline QString json_schema()
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

inline QString json_schema_lote(const OutlinePlan &plan, const QList<int> &ids, bool include_root = false)
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

inline QString json_schema_resumo_obra()
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

inline QString trecho_distribuido(const QString &body, int limit)
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

inline QString caminho_secao(const OutlinePlan &plan, int id)
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

inline QString prompt_para_lote_hierarquico(const OutlinePlan &plan,
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

inline QString prompt_para_resumo_obra(const OutlinePlan &plan,
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

inline QString limpar_saida_llama(QString output)
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

inline QVariantMap normalizar_item(const QVariantMap &item, int nivel)
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

inline QVariantMap normalizar_mapa_compacto(const QVariantMap &raw_root)
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

inline int id_secao_mencionada(const QString &summary, const OutlinePlan &plan)
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

inline QString limpar_resumo_ia(QString summary, int summary_limit = 900)
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

inline int incorporar_resumos_lote(const OutlinePlan &plan,
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

inline QString resumo_hierarquico_fallback(const OutlinePlan &plan,
                                           int id,
                                           const QHash<int, QString> &summaries)
{
    if (id < 1 || id > plan.sections.size()) {
        return {};
    }
    const OutlineSection &section = plan.sections.at(id - 1);
    QStringList child_summaries;
    for (const OutlineSection &candidate : plan.sections) {
        if (candidate.parent_id == id && summaries.contains(candidate.id)) {
            child_summaries.append(summaries.value(candidate.id));
            if (child_summaries.size() == 4) {
                break;
            }
        }
    }
    if (!child_summaries.isEmpty()) {
        return child_summaries.join(QStringLiteral(" "));
    }
    return resumo_fonte(section);
}

inline QVariantMap construir_no_hierarquico(const OutlinePlan &plan,
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

inline QVariantMap construir_resultado_hierarquico(const OutlinePlan &plan,
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

inline QVariantList normalizar_conceitos(const QVariantList &items, const QVariantList &fallback)
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

inline QVariantList normalizar_relacoes(const QVariantList &items, const QVariantList &fallback)
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
        QVariantMap rel;
        rel.insert(QStringLiteral("origem"), origem);
        rel.insert(QStringLiteral("destino"), destino);
        rel.insert(QStringLiteral("tipo"), tipo);
        result.append(rel);
        seen.insert(key);
        if (result.size() >= 30) {
            break;
        }
    }
    return result.isEmpty() ? fallback : result;
}

inline QVariantMap executar_pipeline_hierarquico(
    const QString &texto,
    const QString &idioma,
    const QVariantList &conceitos,
    const QVariantList &relacoes,
    const JsonInferenceExecutor &executor,
    bool *any_normal_process_out = nullptr,
    int context_size = 8192,
    int predict_tokens = 2000,
    int seed = 7,
    int batch_size = 14)
{
    const int max_input_chars = qBound(3000, (context_size - predict_tokens - 1800) * 3, kMaxInputChars);
    const OutlinePlan outline_plan = construir_plano_outline(texto);
    QVariantMap root;
    QVariantMap raw_root;
    bool any_normal_process = false;

    if (outline_plan.usable) {
        QHash<int, QString> summaries;
        int accepted_summaries = 0;
        int batch_count = 0;
        if (outline_plan.max_depth > 2 && outline_plan.sections.size() <= batch_size) {
            QList<int> all_ids;
            for (const OutlineSection &section : outline_plan.sections) {
                all_ids.append(section.id);
            }
            const JsonInferenceResult inference = executor(
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
                    const JsonInferenceResult inference = executor(
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
        const JsonInferenceResult root_inference = executor(
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
            if (any_normal_process_out != nullptr) {
                *any_normal_process_out = false;
            }
            return {};
        }
        root = construir_resultado_hierarquico(
            outline_plan, summaries, root_summary, accepted_summaries, batch_count);
    } else {
        const JsonInferenceResult inference = executor(
            prompt_para_mapa(texto, idioma, conceitos, relacoes, max_input_chars),
            json_schema(),
            context_size,
            predict_tokens,
            seed);
        any_normal_process = inference.normal_process;
        if (inference.data.isEmpty()) {
            if (any_normal_process_out != nullptr) {
                *any_normal_process_out = false;
            }
            return {};
        }
        raw_root = inference.data;
        root = raw_root.contains(QStringLiteral("topicos")) ? normalizar_mapa_compacto(raw_root)
                                                            : normalizar_item(raw_root, 0);
    }

    if (any_normal_process_out != nullptr) {
        *any_normal_process_out = any_normal_process;
    }
    root.insert(QStringLiteral("conceitos"),
                normalizar_conceitos(raw_root.value(QStringLiteral("conceitos")).toList(), conceitos));
    root.insert(QStringLiteral("relacoes"),
                normalizar_relacoes(raw_root.value(QStringLiteral("relacoes")).toList(), relacoes));
    return root;
}

}  // namespace source::plugin::Modelo::ProcessadorIAFns
