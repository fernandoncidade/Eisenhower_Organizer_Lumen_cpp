#pragma once

#include <QList>
#include "source/services/incore_12_Manual.hpp"

namespace source::InterfaceCore::incore_16_manual_pt_BR {

using namespace source::InterfaceCore::incore_12_Manual;

// Seções documentadas em Português (Brasil) para o manual interativo:
// Inclui instruções sobre criação e edição responsiva de tarefas (com confirmação instantânea sem travamentos),
// visibilidade do ciclo de vida no Calendário ([Concluída], [Pendente] e [Em Progresso]),
// padronização do temporizador Pomodoro ('⏱️ Temporizador'), filtros, integrações,
// gerenciador interativo de download do modelo local Qwen 2.5 (GGUF) com suporte a pausa/retomada e exclusão,
// integração com modelos locais via Ollama com guia interativo de comandos e síntese de nós no Mapa Mental.
QList<ManualSection> pt_document();
QList<ManualSection> feynman_pt_document();
QList<ManualSection> pomodoro_pt_document();
QList<ManualSection> mapamental_pt_document();
QList<ManualSection> audiolivro_pt_document();
QList<ManualSection> agenda_pt_document();
QList<ManualSection> modelo_pt_document();
QList<ManualSection> gantt_pt_document();
QList<ManualSection> final_sections_pt_document();

}  // namespace source::InterfaceCore::incore_16_manual_pt_BR
