#pragma once

#include "source/modules/feynman/mod_05_metodo_feynman.hpp"

class QListWidgetItem;

namespace source::plugin::Feynman::MetodoFeynmanFns {
void vincular_arquivo_conceito(MetodoFeynman &self, QListWidgetItem *item);
void remover_arquivo_vinculado(MetodoFeynman &self, QListWidgetItem *item);
void abrir_arquivo_conceito(MetodoFeynman &self, QListWidgetItem *item);
void on_lista_conceitos_double_click(MetodoFeynman &self, QListWidgetItem *item);
void mostrar_tooltip_arquivo_vinculado(MetodoFeynman &self, QListWidgetItem *item);
void ocultar_tooltip_arquivo_vinculado(MetodoFeynman &self);
void configurar_tooltip_arquivo_vinculado(MetodoFeynman &self);
}
