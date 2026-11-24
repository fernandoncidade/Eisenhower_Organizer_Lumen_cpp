# Conformidade Algorítmica, Acessibilidade e Validação / Algorithmic Compliance & Verification — EISENHOWER ORGANIZER / LÚMEN

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details open>
<summary>Clique para expandir / recolher o documento de conformidade em português</summary><br>

# EISENHOWER ORGANIZER / LÚMEN — Conformidade Algorítmica, Acessibilidade e Validação Automatizada

**Versão:** `2026.11.22.0`  
**Data:** `22 de novembro de 2026`  
**Autor:** Fernando Nillsson Cidade  

---

## 1. Fundamentos Matemáticos e Invariantes Algorítmicos

### 1.1. Método do Caminho Crítico e Validação DAG (`Gantt` — `CpmEngine` & `DependencyValidator`)
O módulo **Cronograma de Gantt BIM 4D** implementa cálculo clássico de redes de precedência sobre um Grafo Acíclico Dirigido (DAG):

1. **Validação Topológica (`DependencyValidator`):** Antes da execução do CPM, toda inclusão de vínculo (`TI`/`FS`, `II`/`SS`, `TT`/`FF`, `IT`/`SF`) passa por busca em grafo para impedir dependências circulares (ciclos diretos, transitivos ou auto-vínculos), garantindo ordenamento topológico válido.
2. **Passagem Direta (*Forward Pass*):** Determinação das datas mais cedo de início ($ES_j$) e término ($EF_j$) considerando a duração útil descontados finais de semana e feriados (`WorkCalendar`) mais a defasagem (*lag* $L_{ij}$):
   $$EF_i = ES_i + D_i, \qquad ES_j = \max_{(i \to j)} \{ g_{ij}(ES_i, EF_i, D_j, L_{ij}) \}$$
3. **Passagem Reversa (*Backward Pass*) e Folga Total (*Total Float*):** Determinação de $LF_i$ e $LS_i$ a partir do marco final $T_{\text{proj}} = \max_k EF_k$:
   $$LS_i = LF_i - D_i, \qquad TF_i = LS_i - ES_i = LF_i - EF_i$$
4. **Caminho Crítico:** Toda atividade com folga total nula ($TF_i = 0$) pertence ao caminho crítico, sendo destacada visualmente em `GanttTimelineWidget`.

### 1.2. Métricas de Progresso (`TaskProgressHelper`) e Heurística TF-IDF (`Mapa Mental`)
- **`TaskProgressHelper`:** Calcula o percentual de execução de `0%` a `100%` com limitação segura com base nos ciclos Pomodoro concluídos versus meta configurada ou no percentual físico sincronizado do Gantt, prevenindo divisão por zero quando a meta é `0`.
- **Fallback Heurístico (`Mapa Mental`):** Caso não exista modelo GGUF ou servidor Ollama ativo, o extrator pontua sentenças e termos por frequência ponderada (TF-IDF simplificado), detecta idioma e constrói uma árvore de conceitos sem depender de rede.

---

## 2. Integridade Transacional e Acessibilidade Cognitiva (Store & ADA)

1. **Invariantes do Armazenamento (`UnifiedTaskStore` / SQLite):**
   - `tarefas.json` preserva contagem exata entre quadrantes, integridade dos campos `Em Progresso` / `Pendente` / `Concluída`, geração determinística de recorrências (`avoid duplicates`) e proteção contra corrupção via `QLockFile` + `QSaveFile`.
   - `listas.db` (Agenda) e `gantt_database.db` (Gantt) mantêm chaves estrangeiras e histórico de migrações/auditoria consistentes.
2. **Acessibilidade Cognitiva e Visual (`Lúmen`, `Audiolivro`, Microsoft Store ADA):**
   - Conformidade documentada para publicação em lojas no artefato [`MS-Store-ADA-v8.9-FINAL-EN.pdf`](../source/assets/LICENSES/MS-Store-ADA-v8.9-FINAL-EN.pdf);
   - Apoio dedicado a usuários com **TDAH, dislexia e TEA**: fontes configuráveis por módulo/vínculo, controle granular de espaçamento/margem/recuo no Audiolivro, **Régua de Foco** linha a linha, realce sincronizado palavra/frase/parágrafo e modos visuais simplificados (`Abas`, `Matriz 2x2`, recolhimento horizontal com rotação 90°).

---

## 3. Verificação Automatizada Contínua (`suite_testes` — 11 Alvos CTest)

Todos os invariantes acima são verificados após cada alteração tanto em **Pure MSVC 14.52** (`suite-testes-msvc-release`) quanto em **MinGW GCC 15.1.0** (`suite-testes-mingw-release`), com **11/11 suítes aprovadas**:

| Executável CTest | Arquivo Fonte | Escopo Verificado |
|---|---|---|
| `test_core_utils` | [`tests/test_core_utils.cpp`](../tests/test_core_utils.cpp) | `AppConfig` dual (`EISENHOWER`/`LUMEN`), `TrialManager`, `LogManager`, `TaskProgressHelper`, `ApplicationPathUtils` |
| `test_store_models` | [`tests/test_store_models.cpp`](../tests/test_store_models.cpp) | `UnifiedTaskStore` (`QLockFile`/`QSaveFile`), filtros, busca, normalização de datas, recorrências e exportação |
| `test_ui_host` | [`tests/test_ui_host.cpp`](../tests/test_ui_host.cpp) | `EisenhowerMatrixApp`, `OpcoesSobre` (todos documentos Eisenhower/Lúmen `pt_BR`/`en_US`), `EditTaskDialog`, layouts |
| `test_plugin_pomodoro` | [`tests/test_plugin_pomodoro.cpp`](../tests/test_plugin_pomodoro.cpp) | Temporizador, transições Kanban, discrepância de ciclo e contrato `IPomodoroPlugin` |
| `test_plugin_feynman` | [`tests/test_plugin_feynman.cpp`](../tests/test_plugin_feynman.cpp) | 4 etapas Feynman, persistência de conceitos, prioridades e contrato `IFeynmanPlugin` |
| `test_plugin_mapamental` | [`tests/test_plugin_mapamental.cpp`](../tests/test_plugin_mapamental.cpp) | Grafo de conceitos, serialização JSON, heurística NLP/TF-IDF, extratores ZIP/XML/CSV e `IMapaMentalPlugin` |
| `test_plugin_modelo` | [`tests/test_plugin_modelo.cpp`](../tests/test_plugin_modelo.cpp) | Prompts hierárquicos Qwen, detecção `llama.cpp` / Ollama e contrato `IModeloPlugin` |
| `test_plugin_audiolivro` | [`tests/test_plugin_audiolivro.cpp`](../tests/test_plugin_audiolivro.cpp) | `audiolivro_settings`, estado de leitura texto/PDF, `FocusRuler` e contrato `IAudiolivroPlugin` |
| `test_plugin_agenda` | [`tests/test_plugin_agenda.cpp`](../tests/test_plugin_agenda.cpp) | `listas.db` SQLite, agendas personalizadas aninhadas, modos `Preto`/`Coloridas` e `IAgendaPlugin` |
| `test_plugin_gantt` | [`tests/test_plugin_gantt.cpp`](../tests/test_plugin_gantt.cpp) | `DependencyValidator` DAG/ciclos, `CpmEngine`, `GanttDatabase`, `Bim4DService`, `SecurityService` RBAC e `IGanttPlugin` |
| `test_frameworks_plugins` | [`tests/test_frameworks_plugins.cpp`](../tests/test_frameworks_plugins.cpp) | Qt `6.12.x`, `QSQLITE`, `Pdf`/`Multimedia`/`WebSockets`, 14 `.qm`, carga dinâmica `QLibrary` das 7 DLLs, `LICENSE` e `docs/` |

---

## 4. Limitações de Maturidade e Conferência Obrigatória ("AS IS")

> [!WARNING]
> **Caráter Auxiliar da Validação CTest:** A aprovação de 11/11 executáveis na suíte CTest atesta consistência de regressão dos casos implementados e **não constitui certificação regulatória, clínica ou profissional definitiva**. No módulo **Gantt & BIM 4D**, cronogramas executivos, dependências e caminhos críticos exigem validação por profissional habilitado com **ART (CREA)** ou **RRT (CAU)** (**Leis nº 5.194/1966, nº 6.496/1977 e nº 12.378/2010**).

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details open>
<summary>Click to expand / collapse compliance in English</summary><br>

# EISENHOWER ORGANIZER / LÚMEN — Algorithmic Compliance, Accessibility & Verification

**Version:** `2026.11.22.0`  
**Date:** `November 22, 2026`  
**Author:** Fernando Nillsson Cidade  

---

## 1. Algorithmic Foundations (`Gantt CPM`, `TaskProgressHelper`, `Mind Map NLP`)

1. **DAG Validation & Critical Path Method (`DependencyValidator` & `CpmEngine`):** Blocks cyclic graphs across `FS`, `SS`, `FF`, `SF` links before executing calendar-aware Forward Pass ($ES, EF$), Backward Pass ($LS, LF$), Total Float ($TF = LS - ES$), and Critical Path identification ($TF = 0$).
2. **Bounded Progress & Heuristic Fallback:** `TaskProgressHelper` safely bounds 0–100% completion across Pomodoro cycles and Gantt physical progress without divide-by-zero edge cases; Mind Map runs local TF-IDF heuristic extraction when offline without a GGUF/Ollama model.

## 2. Transactional Storage & Cognitive Accessibility

- **Storage Invariants:** `UnifiedTaskStore` (`QLockFile` + `QSaveFile` + `.bak`) and parameterized SQLite (`listas.db`, `gantt_database.db`).
- **Accessibility (Lúmen / Audiobook / Store ADA):** Documented in [`MS-Store-ADA-v8.9-FINAL-EN.pdf`](../source/assets/LICENSES/MS-Store-ADA-v8.9-FINAL-EN.pdf); includes ADHD/dyslexia/ASD typography controls, synchronized word/sentence/paragraph speech highlight, and line-by-line **Focus Ruler**.

## 3. Unified 11-Target CTest Verification (`Qt6: Compilar e Executar SUITE (MSVC)` / `Qt6: Compilar e Executar SUITE (MinGW)`)

Every subsystem is continuously verified across all 11 Qt Test binaries (`test_core_utils`, `test_store_models`, `test_ui_host`, `test_plugin_pomodoro`, `test_plugin_feynman`, `test_plugin_mapamental`, `test_plugin_modelo`, `test_plugin_audiolivro`, `test_plugin_agenda`, `test_plugin_gantt`, `test_frameworks_plugins`) passing **11/11** under both Pure MSVC 14.52 and MinGW 15.1.0.

## 4. Auxiliary Scope Disclaimer ("AS IS")

Passing regression tests does not constitute medical/clinical certification (Audiobook/Lúmen) or definitive engineering scheduling sign-off (`4D BIM Gantt` schedules remain auxiliary aids requiring independent human review and licensed **ART/RRT** oversight where legally mandated).

</details>
