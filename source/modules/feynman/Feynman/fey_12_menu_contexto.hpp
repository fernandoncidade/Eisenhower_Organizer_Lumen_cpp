#pragma once

#include "source/modules/feynman/mod_05_metodo_feynman.hpp"

class QListWidgetItem;

namespace source::plugin::Feynman::MetodoFeynmanFns {
void mostrar_menu_contexto(MetodoFeynman &self, const QPoint &position);
void redefinir_dominio(MetodoFeynman &self, QListWidgetItem *item, int novo_nivel);
void redefinir_prioridade(MetodoFeynman &self, QListWidgetItem *item, int nova_prioridade);
}
