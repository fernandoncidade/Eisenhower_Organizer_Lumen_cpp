# Sugestões de Issues e Roadmap / Suggested Issues & Roadmap — EISENHOWER ORGANIZER / LÚMEN

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details open>
<summary>Clique para expandir / recolher sugestões em português</summary><br>

# EISENHOWER ORGANIZER / LÚMEN — Backlog Técnico e Sugestões para Contribuidores

**Versão:** `2026.11.22.0`  
**Data:** `22 de novembro de 2026`  

Este documento reúne propostas técnicas e frentes de evolução mapeadas para orientar colaboradores e desenvolvedores que desejam contribuir com o ecossistema **Eisenhower Organizer / Lúmen** sob a licença **GNU AGPLv3**. Por se tratar de um projeto em evolução contínua com ferramentas auxiliares sujeitas à conferência independente ("AS IS"), contribuições voltadas à robustez algorítmica, acessibilidade cognitiva e testes automatizados na suíte `tests/` são especialmente bem-vindas.

---

### Issue 1: Análise de Valor Agregado (EVM) e Nivelamento de Recursos no Cronograma Gantt BIM 4D
- **Área:** `source/modules/gantt` (`CpmEngine` / `GanttDatabase`)
- **Dificuldade:** Avançada
- **Descrição:** Expandir o motor `CpmEngine` e o esquema `gantt_database.db` com métricas de **Valor Agregado (EVM)** — `PV`, `EV`, `AC`, `CPI`, `SPI` e curvas S exportáveis — além de heurística de nivelamento de recursos sem alterar dependências críticas (`TI`/`II`/`TT`/`IT`).
- **Critério de Aceitação:** Cobertura completa por novos casos determinísticos em [`tests/test_plugin_gantt.cpp`](../tests/test_plugin_gantt.cpp) aprovados em `Qt6: Compilar e Executar SUITE (MSVC)` e `Qt6: Compilar e Executar SUITE (MinGW)`.

---

### Issue 2: Síntese de Voz Neural 100% Offline (ONNX / Piper) no Audiolivro
- **Área:** `source/modules/audiolivro`
- **Dificuldade:** Avançada
- **Descrição:** Complementar as vozes locais SAPI do Windows e as vozes online Edge TTS com suporte opcional sob demanda a modelos neurais leves offline (ex.: Piper / ONNX Runtime), mantendo paridade de realce palavra por palavra e funcionamento na Régua de Foco sem depender de conexão externa.
- **Critério de Aceitação:** Download gerenciável sob demanda (seguindo o padrão leve do `ModelDownloadDialog`) e preservação do controle de armazenamento pelo usuário.

---

### Issue 3: Exportação e Importação Bidirecional `.ics` (iCalendar RFC 5545) no Calendário e Agenda
- **Área:** `source/ui/CalendarDialog` / `source/modules/agenda`
- **Dificuldade:** Média
- **Descrição:** Permitir exportar tarefas com prazo, recorrências e avaliações de `listas.db` para arquivos padrão `.ics` (iCalendar RFC 5545), além de importar compromissos acadêmicos locais preservando a filosofia *Local-First* sem sincronização automática em nuvem.
- **Critério de Aceitação:** Testes de *round-trip* com fusos horários e acentuação UTF-8 em `test_plugin_agenda` e `test_store_models`.

---

### Issue 4: Indexação Espacial e Exportação Vetorial SVG no Mapa Mental
- **Área:** `source/modules/mapamental`
- **Dificuldade:** Média
- **Descrição:** Além da exportação PNG existente, implementar exportação vetorial `.svg` em alta resolução com preservação de conectores e cores de prioridade, combinada com busca instantânea em mapas com milhares de nós.
- **Critério de Aceitação:** Validação estrutural do SVG gerado (XML bem-formado e preservação de rótulos UTF-8) em `test_plugin_mapamental`.

---

### Issue 5: Estatísticas Semanais/Mensais de Foco e Ciclos Concluídos no Pomodoro
- **Área:** `source/modules/pomodoro`
- **Dificuldade:** Média
- **Descrição:** Adicionar painel local de histórico e métricas de foco (`min/dia`, taxa de conclusão vs meta de ciclos 🍅/🎯, distribuição por quadrante Eisenhower), persistido em JSON atômico (`QSaveFile`) sem envio externo.
- **Critério de Aceitação:** Consistência após fechamento/reabertura e tradução dinâmica `pt_BR` / `en_US` verificadas em `test_plugin_pomodoro`.

---

### Issue 6: Perfis Tipográficos e Temas de Alto Contraste WCAG AAA para TDAH, Dislexia e TEA (Lúmen)
- **Área:** `source/app/` / `source/utils/FontUtils.hpp`
- **Dificuldade:** Baixa / Média
- **Descrição:** Adicionar presets de leitura e acessibilidade com espaçamento ampliado entre letras/linhas, paletas de contraste WCAG AAA e redução opcional de animações no módulo Lúmen e na Régua de Foco do Audiolivro.
- **Critério de Aceitação:** Verificação visual nos três layouts da Matriz de Eisenhower e atualização ao vivo via `changeEvent`.

---

### Issue 7: Fuzzing e Testes de Estresse nos Importadores de Documentos (`EPUB`/`DOCX`/`XLSX`/`XML`/`PDF`)
- **Área:** `tests/` / `source/modules/mapamental` / `source/modules/gantt`
- **Dificuldade:** Média / Avançada
- **Descrição:** Construir bateria de testes adversariais com arquivos ZIP malformados, XMLs profundamente aninhados, CSVs com prefixos de fórmula e PDFs corrompidos para assegurar rejeição segura sem vazamento de memória ou travamentos.
- **Critério de Aceitação:** Execução determinística na suíte `Qt6: Compilar e Executar SUITE (MSVC)` / `Qt6: Compilar e Executar SUITE (MinGW)`.

---

### Issue 8: Empacotamento e Suporte CMake Cross-Platform para Linux (AppImage / Flatpak)
- **Área:** `CMakeLists.txt` / `CMakePresets.json`
- **Dificuldade:** Avançada
- **Descrição:** Desacoplar guardas específicos do Windows (registro/Ícones `.ico`/TTS SAPI) por abstrações condicionais de plataforma, permitindo compilar ambos os alvos `EISENHOWER` e `LUMEN` nativamente em Linux com Qt 6.12.0 + GCC/Clang mantendo 100% da suíte CTest aprovada.
- **Critério de Aceitação:** Presets Linux no `CMakePresets.json` e compatibilidade com a suíte completa `tests/`.

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details open>
<summary>Click to expand / collapse suggested issues in English</summary><br>

# EISENHOWER ORGANIZER / LÚMEN — Technical Backlog & Contributor Suggestions

**Version:** `2026.11.22.0`  
**Date:** `November 22, 2026`  

This document highlights mapped engineering tasks and enhancement opportunities for contributors collaborating on **Eisenhower Organizer / Lúmen** under **GNU AGPLv3**. Because modules are auxiliary tools provided "AS IS", contributions strengthening algorithmic precision, cognitive accessibility, input validation, and `tests/` coverage are especially appreciated.

---

### Issue 1: Earned Value Management (EVM) & Resource Leveling in 4D BIM Gantt
- **Area:** `source/modules/gantt` (`CpmEngine` / `GanttDatabase`)
- **Difficulty:** Advanced
- **Description:** Extend `CpmEngine` and `gantt_database.db` with EVM metrics (`PV`, `EV`, `AC`, `CPI`, `SPI`, exportable S-curves) and resource leveling heuristics while preserving DAG precedence invariants (`FS`/`SS`/`FF`/`SF`).
- **Acceptance Criteria:** Deterministic unit tests inside [`tests/test_plugin_gantt.cpp`](../tests/test_plugin_gantt.cpp) passing on both `Qt6: Compilar e Executar SUITE (MSVC)` and `Qt6: Compilar e Executar SUITE (MinGW)`.

---

### Issue 2: 100% Offline Neural Voice Synthesis (ONNX / Piper) in Audiobook
- **Area:** `source/modules/audiolivro`
- **Difficulty:** Advanced
- **Description:** Complement Windows SAPI local voices and optional online Edge TTS voices with an optional on-demand offline neural TTS runtime (e.g., Piper / ONNX Runtime) keeping word-level Focus Ruler highlight synchronization fully offline.
- **Acceptance Criteria:** On-demand weight download matching `ModelDownloadDialog` UX and user disk reclamation controls.

---

### Issue 3: Bidirectional `.ics` (iCalendar RFC 5545) Import/Export in Calendar & Agenda
- **Area:** `source/ui/CalendarDialog` / `source/modules/agenda`
- **Difficulty:** Medium
- **Description:** Support exporting due tasks, recurrences, and academic deadlines from `listas.db` into standard `.ics` files and importing local calendar events without any cloud sync dependency.
- **Acceptance Criteria:** UTF-8 and timezone round-trip tests in `test_plugin_agenda` and `test_store_models`.

---

### Issue 4: Vector SVG Export & Spatial Indexing in Mind Map
- **Area:** `source/modules/mapamental`
- **Difficulty:** Medium
- **Description:** Add crisp resolution-independent `.svg` vector graph export alongside PNG export, plus spatial indexing for graphs containing thousands of linked nodes.
- **Acceptance Criteria:** Structural XML/UTF-8 SVG output checks inside `test_plugin_mapamental`.

---

### Issue 5: Local Focus Analytics & Cycle History Dashboard in Pomodoro
- **Area:** `source/modules/pomodoro`
- **Difficulty:** Medium
- **Description:** Provide local daily/weekly focus analytics (minutes worked, 🍅/🎯 completion ratio, Eisenhower quadrant distribution) persisted atomically with `QSaveFile`.
- **Acceptance Criteria:** Restart persistence and dynamic `pt_BR` / `en_US` translation tested in `test_plugin_pomodoro`.

---

### Issue 6: High-Contrast WCAG AAA & Neurodiverse Reading Presets for Lúmen
- **Area:** `source/app/` / `source/utils/FontUtils.hpp`
- **Difficulty:** Low / Medium
- **Description:** Add accessible presets (letter/line spacing profiles, WCAG AAA contrast themes, reduced motion toggles) across Lúmen layouts and Audiobook Focus Ruler.
- **Acceptance Criteria:** Live verification via `changeEvent` without restarts.

---

### Issue 7: Adversarial Parser & Importer Stress Tests (`EPUB`/`DOCX`/`XLSX`/`XML`/`PDF`)
- **Area:** `tests/` / `source/modules/mapamental` / `source/modules/gantt`
- **Difficulty:** Medium / Advanced
- **Description:** Add adversarial test fixtures covering malformed ZIP archives, deeply nested XML, formula-prefixed CSV cells, and corrupt PDFs to verify clean error handling without leaks or crashes.
- **Acceptance Criteria:** Clean execution inside `Qt6: Compilar e Executar SUITE (MSVC)` and `Qt6: Compilar e Executar SUITE (MinGW)`.

---

### Issue 8: Native Linux CMake Build & Packaging Presets (AppImage / Flatpak)
- **Area:** `CMakeLists.txt` / `CMakePresets.json`
- **Difficulty:** Advanced
- **Description:** Abstract Windows-specific APIs (SAPI TTS fallback, `.ico` resources) cleanly so both `EISENHOWER` and `LUMEN` build and pass all 11 CTest targets natively on Linux with Qt 6.12.0.
- **Acceptance Criteria:** Linux presets in `CMakePresets.json` passing the full test suite.

</details>
