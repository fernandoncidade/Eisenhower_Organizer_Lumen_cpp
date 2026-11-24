#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_34_pdf_speech_highlight.hpp"

#include <QColor>
#include <QFileInfo>
#include <QPdfDocument>
#include <QPdfSelection>
#include <QRegularExpression>

namespace source::plugin::Audiolivro {

PDFSpeechHighlightManager::PDFSpeechHighlightManager(QObject *parent)
    : QObject(parent)
{
}

void PDFSpeechHighlightManager::set_words(const QList<PDFWordRect> &words)
{
    words_ = words;
}

QString extract_word_positions_from_pdf(const QString &pdf_path, int page_index, QList<PDFWordRect> *words)
{
    if (!QFileInfo::exists(pdf_path)) {
        return QString();
    }

    QPdfDocument document;
    if (document.load(pdf_path) != QPdfDocument::Error::None) {
        return QString();
    }

    QString text;
    const int first = page_index >= 0 ? qBound(0, page_index, document.pageCount() - 1) : 0;
    const int last = page_index >= 0 ? first : document.pageCount() - 1;
    int offset = 0;
    for (int page = first; page <= last; ++page) {
        const QString page_text = document.getAllText(page).text();
        if (!text.isEmpty()) {
            text += QStringLiteral("\n\n");
            offset += 2;
        }
        text += page_text;

        if (words != nullptr) {
            QRegularExpression re(QStringLiteral("\\S+"));
            auto it = re.globalMatch(page_text);
            while (it.hasNext()) {
                const auto match = it.next();
                words->append(PDFWordRect{page,
                                          match.captured(),
                                          QRectF(),
                                          static_cast<int>(offset + match.capturedStart())});
            }
        }
        offset += page_text.size();
    }

    return text;
}

}  // namespace source::plugin::Audiolivro
