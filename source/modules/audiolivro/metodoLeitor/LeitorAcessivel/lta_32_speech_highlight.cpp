#include "source/modules/audiolivro/metodoLeitor/LeitorAcessivel/lta_32_speech_highlight.hpp"

#include <QColor>

namespace source::plugin::Audiolivro {

SpeechHighlightManager::SpeechHighlightManager(QObject *parent)
    : QObject(parent)
{
}

void SpeechHighlightManager::set_text(const QString &text)
{
    text_ = text;
}

}  // namespace source::plugin::Audiolivro
