#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_34_extrair_texto_epub.hpp"

#include <QHash>
#include <QRegularExpression>
#include <QSet>
#include <QVector>
#include <QXmlStreamReader>

#include <QtGlobal>

#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_33_zip_utils.hpp"

namespace source::plugin::MapaMental::ProcessadorIAFns {
namespace {

struct TocEntry {
    QString title;
    int level = 1;
    QString href;
};

QString texto_xhtml(const QByteArray &data)
{
    QXmlStreamReader xml(data);
    QString texto;
    int skip_depth = 0;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            const QString nome = xml.name().toString().toLower();
            if (nome == QStringLiteral("script") || nome == QStringLiteral("style")
                || nome == QStringLiteral("head")) {
                ++skip_depth;
            } else if (skip_depth == 0
                       && (nome == QStringLiteral("p") || nome == QStringLiteral("div")
                           || nome == QStringLiteral("section") || nome == QStringLiteral("article")
                           || nome == QStringLiteral("li") || nome.startsWith(QLatin1Char('h')))) {
                if (!texto.endsWith(QLatin1Char('\n'))) {
                    texto += QLatin1Char('\n');
                }
            }
        } else if (xml.isCharacters() && skip_depth == 0) {
            const QString trecho = xml.text().toString().simplified();
            if (!trecho.isEmpty()) {
                if (!texto.endsWith(QLatin1Char('\n')) && !texto.endsWith(QLatin1Char(' '))) {
                    texto += QLatin1Char(' ');
                }
                texto += trecho;
            }
        } else if (xml.isEndElement()) {
            const QString nome = xml.name().toString().toLower();
            if (skip_depth > 0
                && (nome == QStringLiteral("script") || nome == QStringLiteral("style")
                    || nome == QStringLiteral("head"))) {
                --skip_depth;
            } else if (skip_depth == 0
                       && (nome == QStringLiteral("p") || nome == QStringLiteral("div")
                           || nome == QStringLiteral("li") || nome.startsWith(QLatin1Char('h')))) {
                texto += QLatin1Char('\n');
            }
        }
    }

    if (!texto.trimmed().isEmpty()) {
        return texto.trimmed();
    }

    QString fallback = QString::fromUtf8(data);
    fallback.remove(QRegularExpression(QStringLiteral("<script[^>]*>.*?</script>"),
                                       QRegularExpression::DotMatchesEverythingOption
                                           | QRegularExpression::CaseInsensitiveOption));
    fallback.remove(QRegularExpression(QStringLiteral("<style[^>]*>.*?</style>"),
                                       QRegularExpression::DotMatchesEverythingOption
                                           | QRegularExpression::CaseInsensitiveOption));
    fallback.replace(QRegularExpression(QStringLiteral("<[^>]+>")), QStringLiteral(" "));
    return fallback.simplified();
}

QString caminho_rootfile(const QString &caminho)
{
    QXmlStreamReader xml(ler_entrada_zip(caminho, QStringLiteral("META-INF/container.xml")));
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == QLatin1String("rootfile")) {
            return xml.attributes().value(QStringLiteral("full-path")).toString();
        }
    }
    return {};
}

QString caminho_nav(const QString &caminho, const QString &opf)
{
    QXmlStreamReader xml(ler_entrada_zip(caminho, opf));
    while (!xml.atEnd()) {
        xml.readNext();
        if (!xml.isStartElement()) {
            continue;
        }
        if (xml.name() != QLatin1String("item")) {
            continue;
        }

        const QString href = xml.attributes().value(QStringLiteral("href")).toString();
        const QString props = xml.attributes().value(QStringLiteral("properties")).toString();
        const QString media = xml.attributes().value(QStringLiteral("media-type")).toString();
        if (href.isEmpty()) {
            continue;
        }
        if (props.contains(QStringLiteral("nav"), Qt::CaseInsensitive)) {
            return resolver_caminho_zip(opf, href);
        }
        if (media.contains(QStringLiteral("xhtml"), Qt::CaseInsensitive)
            && href.contains(QStringLiteral("nav"), Qt::CaseInsensitive)) {
            return resolver_caminho_zip(opf, href);
        }
    }
    return {};
}

QString caminho_ncx(const QString &caminho, const QString &opf)
{
    QXmlStreamReader xml(ler_entrada_zip(caminho, opf));
    while (!xml.atEnd()) {
        xml.readNext();
        if (!xml.isStartElement()) {
            continue;
        }
        if (xml.name() != QLatin1String("item")) {
            continue;
        }

        const QString href = xml.attributes().value(QStringLiteral("href")).toString();
        const QString media = xml.attributes().value(QStringLiteral("media-type")).toString();
        const QString id = xml.attributes().value(QStringLiteral("id")).toString();
        if (href.isEmpty()) {
            continue;
        }
        if (media.contains(QStringLiteral("application/x-dtbncx+xml"), Qt::CaseInsensitive)
            || id.contains(QStringLiteral("ncx"), Qt::CaseInsensitive)) {
            return resolver_caminho_zip(opf, href);
        }
    }
    return {};
}

QVector<TocEntry> parse_nav_xhtml(const QString &caminho, const QString &nav_path)
{
    QVector<TocEntry> entries;
    if (nav_path.isEmpty()) {
        return entries;
    }

    QXmlStreamReader xml(ler_entrada_zip(caminho, nav_path));
    bool in_toc_nav = false;
    bool in_link = false;
    int list_depth = 0;
    QString current_title;
    QString current_href;

    while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
            const QString name = xml.name().toString().toLower();
            if (name == QStringLiteral("nav")) {
                const QString type = xml.attributes().value(QStringLiteral("epub:type")).toString().toLower();
                const QString role = xml.attributes().value(QStringLiteral("role")).toString().toLower();
                if (type == QStringLiteral("toc") || role == QStringLiteral("doc-toc")) {
                    in_toc_nav = true;
                    list_depth = 0;
                }
            } else if (in_toc_nav && name == QStringLiteral("ol")) {
                ++list_depth;
            } else if (in_toc_nav && name == QStringLiteral("a")) {
                in_link = true;
                current_title.clear();
                current_href = xml.attributes().value(QStringLiteral("href")).toString();
            }
        } else if (xml.isCharacters() && in_toc_nav && in_link) {
            current_title += xml.text().toString();
        } else if (xml.isEndElement()) {
            const QString name = xml.name().toString().toLower();
            if (in_toc_nav && name == QStringLiteral("a")) {
                in_link = false;
                const QString title = current_title.simplified().trimmed();
                if (!title.isEmpty() && !current_href.isEmpty()) {
                    TocEntry entry;
                    entry.title = title;
                    entry.level = qMax(1, list_depth);
                    entry.href = resolver_caminho_zip(nav_path, current_href);
                    entries.append(entry);
                }
            } else if (in_toc_nav && name == QStringLiteral("ol")) {
                list_depth = qMax(0, list_depth - 1);
            } else if (name == QStringLiteral("nav")) {
                in_toc_nav = false;
            }
        }
    }

    return entries;
}

QVector<TocEntry> parse_ncx(const QString &caminho, const QString &ncx_path)
{
    QVector<TocEntry> entries;
    if (ncx_path.isEmpty()) {
        return entries;
    }

    QXmlStreamReader xml(ler_entrada_zip(caminho, ncx_path));
    struct WorkItem {
        QString title;
        QString href;
        int level = 1;
    };

    QList<WorkItem> stack;
    bool in_text = false;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            const QString name = xml.name().toString();
            if (name == QLatin1String("navPoint")) {
                WorkItem item;
                item.level = stack.size() + 1;
                stack.append(item);
            } else if (name == QLatin1String("text")) {
                in_text = true;
            } else if (name == QLatin1String("content") && !stack.isEmpty()) {
                stack.last().href = xml.attributes().value(QStringLiteral("src")).toString();
            }
        } else if (xml.isCharacters() && in_text && !stack.isEmpty()) {
            stack.last().title += xml.text().toString();
        } else if (xml.isEndElement()) {
            const QString name = xml.name().toString();
            if (name == QLatin1String("text")) {
                in_text = false;
            } else if (name == QLatin1String("navPoint") && !stack.isEmpty()) {
                WorkItem item = stack.takeLast();
                const QString title = item.title.simplified().trimmed();
                if (!title.isEmpty() && !item.href.isEmpty()) {
                    TocEntry entry;
                    entry.title = title;
                    entry.level = item.level;
                    entry.href = resolver_caminho_zip(ncx_path, item.href);
                    entries.append(entry);
                }
            }
        }
    }

    return entries;
}

QString texto_com_toc(const QString &caminho,
                      const QStringList &entradas,
                      const QVector<TocEntry> &toc_entries)
{
    QHash<QString, QList<TocEntry>> toc_por_arquivo;
    for (const TocEntry &entry : toc_entries) {
        QString arquivo = entry.href;
        const int hash_index = arquivo.indexOf(QLatin1Char('#'));
        if (hash_index >= 0) {
            arquivo = arquivo.left(hash_index);
        }
        if (!arquivo.isEmpty()) {
            toc_por_arquivo[arquivo].append(entry);
        }
    }

    QSet<QString> vistos;
    QString texto;
    for (const QString &entrada : entradas) {
        if (vistos.contains(entrada)) {
            continue;
        }
        vistos.insert(entrada);

        if (toc_por_arquivo.contains(entrada)) {
            const QList<TocEntry> entries = toc_por_arquivo.value(entrada);
            for (const TocEntry &entry : entries) {
                if (entry.title.trimmed().isEmpty()) {
                    continue;
                }
                const int level = qBound(1, entry.level, 6);
                texto += QString(level, QLatin1Char('#'));
                texto += QLatin1Char(' ');
                texto += entry.title.trimmed();
                texto += QLatin1Char('\n');
            }
        }

        const QString trecho = texto_xhtml(ler_entrada_zip(caminho, entrada));
        if (!trecho.isEmpty()) {
            texto += trecho;
            texto += QStringLiteral("\n\n");
        }
    }
    return texto.trimmed();
}

QStringList entradas_spine(const QString &caminho, const QString &opf)
{
    QXmlStreamReader xml(ler_entrada_zip(caminho, opf));
    QHash<QString, QString> manifest;
    QStringList spine;

    while (!xml.atEnd()) {
        xml.readNext();
        if (!xml.isStartElement()) {
            continue;
        }

        if (xml.name() == QLatin1String("item")) {
            const QString id = xml.attributes().value(QStringLiteral("id")).toString();
            const QString href = xml.attributes().value(QStringLiteral("href")).toString();
            const QString media = xml.attributes().value(QStringLiteral("media-type")).toString();
            if (!id.isEmpty() && !href.isEmpty()
                && (media.contains(QStringLiteral("xhtml"), Qt::CaseInsensitive)
                    || media.contains(QStringLiteral("html"), Qt::CaseInsensitive))) {
                manifest.insert(id, resolver_caminho_zip(opf, href));
            }
        } else if (xml.name() == QLatin1String("itemref")) {
            const QString idref = xml.attributes().value(QStringLiteral("idref")).toString();
            if (manifest.contains(idref)) {
                spine.append(manifest.value(idref));
            }
        }
    }

    return spine;
}

}  // namespace

QString extrair_texto_epub(const QString &caminho)
{
    QStringList entradas;
    const QString opf = caminho_rootfile(caminho);
    QVector<TocEntry> toc_entries;
    if (!opf.isEmpty()) {
        entradas = entradas_spine(caminho, opf);
        const QString nav_path = caminho_nav(caminho, opf);
        toc_entries = parse_nav_xhtml(caminho, nav_path);
        if (toc_entries.isEmpty()) {
            const QString ncx_path = caminho_ncx(caminho, opf);
            toc_entries = parse_ncx(caminho, ncx_path);
        }
    }

    if (entradas.isEmpty()) {
        for (const QString &entrada : listar_entradas_zip(caminho)) {
            if (entrada.endsWith(QStringLiteral(".xhtml"), Qt::CaseInsensitive)
                || entrada.endsWith(QStringLiteral(".html"), Qt::CaseInsensitive)
                || entrada.endsWith(QStringLiteral(".htm"), Qt::CaseInsensitive)) {
                entradas.append(entrada);
            }
        }
    }

    if (!toc_entries.isEmpty()) {
        return texto_com_toc(caminho, entradas, toc_entries);
    }

    QSet<QString> vistos;
    QString texto;
    for (const QString &entrada : entradas) {
        if (vistos.contains(entrada)) {
            continue;
        }
        vistos.insert(entrada);
        const QString trecho = texto_xhtml(ler_entrada_zip(caminho, entrada));
        if (!trecho.isEmpty()) {
            texto += trecho;
            texto += QStringLiteral("\n\n");
        }
    }
    return texto.trimmed();
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
