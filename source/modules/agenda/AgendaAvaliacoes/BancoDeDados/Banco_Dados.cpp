#include "Banco_Dados.hpp"

#include "DicionariosListasBancoDeDados/CURSOS_EN.hpp"
#include "DicionariosListasBancoDeDados/CURSOS_PT.hpp"
#include "DicionariosListasBancoDeDados/dicionario_de_cores.hpp"
#include "DicionariosListasBancoDeDados/dicionario_disciplinas.hpp"
#include "DicionariosListasBancoDeDados/lista_avaliacoes_en_US.hpp"
#include "DicionariosListasBancoDeDados/lista_avaliacoes_pt_BR.hpp"
#include "DicionariosListasBancoDeDados/lista_cursos_en_US.hpp"
#include "DicionariosListasBancoDeDados/lista_cursos_pt_BR.hpp"
#include "DicionariosListasBancoDeDados/lista_turmas_en_US.hpp"
#include "DicionariosListasBancoDeDados/lista_turmas_pt_BR.hpp"
#include "DicionariosListasBancoDeDados/mapeamento_cursos.hpp"
#include "DicionariosListasBancoDeDados/mapeamento_ementas.hpp"
#include "DicionariosListasBancoDeDados/mapeamento_semestres.hpp"
#include "Listas_Personalizadas.hpp"
#include "source/modules/agenda/AgendaAvaliacoes/utils/CaminhoPersistenteUtils.hpp"
#include "source/utils/LogManager.hpp"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSet>

namespace {
    QStringList g_listaCursos;
    QStringList g_listaTurmas;
    QStringList g_listaAvaliacoes;
    QStringList g_listaSequencias;
    QHash<QString, QString> g_mapeamentoCodigoCor;
    QHash<QString, QString> g_mapeamentoNomeCor;

    QHash<QString, QString> g_mapeamentoCoresPersonalizadas;
    bool g_coresPersonalizadasCarregadas = false;

    QString casefold(const QString &s) {
        QString res = s.trimmed().toCaseFolded();
        res.replace(QChar(0x2013), QChar('-'));
        res.replace(QChar(0x2014), QChar('-'));
        res.replace(QChar(0x2212), QChar('-'));
        return res;
    }

    QString caminho_arquivo_cores_personalizadas() {
        return QDir(obter_caminho_persistente()).filePath(QStringLiteral("cores_disciplinas_personalizadas.json"));
    }

    void carregar_cores_personalizadas() {
        if (g_coresPersonalizadasCarregadas)
            return;
        g_coresPersonalizadasCarregadas = true;
        g_mapeamentoCoresPersonalizadas.clear();
        QFile file(caminho_arquivo_cores_personalizadas());
        if (file.open(QIODevice::ReadOnly)) {
            const QJsonObject obj = QJsonDocument::fromJson(file.readAll()).object();
            for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
                if (it.value().isString()) {
                    g_mapeamentoCoresPersonalizadas.insert(casefold(it.key().trimmed()), it.value().toString().trimmed());
                }
            }
        }
    }

    void salvar_cores_personalizadas() {
        QDir().mkpath(obter_caminho_persistente());
        QFile file(caminho_arquivo_cores_personalizadas());
        if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            QJsonObject obj;
            for (auto it = g_mapeamentoCoresPersonalizadas.constBegin(); it != g_mapeamentoCoresPersonalizadas.constEnd(); ++it) {
                obj.insert(it.key(), it.value());
            }
            file.write(QJsonDocument(obj).toJson(QJsonDocument::Indented));
        }
    }

    const CursoCurricular *buscar_curso(const EstruturaCursos &estrutura, const QString &curso) {
        for (const auto &item : estrutura) {
            if (item.nome == curso || casefold(item.nome) == casefold(curso)) {
                return &item;
            }
        }
        return nullptr;
    }

    const EmentaCurricular *buscar_ementa(const CursoCurricular &curso, const QString &ementa) {
        for (const auto &item : curso.ementas) {
            if (item.nome == ementa || casefold(item.nome) == casefold(ementa)) {
                return &item;
            }
        }
        return nullptr;
    }

    const SemestreCurricular *buscar_semestre(const EmentaCurricular &ementa, const QString &semestre) {
        for (const auto &item : ementa.semestres) {
            if (item.nome == semestre || casefold(item.nome) == casefold(semestre)) {
                return &item;
            }
        }
        return nullptr;
    }
} // namespace

QString obter_idioma_atual() {
    static QString s_cached_idioma = QStringLiteral("pt_BR");
    static QDateTime s_last_modified;
    static QString s_cached_path;

    const QString path = QDir(obter_caminho_persistente()).filePath(QStringLiteral("language.json"));
    QFileInfo fi(path);
    if (!fi.exists()) {
        return QStringLiteral("pt_BR");
    }

    const QDateTime mtime = fi.lastModified();
    if (path == s_cached_path && mtime == s_last_modified && !s_cached_idioma.isEmpty()) {
        return s_cached_idioma;
    }

    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        const QString idioma = QJsonDocument::fromJson(file.readAll()).object().value(QStringLiteral("idioma")).toString();
        if (!idioma.isEmpty()) {
            s_cached_idioma = idioma;
            s_last_modified = mtime;
            s_cached_path = path;
            return s_cached_idioma;
        }
    }
    return s_cached_idioma;
}

QStringList _mesclar_listas(const QList<QStringList> &listas) {
    QStringList resultado;
    QSet<QString> vistos;
    for (const QStringList &lista : listas) {
        for (const QString &item : lista) {
            const QString chave = casefold(item);
            if (vistos.contains(chave)) {
                continue;
            }
            vistos.insert(chave);
            resultado << item;
        }
    }
    return resultado;
}

QStringList obter_cursos() {
    const QString idioma = obter_idioma_atual();
    const QStringList base = idioma == "en_US" ? lista_cursos_en_US() : lista_cursos_pt_BR();
    return _mesclar_listas({base, listar_cursos(idioma)});
}

QStringList obter_turmas() {
    const QString idioma = obter_idioma_atual();
    const QStringList base = idioma == "en_US" ? lista_turmas_en_US() : lista_turmas_pt_BR();
    return _mesclar_listas({base, listar_turmas(idioma)});
}

QStringList obter_avaliacoes() {
    const QString idioma = obter_idioma_atual();
    const QStringList base = idioma == "en_US" ? lista_avaliacoes_en_US() : lista_avaliacoes_pt_BR();
    return _mesclar_listas({base, listar_tipos_atividade(idioma)});
}

QStringList obter_sequencias() {
    QStringList base{QString()};
    for (int i = 1; i <= 10; ++i)
        base << QString::number(i);
    return _mesclar_listas({base, listar_sequencias_atividade(obter_idioma_atual())});
}

QString traduzir_curso(const QString &curso, const QString &idioma_destino) {
    const QString idioma = idioma_destino.isEmpty() ? obter_idioma_atual() : idioma_destino;
    if (idioma == "pt_BR")
        return curso;
    return mapeamento_cursos().value(idioma).value(curso, curso);
}

QString traduzir_ementa(const QString &ementa, const QString &idioma_destino) {
    const QString idioma = idioma_destino.isEmpty() ? obter_idioma_atual() : idioma_destino;
    if (idioma == "pt_BR")
        return ementa;
    return mapeamento_ementas().value(idioma).value(ementa, ementa);
}

QString traduzir_semestre(const QString &semestre, const QString &idioma_destino) {
    const QString idioma = idioma_destino.isEmpty() ? obter_idioma_atual() : idioma_destino;
    if (idioma == "pt_BR")
        return semestre;
    return mapeamento_semestres().value(idioma).value(semestre, semestre);
}

const EstruturaCursos &obter_estrutura_cursos() {
    return obter_idioma_atual() == "en_US" ? CURSOS_EN() : CURSOS_PT();
}

QStringList obter_ementas(const QString &curso) {
    const QString idioma = obter_idioma_atual();
    const auto personalizadas = listar_ementas(idioma, curso);
    if (curso.trimmed().isEmpty())
        return _mesclar_listas({personalizadas});
    if (const CursoCurricular *c = buscar_curso(obter_estrutura_cursos(), curso)) {
        QStringList base;
        for (const auto &ementa : c->ementas)
            base << ementa.nome;
        return _mesclar_listas({base, personalizadas});
    }
    return _mesclar_listas({personalizadas});
}

QStringList obter_semestres(const QString &curso, const QString &ementa) {
    const auto personalizadas = listar_semestres(obter_idioma_atual(), curso, ementa);
    if (curso.trimmed().isEmpty() || ementa.trimmed().isEmpty())
        return _mesclar_listas({personalizadas});
    if (const CursoCurricular *c = buscar_curso(obter_estrutura_cursos(), curso)) {
        if (const EmentaCurricular *e = buscar_ementa(*c, ementa)) {
            QStringList base;
            for (const auto &sem : e->semestres)
                base << sem.nome;
            return _mesclar_listas({base, personalizadas});
        }
    }
    return _mesclar_listas({personalizadas});
}

QStringList obter_disciplinas(const QString &curso, const QString &ementa, const QString &semestre) {
    const auto personalizadas = listar_disciplinas(obter_idioma_atual(), curso, ementa, semestre);
    if (curso.trimmed().isEmpty() || ementa.trimmed().isEmpty() || semestre.trimmed().isEmpty())
        return _mesclar_listas({personalizadas});
    if (const CursoCurricular *c = buscar_curso(obter_estrutura_cursos(), curso)) {
        if (const EmentaCurricular *e = buscar_ementa(*c, ementa)) {
            if (const SemestreCurricular *s = buscar_semestre(*e, semestre)) {
                return _mesclar_listas({s->disciplinas, personalizadas});
            }
        }
    }
    return _mesclar_listas({personalizadas});
}

QHash<QString, bool> registrar_valores_personalizados(const QString &curso, const QString &ementa,
                                                      const QString &semestre, const QString &disciplina,
                                                      const QString &turma, const QString &tipo,
                                                      const QString &sequencia, bool registrar_curso,
                                                      bool registrar_ementa, bool registrar_semestre,
                                                      bool registrar_disciplina, bool registrar_turma,
                                                      bool registrar_tipo, bool registrar_sequencia) {
    QHash<QString, bool> resultados;
    const QString idioma = obter_idioma_atual();
    if (registrar_curso)
        resultados.insert("curso", inserir_curso(idioma, curso));
    if (registrar_ementa)
        resultados.insert("ementa", inserir_ementa(idioma, curso, ementa));
    if (registrar_semestre)
        resultados.insert("semestre", inserir_semestre(idioma, curso, ementa, semestre));
    if (registrar_disciplina)
        resultados.insert("disciplina", inserir_disciplina(idioma, curso, ementa, semestre, disciplina));
    if (registrar_turma)
        resultados.insert("turma", inserir_turma(idioma, turma));
    if (registrar_tipo)
        resultados.insert("tipo", inserir_tipo_atividade(idioma, tipo));
    if (registrar_sequencia)
        resultados.insert("sequencia", inserir_sequencia_atividade(idioma, sequencia));
    return resultados;
}

void atualizar_listas_exportadas() {
    g_listaCursos = obter_cursos();
    g_listaTurmas = obter_turmas();
    g_listaAvaliacoes = obter_avaliacoes();
    g_listaSequencias = obter_sequencias();
}

QStringList lista_cursos() {
    if (g_listaCursos.isEmpty())
        g_listaCursos = obter_cursos();
    return g_listaCursos;
}

QStringList lista_turmas() {
    if (g_listaTurmas.isEmpty())
        g_listaTurmas = obter_turmas();
    return g_listaTurmas;
}

QStringList lista_avaliacoes() {
    if (g_listaAvaliacoes.isEmpty())
        g_listaAvaliacoes = obter_avaliacoes();
    return g_listaAvaliacoes;
}

QStringList lista_sequencias() {
    if (g_listaSequencias.isEmpty())
        g_listaSequencias = obter_sequencias();
    return g_listaSequencias;
}

QHash<QString, QString> construir_mapeamento_codigo_para_cor() {
    QHash<QString, QString> mapping;
    g_mapeamentoNomeCor.clear();
    const QRegularExpression pattern("([A-Z]{1,3}\\d{3,4})$", QRegularExpression::CaseInsensitiveOption);
    for (auto it = dicionario_disciplinas().cbegin(); it != dicionario_disciplinas().cend(); ++it) {
        const QString cor = dicionario_de_cores().value(it.key());
        if (cor.isEmpty()) {
            continue;
        }
        for (const QString &disciplina : it.value()) {
            const QString trimmed = disciplina.trimmed();
            g_mapeamentoNomeCor.insert(casefold(trimmed), cor);
            const auto match = pattern.match(trimmed);
            if (match.hasMatch()) {
                mapping.insert(match.captured(1).toUpper(), cor);
            }
        }
    }
    return mapping;
}

QString obter_cor_por_nome(const QString &nome_disciplina) {
    if (nome_disciplina.trimmed().isEmpty())
        return QString();
    if (g_mapeamentoCodigoCor.isEmpty())
        g_mapeamentoCodigoCor = construir_mapeamento_codigo_para_cor();
    const QRegularExpression pattern("([A-Z]{1,3}\\d{3,4})$", QRegularExpression::CaseInsensitiveOption);
    const auto match = pattern.match(nome_disciplina.trimmed());
    if (match.hasMatch()) {
        const QString cor = g_mapeamentoCodigoCor.value(match.captured(1).toUpper());
        if (!cor.isEmpty())
            return cor;
    }
    const QString lower = casefold(nome_disciplina);
    const QString corNome = g_mapeamentoNomeCor.value(lower);
    if (!corNome.isEmpty()) {
        return corNome;
    }

    carregar_cores_personalizadas();

    if (g_mapeamentoCoresPersonalizadas.contains(lower)) {
        const QString cor = g_mapeamentoCoresPersonalizadas.value(lower);
        if (!cor.isEmpty())
            return cor;
    }
    if (match.hasMatch()) {
        const QString code_lower = casefold(match.captured(1).toUpper());
        if (g_mapeamentoCoresPersonalizadas.contains(code_lower)) {
            const QString cor = g_mapeamentoCoresPersonalizadas.value(code_lower);
            if (!cor.isEmpty())
                return cor;
        }
    }


    return QString();
}

QString obter_ou_atribuir_cor_disciplina(const QString &nome_disciplina) {
    if (nome_disciplina.trimmed().isEmpty()) {
        return QString();
    }
    const QString cor_existente = obter_cor_por_nome(nome_disciplina);
    if (!cor_existente.isEmpty()) {
        return cor_existente;
    }

    carregar_cores_personalizadas();

    static const QStringList palette = {
        QStringLiteral("#E53935"),
        QStringLiteral("#8E24AA"),
        QStringLiteral("#3949AB"),
        QStringLiteral("#039BE5"),
        QStringLiteral("#00897B"),
        QStringLiteral("#43A047"),
        QStringLiteral("#FDD835"),
        QStringLiteral("#FB8C00"),
        QStringLiteral("#6D4C41"),
        QStringLiteral("#546E7A"),
    };

    QSet<QString> cores_usadas;
    for (const QString &cor : dicionario_de_cores().values()) {
        cores_usadas.insert(cor.trimmed().toUpper());
    }
    for (const QString &cor : g_mapeamentoCoresPersonalizadas.values()) {
        cores_usadas.insert(cor.trimmed().toUpper());
    }

    QSet<QString> cores_customizadas_usadas;
    for (const QString &cor : g_mapeamentoCoresPersonalizadas.values()) {
        cores_customizadas_usadas.insert(cor.trimmed().toUpper());
    }

    QString cor_selecionada;
    for (const QString &c : palette) {
        if (!cores_usadas.contains(c.trimmed().toUpper())) {
            cor_selecionada = c;
            break;
        }
    }
    if (cor_selecionada.isEmpty()) {
        for (const QString &c : palette) {
            if (!cores_customizadas_usadas.contains(c.trimmed().toUpper())) {
                cor_selecionada = c;
                break;
            }
        }
    }
    if (cor_selecionada.isEmpty() && !palette.isEmpty()) {
        uint hash_val = 0;
        const QString trimmed_lower = casefold(nome_disciplina.trimmed());
        for (QChar c : trimmed_lower) {
            hash_val = (hash_val * 31) + c.unicode();
        }
        cor_selecionada = palette.value(hash_val % palette.size());
    }

    const QString lower = casefold(nome_disciplina.trimmed());
    g_mapeamentoCoresPersonalizadas.insert(lower, cor_selecionada);

    const QRegularExpression pattern("([A-Z]{1,3}\\d{3,4})$", QRegularExpression::CaseInsensitiveOption);
    const auto match = pattern.match(nome_disciplina.trimmed());
    if (match.hasMatch()) {
        g_mapeamentoCoresPersonalizadas.insert(casefold(match.captured(1).toUpper()), cor_selecionada);
    }
    salvar_cores_personalizadas();

    return cor_selecionada;
}
