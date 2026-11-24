#pragma once
#include "source/modules/pomodoro/Pomodoro/tmp_05_GerenciadorTempo.hpp"
namespace source::plugin::Pomodoro::GerenciadorTempoFns {
void abrir_arquivo_vinculado_coluna_doing_timer(GerenciadorTempo &self, QListWidgetItem *item);
void sincronizar_estado_coluna_doing_timer(GerenciadorTempo &self);
void mostrar_menu_contexto_coluna_doing_timer(GerenciadorTempo &self, const QPoint &pos);
}
