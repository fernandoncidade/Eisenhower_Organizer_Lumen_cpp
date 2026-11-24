#pragma once

#include <QList>
#include "source/services/incore_12_Manual.hpp"

namespace source::InterfaceCore::incore_17_manual_en_US {

using namespace source::InterfaceCore::incore_12_Manual;

// Documented sections in English (US) for the interactive manual:
// Includes instructions on responsive task creation/editing (with instantaneous confirmation without freezing),
// full lifecycle visibility in the Calendar ([Completed], [Pending], and [In Progress]),
// Pomodoro timer frame standardization ('⏱️ Timer'), filters, integrations,
// interactive on-demand download manager for local Qwen 2.5 (GGUF) with pause/resume and deletion support,
// local Ollama model integration with interactive command instructions and Mind Map node synthesis.
QList<ManualSection> en_document();
QList<ManualSection> feynman_en_document();
QList<ManualSection> pomodoro_en_document();
QList<ManualSection> mapamental_en_document();
QList<ManualSection> audiolivro_en_document();
QList<ManualSection> agenda_en_document();
QList<ManualSection> modelo_en_document();
QList<ManualSection> gantt_en_document();
QList<ManualSection> final_sections_en_document();

}  // namespace source::InterfaceCore::incore_17_manual_en_US
