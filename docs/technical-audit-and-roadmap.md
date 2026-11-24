# Auditoria Técnica, ADRs, Matriz V&V e Roadmap / Technical Audit, ADRs & Roadmap — EISENHOWER ORGANIZER / LÚMEN

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details open>
<summary>Clique para expandir / recolher auditoria técnica e roadmap em português</summary><br>

# Auditoria Técnica, Decisões Arquiteturais (ADRs) e Matriz V&V

**Versão:** `2026.11.22.0`  
**Data:** `22 de novembro de 2026`  
**Autor:** Fernando Nillsson Cidade  

---

## A. Inventário Funcional e Classificação por Módulo

| Módulo / Componente | Escopo Implementado | Classificação Atual | Evidência em Código / Testes | Observações |
|---|---|---|---|---|
| **Eisenhower Host & `AppConfig`** | Identidade dual `EISENHOWER`/`LUMEN`, 4 quadrantes, `2x2` + Abas Verticais/Horizontais recolhíveis (90°), Calendário, Recorrências, Lembretes, `JSON/CSV/XLSX/PDF` | Implementado, Integrado, Testado, Documentado | `source/app/*`, `source/utils/AppConfig.hpp`, `test_core_utils`, `test_ui_host` | Persistência por produto em `%LOCALAPPDATA%` |
| **Persistência `UnifiedTaskStore`** | Escrita atômica `QSaveFile`, trava `QLockFile`, backup `.bak`, sincronização multithread/módulos | Implementado, Integrado, Testado, Documentado | `source/utils/UnifiedTaskStore.*`, `test_store_models` | Proteção contra sobrescrita de JSON corrompido |
| **Módulo Pomodoro** | Timer FSM, Kanban `Todo/Doing/Done`, alarme `Qt6::Multimedia`, `CycleMismatchDialog` | Implementado, Integrado, Testado, Documentado | `source/modules/pomodoro/*`, `test_plugin_pomodoro` | Sincronização bidirecional com o host |
| **Módulo Feynman** | 4 estágios de estudo ativo, worker assíncrono, vínculos externos, seleção em lote | Implementado, Integrado, Testado, Documentado | `source/modules/feynman/*`, `test_plugin_feynman` | Requer `integracao_time` válida na normalização |
| **Módulo Mapa Mental** | Grafo visual, extratores `PDF/EPUB/DOCX/XLSX/CSV/JSON/XML/SQLite`, motor TF-IDF + exportação PNG/JSON | Implementado, Integrado, Testado, Documentado | `source/modules/mapamental/*`, `test_plugin_mapamental` | Usa `Qt6::CorePrivate::QZipReader` |
| **Módulo Modelo (IA Local)** | `ModelDownloadDialog` (Hugging Face Range), `llama.cpp b8987` CPU/Vulkan, `OllamaInstrucoesDialog` | Implementado, Integrado, Testado, Documentado | `source/modules/modelo/*`, `test_plugin_modelo` | ~2,17 GB GGUF baixado apenas sob demanda |
| **Módulo Audiolivro** | Leitor Texto/`Qt6::Pdf`, `FocusRuler`, SAPI offline + `Qt6::WebSockets` Edge TTS (300+ vozes) | Implementado, Integrado, Testado, Documentado | `source/modules/audiolivro/*`, `test_plugin_audiolivro` | Edge TTS requer conexão opcional iniciada pelo usuário |
| **Módulo Agenda** | Banco SQLite `listas.db`, agendas aninhadas pai-filho, modos `Preto`/`Coloridas`, vínculos | Implementado, Integrado, Testado, Documentado | `source/modules/agenda/*`, `test_plugin_agenda` | Sincroniza exclusões de listas com barra de menus |
| **Módulo Gantt BIM 4D** | WBS, `DependencyValidator` DAG, `CpmEngine`, `Bim4DService`, MS Project XML, CSV, `FieldProgressService`, `AuditService`, `SecurityService` RBAC + HTTP REST | Implementado, Integrado, Testado, Documentado | `source/modules/gantt/*`, `test_plugin_gantt` | Suporta carregamento standalone DLL (`GanttPlugin.dll`) e in-process |
| **Governança Dual & Store** | `LICENSE` GNU AGPLv3 + `source/assets/{eisenhower,lumen}/` (`EULA`/`CLC`/`Privacy`) + `OpcoesSobre` | Implementado, Integrado, Testado, Documentado | `LICENSE`, `docs/*`, `source/assets/*`, `source/ui/ui_02_OpcoesSobre.cpp`, `test_frameworks_plugins` | Marcação explícita da liberdade em `source/assets/` |

---

## B. Registro de Decisões Arquiteturais (ADRs)

- **ADR-01 — Compilação Dual Unificada via `BUILD_TARGET` (`AppConfig.hpp`):** Em vez de bifurcar repositórios para **Eisenhower Organizer** e **Lúmen**, a compilação condicional `APP_EISENHOWER`/`APP_LUMEN` garante reaproveitamento integral de melhorias de UI, persistência, acessibilidade e testes mantendo pastas `%LOCALAPPDATA%` e ativos `source/assets/{eisenhower,lumen}/` estritamente separados.
- **ADR-02 — Consolidação Nativa + Preservação de Contratos DLL (`tests/` & `source/modules/`):** Os 7 módulos operam nativamente integrados ao host para inicialização instantânea e sincronização em tempo real via `UnifiedTaskStore`, enquanto o CMake mantém os alvos modulares independentes (`*Plugin.dll`) verificados via `QLibrary` em `test_frameworks_plugins`.
- **ADR-03 — IA Local Sob Demanda em Vez de Peso Pré-Embutido:** Retirar o arquivo `qwen2.5-3b-instruct-q4_k_m.gguf` (~2,17 GB) do pacote de instalação reduziu drasticamente o tamanho dos instaladores, delegando ao `ModelDownloadDialog` (HTTP Range) e ao assistente `Ollama` a escolha consciente pelo usuário.
- **ADR-04 — Pure MSVC (`Visual Studio 18 2026`) + CRT App-Local Sem `vc_redist.x64.exe`:** A adoção de builds Pure MSVC (`build_*_msvc`) combinadas com implantação apenas das 5 DLLs app-local do VC++ Runtime (`msvcp140*.dll`, `vcruntime140*.dll`) eliminou ~18 MB redundantes dos instaladores Inno Setup e manteve paridade limpa com MinGW `15.1.0`.
- **ADR-05 — Unificação Total da Suíte de Testes em `build_suite_testes_{msvc,mingw}`:** Consolidar os testes antigos e os 11 novos executáveis Qt Test sob um único `EisenhowerTestCore` estático (sem distribuição externa e com trava CMake contra `build_tests_*`) eliminou fragmentação de builds e manteve execução completa por 1 tarefa (`Qt6: Compilar e Executar SUITE (MSVC)` / `Qt6: Compilar e Executar SUITE (MinGW)`).
- **ADR-06 — Modelo de Duplo Licenciamento (`GNU AGPLv3` + `EULA`/`CLC`):** Separar explicitamente o código-fonte público aberto sob `GNU AGPLv3` (com liberdade total para forks adaptarem `source/assets/` após remoção das marcas exclusivas) das edições oficiais assinadas na Microsoft Store resguarda o ecossistema livre sem inviabilizar a sustentabilidade comercial oficial na Store.

---

## C. Matriz de Verificação & Validação (V&V) — 11 Suítes CTest

| ID | Alvo CTest | Toolchains Validados | Critério Objetivo de Aceitação |
|---|---|---|---|
| **V1** | `test_core_utils` | Pure MSVC `14.52` + MinGW `15.1.0` | `AppConfig`, `TrialManager` 7 dias, `LogManager`, `TaskProgressHelper` aprovados (0 falhas) |
| **V2** | `test_store_models` | Pure MSVC `14.52` + MinGW `15.1.0` | Atomicidade `UnifiedTaskStore`, filtros, recorrências e parsing de datas aprovados |
| **V3** | `test_ui_host` | Pure MSVC `14.52` + MinGW `15.1.0` | `EisenhowerMatrixApp` e carregamento completo de `OpcoesSobre` para Eisenhower & Lúmen (`pt_BR`/`en_US`) |
| **V4–V10** | `test_plugin_{pomodoro,feynman,mapamental,modelo,audiolivro,agenda,gantt}` | Pure MSVC `14.52` + MinGW `15.1.0` | Todos os 7 fluxos funcionais e interfaces de plugin aprovados isoladamente |
| **V11** | `test_frameworks_plugins` | Pure MSVC `14.52` + MinGW `15.1.0` | Qt `6.12.x`, drivers SQL/PDF/WebSockets/Multimedia, 14 `.qm`, 7 DLLs via `QLibrary`, `LICENSE` e `docs/` verificados |

---

## D. Roadmap Técnico Curado

- **Concluído (`v2026.11.22.0`):** C++23 + Qt 6.12.0, Pure MSVC & MinGW 15.1.0, distribuições limpas sem `vc_redist.x64.exe`, suíte unificada 11/11 CTest, IA sob demanda Qwen/Ollama, Gantt BIM 4D, documentação completa + Duplo Licenciamento, consolidação `library/` / `images/interfaces/` / `source/windows_app_icon.rc.in` e automação de `256` alvos no `find_VersionEditor`.
- **Próximas Frentes (`docs/suggested-issues.md`):** Métricas EVM no Gantt, TTS neural offline ONNX/Piper no Audiolivro, exportação `.ics`, exportação `.svg` no Mapa Mental, presets WCAG AAA no Lúmen e portabilidade CMake Linux.

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details open>
<summary>Click to expand / collapse technical audit and roadmap in English</summary><br>

# Technical Audit, Architecture Decision Records (ADRs) & V&V Matrix

**Version:** `2026.11.22.0`  
**Date:** `November 22, 2026`  
**Author:** Fernando Nillsson Cidade  

---

## A. Module Inventory & Verification Summary

Every core component — `Eisenhower Host` (`AppConfig` dual build target), `UnifiedTaskStore` (`QLockFile` + `QSaveFile`), `Pomodoro`, `Feynman`, `Mind Map`, `Modelo` (`ModelDownloadDialog` + `llama.cpp b8987` + `Ollama`), `Audiobook` (`FocusRuler` + SAPI/EdgeTTS), `Agenda` (`listas.db`), `4D BIM Gantt` (`DependencyValidator`, `CpmEngine`, `Bim4DService`, `SecurityService`), and Dual-Licensing assets — is **Implemented, Integrated, Tested (11/11 CTest on MSVC & MinGW), and Documented**.

## B. Architecture Decision Records (ADRs)

- **ADR-01 — Unified Dual-Brand Build Target (`BUILD_TARGET=EISENHOWER|LUMEN`):** Single shared codebase cleanly separating product icons, `%LOCALAPPDATA%` stores, logs, and `source/assets/{eisenhower,lumen}/`.
- **ADR-02 — Native Integration with Preserved Standalone DLL Contracts:** Fast in-process host startup paired with verified `QLibrary` loading of standalone plugin DLLs (`test_frameworks_plugins`).
- **ADR-03 — On-Demand Local AI Weights:** Eliminates ~2.17 GB pre-bundled installer payload via Hugging Face HTTP Range downloading and local `Ollama` integration.
- **ADR-04 — Pure MSVC + App-Local CRT Without `vc_redist.x64.exe`:** Cuts ~18 MB of redundant redistributable bloat from Inno Setup MSVC packages.
- **ADR-05 — Unified Local Test Suite (`build_suite_testes_{msvc,mingw}`):** Consolidates all 11 Qt Test binaries under static `EisenhowerTestCore` with zero external distribution and CMake guards against legacy `build_tests_*` folders.
- **ADR-06 — Transparent Dual Licensing (`GNU AGPLv3` + Official Store `EULA`/`CLC`):** Preserves open-source copyleft freedom (including fork customization of `source/assets/` after removing exclusive author trademarks) alongside commercial/Store protection for signed official releases.

## C. V&V Matrix & Roadmap

All 11 CTest suites (`test_core_utils`, `test_store_models`, `test_ui_host`, `test_plugin_*`, `test_frameworks_plugins`) pass on both Pure MSVC 14.52 and MinGW 15.1.0, completing the `v2026.11.22.0` consolidation milestone (including `library/` / `images/interfaces/` / `source/windows_app_icon.rc.in` standardization and 256-target `find_VersionEditor` tracking). Forward roadmap items are tracked in [`docs/suggested-issues.md`](suggested-issues.md#enus).

</details>
