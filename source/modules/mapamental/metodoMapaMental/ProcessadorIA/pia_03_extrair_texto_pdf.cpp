#include "source/modules/mapamental/metodoMapaMental/ProcessadorIA/pia_03_extrair_texto_pdf.hpp"

#include <QtPdf/QPdfBookmarkModel>
#include <QtPdf/QPdfDocument>
#include <QtPdf/QPdfSelection>

#include <algorithm>

#include <QHash>
#include <QModelIndex>
#include <QPointF>
#include <QRegularExpression>
#include <QStringList>
#include <QVariant>
#include <QVector>

#include <QtGlobal>

namespace source::plugin::MapaMental::ProcessadorIAFns {
namespace {

struct PdfMarker {
    QString title;
    int level = 1;
    int page = -1;
    QPointF location;
    bool has_location = false;
    int order = 0;
};

QString titulo_documento(QPdfDocument &documento)
{
    return documento.metaData(QPdfDocument::MetaDataField::Title).toString().simplified().trimmed();
}

QString normalizar_titulo_marcador(const QString &titulo, int level)
{
    const QString trimmed = titulo.simplified().trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }

    static const QRegularExpression numbered_chapter(QStringLiteral("^(\\d{1,3})\\s*[\\.)]\\s*(.+)$"));
    const QRegularExpressionMatch chapter_match = numbered_chapter.match(trimmed);
    if (level >= 2 && chapter_match.hasMatch()) {
        const QString numero = chapter_match.captured(1);
        const QString nome = chapter_match.captured(2).simplified().trimmed().toUpper();
        return QStringLiteral("CAPÍTULO %1/%2").arg(numero, nome);
    }

    return trimmed;
}

QVector<PdfMarker> extrair_marcadores(QPdfDocument &documento)
{
    QVector<PdfMarker> markers;
    QPdfBookmarkModel model;
    model.setDocument(&documento);
    int order = 0;

    const auto walk = [&](const QModelIndex &parent, int level, const auto &self) -> void {
        const int rows = model.rowCount(parent);
        for (int row = 0; row < rows; ++row) {
            const QModelIndex index = model.index(row, 0, parent);
            if (!index.isValid()) {
                continue;
            }
            const QString title = model.data(index, static_cast<int>(QPdfBookmarkModel::Role::Title))
                                      .toString()
                                      .simplified();
            const int page = model.data(index, static_cast<int>(QPdfBookmarkModel::Role::Page)).toInt();
            const QVariant loc_data = model.data(index, static_cast<int>(QPdfBookmarkModel::Role::Location));
            const bool has_location = loc_data.canConvert<QPointF>();
            if (!title.isEmpty() && page >= 0) {
                PdfMarker marker;
                marker.title = title;
                marker.level = level;
                marker.page = page;
                marker.location = has_location ? loc_data.toPointF() : QPointF();
                marker.has_location = has_location;
                marker.order = order++;
                markers.append(marker);
            }
            self(index, level + 1, self);
        }
    };

    walk(QModelIndex(), 1, walk);
    return markers;
}

qreal clamp_position(qreal value, qreal min_value, qreal max_value)
{
    return qBound(min_value, value, max_value);
}

QString selection_text(QPdfDocument &documento, int page, qreal y_start, qreal y_end)
{
    const QSizeF size = documento.pagePointSize(page);
    if (size.isEmpty() || size.width() <= 0.0 || size.height() <= 0.0) {
        return {};
    }
    const qreal y0 = clamp_position(y_start, 0.0, size.height());
    const qreal y1 = clamp_position(y_end, 0.0, size.height());
    if (y1 <= y0 + 1.0) {
        return {};
    }
    const QPdfSelection sel = documento.getSelection(page, QPointF(0.0, y0), QPointF(size.width(), y1));
    return sel.text().trimmed();
}

QString extrair_intervalo(QPdfDocument &documento, const PdfMarker &inicio, const PdfMarker *fim)
{
    if (inicio.page < 0 || documento.pageCount() <= 0) {
        return {};
    }
    const int last_page = documento.pageCount() - 1;
    int start_page = inicio.page;
    int end_page = fim != nullptr ? fim->page : last_page;
    start_page = qBound(0, start_page, last_page);
    end_page = qBound(start_page, end_page, last_page);

    const QSizeF start_size = documento.pagePointSize(start_page);
    qreal start_y = 0.0;
    if (inicio.has_location && !start_size.isEmpty()) {
        start_y = clamp_position(inicio.location.y(), 0.0, start_size.height());
    }

    qreal end_y = 0.0;
    if (fim != nullptr) {
        const QSizeF end_size = documento.pagePointSize(end_page);
        if (fim->has_location && !end_size.isEmpty()) {
            end_y = clamp_position(fim->location.y(), 0.0, end_size.height());
        } else {
            end_y = end_size.isEmpty() ? 0.0 : end_size.height();
        }
    } else {
        const QSizeF end_size = documento.pagePointSize(end_page);
        end_y = end_size.isEmpty() ? 0.0 : end_size.height();
    }

    QStringList chunks;
    if (start_page == end_page) {
        const QSizeF size = documento.pagePointSize(start_page);
        const qreal fallback_end = size.isEmpty() ? 0.0 : size.height();
        const qreal y_end = fim != nullptr ? end_y : fallback_end;
        const QString chunk = selection_text(documento, start_page, start_y, y_end);
        if (!chunk.isEmpty()) {
            chunks.append(chunk);
        }
        return chunks.join(QStringLiteral("\n\n")).trimmed();
    }

    const QString first_chunk = selection_text(documento, start_page, start_y,
                                               documento.pagePointSize(start_page).height());
    if (!first_chunk.isEmpty()) {
        chunks.append(first_chunk);
    }

    for (int page = start_page + 1; page < end_page; ++page) {
        const QPdfSelection sel = documento.getAllText(page);
        const QString text = sel.text().trimmed();
        if (!text.isEmpty()) {
            chunks.append(text);
        }
    }

    const QString last_chunk = selection_text(documento, end_page, 0.0, end_y);
    if (!last_chunk.isEmpty()) {
        chunks.append(last_chunk);
    }

    return chunks.join(QStringLiteral("\n\n")).trimmed();
}

}  // namespace

QString extrair_texto_pdf(const QString &caminho)
{
    QPdfDocument documento;
    if (documento.load(caminho) != QPdfDocument::Error::None
        || documento.status() != QPdfDocument::Status::Ready) {
        return {};
    }

    const QString titulo = titulo_documento(documento);
    const QVector<PdfMarker> markers = extrair_marcadores(documento);
    if (!markers.isEmpty()) {
        QVector<PdfMarker> ordered = markers;
        std::sort(ordered.begin(), ordered.end(), [](const PdfMarker &a, const PdfMarker &b) {
            if (a.page == b.page) {
                if (a.has_location && b.has_location && a.location.y() != b.location.y()) {
                    return a.location.y() < b.location.y();
                }
                return a.order < b.order;
            }
            return a.page < b.page;
        });

        QString texto;
        if (!titulo.isEmpty()) {
            texto += titulo;
            texto += QStringLiteral("\n\n");
        }
        for (int i = 0; i < ordered.size(); ++i) {
            const PdfMarker &marker = ordered.at(i);
            const int level = qBound(1, marker.level, 6);
            const QString marker_title = normalizar_titulo_marcador(marker.title, level);
            if (marker_title.isEmpty()) {
                continue;
            }
            texto += QString(level, QLatin1Char('#'));
            texto += QLatin1Char(' ');
            texto += marker_title;
            texto += QLatin1Char('\n');

            const PdfMarker *next = (i + 1) < ordered.size() ? &ordered.at(i + 1) : nullptr;
            const bool proximo_e_filho = next != nullptr && next->level > marker.level;
            const QString trecho = proximo_e_filho ? QString() : extrair_intervalo(documento, marker, next);
            if (!trecho.isEmpty()) {
                texto += trecho;
                texto += QStringLiteral("\n\n");
            }
        }
        return texto.trimmed();
    }

    QString texto;
    if (!titulo.isEmpty()) {
        texto += titulo;
        texto += QStringLiteral("\n\n");
    }
    for (int pagina = 0; pagina < documento.pageCount(); ++pagina) {
        const QPdfSelection selecao = documento.getAllText(pagina);
        const QString pagina_texto = selecao.text().trimmed();
        if (!pagina_texto.isEmpty()) {
            texto += pagina_texto;
            texto += QStringLiteral("\n\n");
        }
    }
    return texto.trimmed();
}

}  // namespace source::plugin::MapaMental::ProcessadorIAFns
