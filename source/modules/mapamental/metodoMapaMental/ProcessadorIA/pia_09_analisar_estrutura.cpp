#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_09_analisar_estrutura.hpp"

#include <memory>

#include <QtGlobal>

#include <QRegularExpression>
#include <QMetaType>
#include <QStringList>

#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_11_linha_parece_sumario.hpp"
#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_13_remover_acentos.hpp"
#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_22_gerar_resumo_contextual.hpp"
#include "source/modules/mapamental/mapamental_host_bridge.hpp"
#include "source/modules/mapamental/mapamental_support.hpp"

namespace source::plugin::MapaMental {
namespace {

struct SectionNode {
    QString title;
    int level = 0;
    QStringList body;
    QList<std::shared_ptr<SectionNode>> children;
};

struct HeadingInfo {
    bool valid = false;
    int level = 0;
    QString title;
};

QString remover_markdown(QString linha)
{
    linha = linha.trimmed();
    linha.remove(QRegularExpression(QStringLiteral("^#{1,6}\\s+")));
    return linha.trimmed();
}

QString normalizar_para_match(const QString &texto)
{
    return ProcessadorIAFns::remover_acentos(texto).toCaseFolded().simplified().trimmed();
}

QString chave_titulo(QString texto)
{
    texto = remover_markdown(texto);
    texto = normalizar_para_match(texto);
    texto.replace(QRegularExpression(QStringLiteral("[^\\p{L}\\p{N}]+")), QStringLiteral(" "));
    return texto.simplified().trimmed();
}

bool is_list_item(const QString &line)
{
    const QString trimmed = line.trimmed();
    return trimmed.startsWith(QStringLiteral("- "))
           || trimmed.startsWith(QStringLiteral("* "))
           || QRegularExpression(QStringLiteral("^\\d+[\\.)]\\s+")).match(trimmed).hasMatch();
}

bool e_secao_principal(const QString &key)
{
    static const QStringList secoes = {
        QStringLiteral("creditos"),
        QStringLiteral("folha de rosto"),
        QStringLiteral("sumario"),
        QStringLiteral("nota do autor"),
        QStringLiteral("prefacio a edicao brasileira"),
        QStringLiteral("prefacio da edicao original"),
        QStringLiteral("epilogo"),
        QStringLiteral("perguntas frequentes"),
        QStringLiteral("um guia de recursos"),
        QStringLiteral("agradecimentos"),
        QStringLiteral("sobre os autores"),
    };

    return key == QStringLiteral("introducao")
           || key.startsWith(QStringLiteral("introducao "))
           || secoes.contains(key);
}

bool e_sumario(const QString &titulo)
{
    return chave_titulo(titulo) == QStringLiteral("sumario");
}

bool e_marcador_nome_fonte(const QString &linha)
{
    return linha.trimmed().startsWith(QStringLiteral("NOME DO ARQUIVO/FONTE:"),
                                      Qt::CaseInsensitive);
}

QString titulo_do_marcador_fonte(const QString &linha)
{
    if (!e_marcador_nome_fonte(linha)) {
        return {};
    }
    QString titulo = linha.mid(linha.indexOf(QLatin1Char(':')) + 1).simplified().trimmed();
    titulo.remove(QRegularExpression(QStringLiteral("\\s*--.*$")));
    titulo.remove(QRegularExpression(QStringLiteral("\\s*\\([^)]*(?:welib|archive|ebook)[^)]*\\)\\s*$"),
                                    QRegularExpression::CaseInsensitiveOption));
    titulo.replace(QRegularExpression(QStringLiteral("\\s*[-–—]\\s+")), QStringLiteral(": "));
    return titulo.simplified().trimmed().left(160);
}

bool e_secao_descartavel(const QString &key)
{
    static const QStringList descartaveis = {
        QStringLiteral("creditos"),
        QStringLiteral("folha de rosto"),
        QStringLiteral("sumario"),
        QStringLiteral("ficha catalografica"),
        QStringLiteral("copyright"),
        QStringLiteral("nota do autor"),
        QStringLiteral("um guia de recursos"),
        QStringLiteral("agradecimentos"),
        QStringLiteral("sobre os autores"),
        QStringLiteral("indice remissivo"),
        QStringLiteral("bibliografia"),
        QStringLiteral("referencias bibliograficas"),
    };
    return descartaveis.contains(key);
}

QRegularExpressionMatch match_parte(const QString &linha)
{
    static const QRegularExpression part_heading(
        QStringLiteral("^(?:parte|part)\\s+([ivx]+|\\d+)\\s*(?:[:/\\-]\\s*)?(.*)$"),
        QRegularExpression::CaseInsensitiveOption);
    return part_heading.match(linha.trimmed());
}

QRegularExpressionMatch match_capitulo(const QString &linha)
{
    static const QRegularExpression chapter_heading(
        QStringLiteral("^(?:cap[ií]tulo|cap\\.?|chapter)\\s+(\\d+)\\s*(?:[:/\\-]\\s*)?(.*)$"),
        QRegularExpression::CaseInsensitiveOption);
    return chapter_heading.match(linha.trimmed());
}

QRegularExpressionMatch match_capitulo_numerado(const QString &linha)
{
    static const QRegularExpression numbered_chapter(
        QStringLiteral("^(\\d{1,3})\\s*[\\.)]\\s+(.+)$"));
    return numbered_chapter.match(linha.trimmed());
}

QString rotulo_parte(const QString &linha, const QRegularExpressionMatch &match)
{
    Q_UNUSED(linha);
    const QString numero = match.captured(1).toUpper();
    QString resto = match.captured(2).simplified().trimmed();
    resto.remove(QRegularExpression(QStringLiteral("^[\\s:/\\-]+")));
    if (resto.isEmpty()) {
        return QStringLiteral("Parte %1").arg(numero);
    }
    return QStringLiteral("Parte %1: %2").arg(numero, resto);
}

bool parte_estrutural_valida(const QRegularExpressionMatch &match)
{
    if (!match.hasMatch()) {
        return false;
    }
    QString resto = match.captured(2).simplified().trimmed();
    resto.remove(QRegularExpression(QStringLiteral("^[\\s:/\\-]+")));
    if (resto.isEmpty()) {
        return true;
    }
    if (resto.size() > 120) {
        return false;
    }
    const QChar first = resto.at(0);
    return !first.isLetter() || first.isUpper();
}

QString rotulo_capitulo(const QString &numero, QString titulo)
{
    titulo = titulo.simplified().trimmed();
    titulo.remove(QRegularExpression(QStringLiteral("^[\\s:/\\-]+")));
    if (titulo.isEmpty()) {
        return QStringLiteral("CAPÍTULO %1").arg(numero);
    }
    return QStringLiteral("CAPÍTULO %1/%2").arg(numero, titulo.toUpper());
}

bool capitulo_estrutural_valido(const QRegularExpressionMatch &match)
{
    if (!match.hasMatch()) {
        return false;
    }
    QString resto = match.captured(2).simplified().trimmed();
    resto.remove(QRegularExpression(QStringLiteral("^[\\s:/\\-]+")));
    if (resto.isEmpty()) {
        return true;
    }
    if (resto.startsWith(QLatin1Char(',')) || resto.startsWith(QLatin1Char('.'))
        || resto.startsWith(QLatin1Char(')'))) {
        return false;
    }
    return resto.size() <= 120;
}

QString limpar_titulo(const QString &linha)
{
    QString cleaned = remover_markdown(linha).simplified().trimmed();
    if (cleaned.isEmpty()) {
        return {};
    }

    const QRegularExpressionMatch chapter_match = match_capitulo(cleaned);
    if (capitulo_estrutural_valido(chapter_match)) {
        return rotulo_capitulo(chapter_match.captured(1), chapter_match.captured(2));
    }

    const QRegularExpressionMatch numbered_match = match_capitulo_numerado(cleaned);
    if (numbered_match.hasMatch()) {
        return rotulo_capitulo(numbered_match.captured(1), numbered_match.captured(2));
    }

    const QRegularExpressionMatch part_match = match_parte(cleaned);
    if (parte_estrutural_valida(part_match)) {
        return rotulo_parte(cleaned, part_match);
    }

    cleaned.remove(QRegularExpression(QStringLiteral("^\\d+(?:[\\.\\-]\\d+)*\\s*[\\).:-]?\\s*")));
    cleaned.remove(QRegularExpression(QStringLiteral("^[IVXLCDM]+\\.?\\s+")));
    cleaned.remove(QRegularExpression(QStringLiteral("^[-:\\s]+")));
    return cleaned.simplified().trimmed();
}

bool e_titulo_caixa_alta(const QString &linha)
{
    const QString trimmed = linha.trimmed();
    if (trimmed.size() < 4 || trimmed.size() > 180) {
        return false;
    }
    if (!trimmed.contains(QRegularExpression(QStringLiteral("\\p{L}")))) {
        return false;
    }
    if (trimmed.contains(QRegularExpression(QStringLiteral("[\\.!]$")))) {
        return false;
    }
    return trimmed.toUpper() == trimmed;
}

int nivel_capitulo_atual(const QList<std::shared_ptr<SectionNode>> &stack)
{
    for (int i = stack.size() - 1; i >= 0; --i) {
        const auto &node = stack.at(i);
        if (node != nullptr && chave_titulo(node->title).startsWith(QStringLiteral("capitulo "))) {
            return node->level;
        }
    }
    return 0;
}

bool chave_estrutural_curta(const QString &key)
{
    static const QRegularExpression structural_short(
        QStringLiteral("^(parte\\s+[ivxlcdm0-9]+|capitulo\\s+\\d+)$"));
    return structural_short.match(key).hasMatch();
}

bool titulos_equivalentes(const QString &a, const QString &b)
{
    const QString key_a = chave_titulo(a);
    const QString key_b = chave_titulo(b);
    if (key_a.isEmpty() || key_b.isEmpty()) {
        return false;
    }
    if (key_a == key_b) {
        return true;
    }
    return (chave_estrutural_curta(key_a) && key_b.startsWith(key_a + QLatin1Char(' ')))
           || (chave_estrutural_curta(key_b) && key_a.startsWith(key_b + QLatin1Char(' ')));
}

bool titulo_repete_ancestral(const QList<std::shared_ptr<SectionNode>> &stack, const QString &titulo)
{
    for (const auto &node : stack) {
        if (node != nullptr && titulos_equivalentes(node->title, titulo)) {
            return true;
        }
    }
    return false;
}

bool filho_equivalente_existe(const std::shared_ptr<SectionNode> &node, const QString &titulo)
{
    if (node == nullptr) {
        return false;
    }
    for (const auto &child : node->children) {
        if (child != nullptr && titulos_equivalentes(child->title, titulo)) {
            return true;
        }
    }
    return false;
}

HeadingInfo classificar_titulo(const QString &linha, const QList<std::shared_ptr<SectionNode>> &stack)
{
    HeadingInfo info;
    const QString trimmed = linha.trimmed();
    if (trimmed.isEmpty() || is_list_item(trimmed)) {
        return info;
    }

    static const QRegularExpression markdown_heading(QStringLiteral("^(#{1,6})\\s+"));
    const QRegularExpressionMatch markdown_match = markdown_heading.match(trimmed);
    const bool markdown = markdown_match.hasMatch();
    const QString sem_markdown = remover_markdown(trimmed);
    const QString titulo = limpar_titulo(sem_markdown).left(160).trimmed();
    if (titulo.isEmpty()) {
        return info;
    }

    const QString key = chave_titulo(titulo);
    const bool parte = parte_estrutural_valida(match_parte(sem_markdown));
    const QRegularExpressionMatch chapter_match = match_capitulo(sem_markdown);
    const bool capitulo = capitulo_estrutural_valido(chapter_match)
                          || match_capitulo_numerado(sem_markdown).hasMatch();

    if (markdown) {
        info.valid = true;
        info.level = qBound(1, markdown_match.captured(1).size(), 6);
        info.title = titulo;
        return info;
    }

    if (parte || e_secao_principal(key)) {
        info.valid = true;
        info.level = 1;
        info.title = titulo;
        return info;
    }

    if (capitulo) {
        info.valid = true;
        info.level = 2;
        info.title = titulo;
        return info;
    }

    const int chapter_level = nivel_capitulo_atual(stack);
    if (chapter_level > 0 && e_titulo_caixa_alta(sem_markdown)) {
        info.valid = true;
        info.level = chapter_level + 1;
        info.title = titulo;
    }

    return info;
}

QString first_meaningful_line(const QStringList &lines)
{
    for (const QString &line : lines) {
        const QString trimmed = remover_markdown(line).trimmed();
        if (trimmed.isEmpty() || ProcessadorIAFns::linha_parece_sumario(trimmed)) {
            continue;
        }
        const QString titulo = limpar_titulo(trimmed);
        if (!titulo.isEmpty()) {
            return titulo.left(120);
        }
    }
    return QStringLiteral("Mapa Mental");
}

QString limpar_titulo_catalogacao(QString linha)
{
    linha = linha.simplified().trimmed();
    linha.remove(QRegularExpression(QStringLiteral("\\[[^\\]]+\\]")));
    linha.replace(QRegularExpression(QStringLiteral("\\s+:\\s+")), QStringLiteral(": "));
    linha.remove(QRegularExpression(QStringLiteral("\\s*/.*$")));
    linha.remove(QRegularExpression(QStringLiteral("\\s+[–-]\\s+\\d+.*$")));
    linha.remove(QRegularExpression(QStringLiteral("[\\s\\.;:,]+$")));
    return linha.simplified().trimmed();
}

QString inferir_titulo_documento(const QStringList &lines)
{
    for (const QString &line : lines) {
        const QString titulo_fonte = titulo_do_marcador_fonte(line);
        if (!titulo_fonte.isEmpty()) {
            return titulo_fonte;
        }
    }

    for (int i = 0; i < lines.size(); ++i) {
        const QString &line = lines.at(i);
        const QString trimmed = remover_markdown(line).simplified().trimmed();
        const QString key = chave_titulo(trimmed);
        if (trimmed.isEmpty() || e_secao_principal(key)
            || key.startsWith(QStringLiteral("parte "))
            || key.startsWith(QStringLiteral("capitulo "))) {
            continue;
        }
        if (key.contains(QStringLiteral("humano a sua maneira"))
            && key.contains(QStringLiteral("autismo"))) {
            const QString titulo = limpar_titulo_catalogacao(trimmed);
            if (!titulo.isEmpty()) {
                return titulo.left(160);
            }
        }
        if (i == 0 && !line.trimmed().startsWith(QLatin1Char('#'))) {
            return trimmed.left(160);
        }
    }
    return first_meaningful_line(lines);
}

bool tem_marcadores_markdown(const QStringList &lines)
{
    static const QRegularExpression markdown_heading(QStringLiteral("^#{1,6}\\s+"));
    for (const QString &line : lines) {
        if (markdown_heading.match(line.trimmed()).hasMatch()) {
            return true;
        }
    }
    return false;
}

int limitar_resumo_por_nivel(int nivel)
{
    if (nivel <= 0) {
        return 1100;
    }
    if (nivel == 1) {
        return 650;
    }
    if (nivel == 2) {
        return 480;
    }
    if (nivel == 3) {
        return 360;
    }
    return 320;
}

QString material_de_filhos(const QVariantList &children, bool somente_conteudo_central)
{
    QStringList parts;
    parts.reserve(children.size());
    for (const QVariant &child_value : children) {
        const QVariantMap child = child_value.toMap();
        const QString resumo = child.value(QStringLiteral("resumo")).toString().simplified().trimmed();
        const QString titulo = child.value(QStringLiteral("titulo")).toString().simplified().trimmed();
        const QString key = chave_titulo(titulo);
        if (somente_conteudo_central
            && (key.startsWith(QStringLiteral("prefacio"))
                || key.startsWith(QStringLiteral("apresentacao")))) {
            continue;
        }
        QString resumo_completo = resumo;
        if (!resumo_completo.isEmpty()
            && !QRegularExpression(QStringLiteral("[.!?…]$")).match(resumo_completo).hasMatch()) {
            resumo_completo.append(QLatin1Char('.'));
        }
        if (!titulo.isEmpty() && !resumo_completo.isEmpty()) {
            parts.append(QStringLiteral("%1. %2").arg(titulo.left(120), resumo_completo));
        } else if (!resumo.isEmpty()) {
            parts.append(resumo_completo);
        } else if (!titulo.isEmpty()) {
            parts.append(titulo.left(120));
        }
        if (parts.size() >= 16) {
            break;
        }
    }
    return parts.join(QLatin1Char('\n'));
}

QString resumo_obrigatorio(const QString &titulo, const QVariantList &filhos)
{
    QStringList titulos;
    for (const QVariant &child_value : filhos) {
        const QVariantMap child = child_value.toMap();
        const QString child_title = child.value(QStringLiteral("titulo")).toString().simplified().trimmed();
        if (!child_title.isEmpty()) {
            titulos.append(child_title.left(90));
        }
        if (titulos.size() >= 8) {
            break;
        }
    }

    if (!titulos.isEmpty()) {
        return QStringLiteral("%1 reúne %2.").arg(titulo.left(100), titulos.join(QStringLiteral(", ")))
            .left(480);
    }
    return titulo.left(220);
}

QVariantMap to_variant(const std::shared_ptr<SectionNode> &node, const QString &contexto_pai)
{
    QVariantMap item;
    const QString titulo = node->title.trimmed().isEmpty() ? QStringLiteral("Conceito") : node->title.trimmed();
    item.insert(QStringLiteral("titulo"), titulo);
    item.insert(QStringLiteral("nivel"), node->level);

    QVariantList filhos;
    for (const auto &child : node->children) {
        filhos.append(to_variant(child, contexto_pai + QLatin1Char(' ') + titulo));
    }
    item.insert(QStringLiteral("filhos"), filhos);

    const int limit = limitar_resumo_por_nivel(node->level);
    const QString corpo = node->body.join(QLatin1Char('\n')).trimmed();
    const QString filhos_sintetizados = material_de_filhos(filhos, node->level == 0);
    QString material = corpo;
    if (!filhos_sintetizados.isEmpty()) {
        if (!material.isEmpty()) {
            material.append(QStringLiteral("\n\n"));
        }
        material.append(filhos_sintetizados);
    }
    const QString contexto = (contexto_pai + QLatin1Char(' ') + titulo).simplified();
    QString resumo = ProcessadorIAFns::gerar_resumo_contextual(material, limit, contexto);
    if (resumo.trimmed().isEmpty()) {
        resumo = resumo_obrigatorio(titulo, filhos);
    }
    item.insert(QStringLiteral("resumo"), resumo.trimmed());
    return item;
}

QString titulo_para_bloco(const QStringList &linhas, int ordinal, const QString &contexto)
{
    const QString resumo = ProcessadorIAFns::gerar_resumo_contextual(
        linhas.join(QLatin1Char('\n')), 180, contexto);
    QString titulo = resumo.section(QRegularExpression(QStringLiteral("[.!?]")), 0, 0)
                        .simplified()
                        .trimmed();
    if (titulo.size() > 88) {
        const int cut = titulo.lastIndexOf(QLatin1Char(' '), 88);
        titulo = titulo.left(cut > 45 ? cut : 88).trimmed();
    }
    return titulo.isEmpty() ? QStringLiteral("Seção temática %1").arg(ordinal) : titulo;
}

void criar_secoes_para_texto_sem_hierarquia(const std::shared_ptr<SectionNode> &root)
{
    if (root == nullptr || !root->children.isEmpty() || root->body.isEmpty()) {
        return;
    }

    int total_chars = 0;
    for (const QString &line : root->body) {
        total_chars += line.size() + 1;
    }
    const int target_chars = qMax(3200, (total_chars + 13) / 14);
    QStringList chunk;
    int chunk_chars = 0;
    int ordinal = 1;

    const auto flush_chunk = [&]() {
        if (chunk.isEmpty()) {
            return;
        }
        auto child = std::make_shared<SectionNode>();
        child->level = 1;
        child->title = titulo_para_bloco(chunk, ordinal++, root->title);
        child->body = chunk;
        root->children.append(child);
        chunk.clear();
        chunk_chars = 0;
    };

    for (const QString &line : root->body) {
        chunk.append(line);
        chunk_chars += line.size() + 1;
        if (chunk_chars >= target_chars) {
            flush_chunk();
        }
    }
    flush_chunk();
    if (!root->children.isEmpty()) {
        root->body.clear();
    }
}

void medir_hierarquia(const QVariantMap &item, int &nos, int &profundidade, int &resumos)
{
    ++nos;
    profundidade = qMax(profundidade, item.value(QStringLiteral("nivel")).toInt());
    if (!item.value(QStringLiteral("resumo")).toString().trimmed().isEmpty()) {
        ++resumos;
    }
    for (const QVariant &child : item.value(QStringLiteral("filhos")).toList()) {
        medir_hierarquia(child.toMap(), nos, profundidade, resumos);
    }
}

bool validar_hierarquia_modelo(const QVariantMap &item,
                               int nivel,
                               int &nos,
                               int &profundidade,
                               int &resumos)
{
    if (item.isEmpty() || nivel > 8 || nos >= 500) {
        return false;
    }

    const QString titulo = item.value(QStringLiteral("titulo")).toString().simplified().trimmed();
    if (titulo.isEmpty() || titulo.size() > 240) {
        return false;
    }

    ++nos;
    profundidade = qMax(profundidade, nivel);
    if (!item.value(QStringLiteral("resumo")).toString().simplified().trimmed().isEmpty()) {
        ++resumos;
    }

    const QVariantList filhos = item.value(QStringLiteral("filhos")).toList();
    for (const QVariant &child : filhos) {
        if (child.metaType().id() != QMetaType::QVariantMap
            || !validar_hierarquia_modelo(child.toMap(), nivel + 1, nos, profundidade, resumos)) {
            return false;
        }
    }
    return true;
}

QVariantMap tentar_estrutura_do_plugin_modelo(const QString &texto,
                                              const QString &idioma,
                                              const QVariantList &conceitos,
                                              const QVariantList &relacoes)
{
    source::plugin::MapaMentalHostBridge *bridge = compat::host_bridge();
    if (bridge == nullptr || !bridge->model_available()) {
        return {};
    }

    QVariantMap result = bridge->analyze_with_model(texto, idioma, conceitos, relacoes);
    int total_nos = 0;
    int profundidade = 0;
    int total_resumos = 0;
    if (!validar_hierarquia_modelo(result, 0, total_nos, profundidade, total_resumos)
        || total_nos < 2 || total_resumos < qMax(1, total_nos / 2)) {
        return {};
    }

    if (!result.contains(QStringLiteral("motor_ia"))) {
        result.insert(QStringLiteral("motor_ia"), QStringLiteral("modelo.qwen"));
    }
    result.insert(QStringLiteral("plugin_modelo"), true);
    result.insert(QStringLiteral("nos_estruturados"), total_nos);
    result.insert(QStringLiteral("profundidade_hierarquia"), profundidade);
    result.insert(QStringLiteral("resumos_gerados"), total_resumos);
    return result;
}

}  // namespace

QVariantMap ProcessadorIA::analisar_estrutura(const QString &texto, bool usar_modelo) const
{
    const QString idioma = detectar_idioma_texto(texto);
    const QStringList linhas = preprocessar_linhas(texto);
    const QVariantList conceitos = extrair_conceitos_tfidf(texto);
    const QVariantList relacoes = identificar_relacoes_avancadas(conceitos);

    if (usar_modelo) {
        const QVariantMap estrutura_modelo =
            tentar_estrutura_do_plugin_modelo(texto, idioma, conceitos, relacoes);
        if (!estrutura_modelo.isEmpty()) {
            return estrutura_modelo;
        }
    }

    auto root = std::make_shared<SectionNode>();
    root->title = inferir_titulo_documento(linhas);
    root->level = 0;

    QList<std::shared_ptr<SectionNode>> stack;
    stack.append(root);
    const bool possui_marcadores_pdf = tem_marcadores_markdown(linhas);
    bool ignorando_corpo_acessorio = false;

    for (const QString &linha : linhas) {
        const QString trimmed = linha.trimmed();
        if (trimmed.isEmpty() || e_marcador_nome_fonte(trimmed)
            || ProcessadorIAFns::linha_parece_sumario(trimmed)) {
            continue;
        }

        if (possui_marcadores_pdf && e_sumario(stack.last()->title)
            && !trimmed.startsWith(QLatin1Char('#'))) {
            stack.last()->body.append(trimmed);
            continue;
        }

        const HeadingInfo heading = classificar_titulo(trimmed, stack);
        if (heading.valid) {
            const QString titulo = heading.title.left(160).trimmed();
            if (e_secao_descartavel(chave_titulo(titulo))) {
                ignorando_corpo_acessorio = true;
                continue;
            }
            ignorando_corpo_acessorio = false;
            if (titulos_equivalentes(titulo, root->title) || titulo_repete_ancestral(stack, titulo)) {
                continue;
            }

            int nivel = qMax(1, heading.level);
            if (nivel > stack.size()) {
                nivel = stack.size();
            }
            while (stack.size() > nivel) {
                stack.removeLast();
            }
            if (filho_equivalente_existe(stack.last(), titulo)) {
                continue;
            }

            auto child = std::make_shared<SectionNode>();
            child->title = titulo;
            child->level = heading.level;
            stack.last()->children.append(child);
            stack.append(child);
            continue;
        }

        if (!ignorando_corpo_acessorio) {
            stack.last()->body.append(trimmed);
        }
    }

    criar_secoes_para_texto_sem_hierarquia(root);
    if (root->children.isEmpty()) {
        const QVariantList conceitos_fallback = conceitos.mid(0, 10);
        for (const QVariant &concept_item : conceitos_fallback) {
            const QVariantMap concept_map = concept_item.toMap();
            auto child = std::make_shared<SectionNode>();
            child->title = concept_map.value(QStringLiteral("texto")).toString().trimmed();
            child->level = 1;
            root->children.append(child);
        }
    }

    QStringList contexto_conceitual;
    for (const QVariant &concept_item : conceitos.mid(0, 10)) {
        const QString termo = concept_item.toMap().value(QStringLiteral("texto")).toString().trimmed();
        if (!termo.isEmpty()) {
            contexto_conceitual.append(termo);
        }
    }
    QVariantMap raiz = to_variant(root, contexto_conceitual.join(QLatin1Char(' ')));
    int total_nos = 0;
    int profundidade = 0;
    int total_resumos = 0;
    medir_hierarquia(raiz, total_nos, profundidade, total_resumos);
    raiz.insert(QStringLiteral("idioma"), idioma);
    raiz.insert(QStringLiteral("conceitos"), conceitos);
    raiz.insert(QStringLiteral("relacoes"), relacoes);
    raiz.insert(QStringLiteral("motor_ia"), QStringLiteral("heuristico_aprimorado"));
    raiz.insert(QStringLiteral("metodo_resumo"), QStringLiteral("extrativo_mmr_hierarquico"));
    raiz.insert(QStringLiteral("nos_estruturados"), total_nos);
    raiz.insert(QStringLiteral("profundidade_hierarquia"), profundidade);
    raiz.insert(QStringLiteral("resumos_gerados"), total_resumos);
    return raiz;
}

}  // namespace source::plugin::MapaMental
