<!-- Multilanguage OBSERVACAO.md for Eisenhower_Organizer_Lumen_cpp -->

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> |
  <a href="#enus">🇺🇸 English (US)</a> |
  <a href="#tree">🌳 Estrutura do Repositório / Directory Tree</a>
</p>

---

<h2 id="ptbr">🇧🇷 Português (BR)</h2>

<details>
<summary>Clique para expandir / recolher a Observação Técnica em português</summary>

# OBSERVAÇÃO TÉCNICA E POSICIONAMENTO ESTRATÉGICO — EISENHOWER ORGANIZER / LÚMEN

Versão documentada: `2026.11.22.0`
Data do snapshot técnico: `22 de novembro de 2026`
Repositório: `Eisenhower_Organizer_Lumen_cpp`
Autor identificado no projeto: `Fernando Nillsson Cidade`
Licença do repositório (código-fonte): `GNU AGPLv3 (GNU Affero General Public License v3)`
Licença das edições oficiais Microsoft Store: `EULA Oficial + CLC do Autor`

## 1. Estado Atual do Projeto e Posicionamento Estratégico

O repositório **Eisenhower Organizer / Lúmen** consolida uma suíte desktop nativa de produtividade, estudo ativo, acessibilidade cognitiva e gestão temporal de projetos construída em **C++23 com Qt 6.12.0**, estruturada para gerar duas identidades oficiais a partir da mesma arquitetura modular (`BUILD_TARGET=EISENHOWER` e `BUILD_TARGET=LUMEN`):

- **EISENHOWER ORGANIZER (`EisenhowerOrganizer.exe` — Store ID `9P289X0185C3`):** Organização de tarefas por matriz de prioridade, execução focada, agenda avaliativa e planejamento de cronogramas / engenharia 4D BIM.
- **LÚMEN (`Lumen.exe` — Store ID `9N70CLLMVRPN`):** Plataforma de estudos e acessibilidade com atenção dedicada a usuários com TDAH, dislexia e TEA, reunindo todos os 7 módulos nativos e recursos assistidos (Régua de Foco, realce sincronizado de fala, layouts simplificados e IA local sob demanda).

A arquitetura adota o paradigma **Local-First**: os dados do usuário (`tarefas.json`, `conceitos_feynman.json`, `mapa_mental_autosave.json`, `listas.db`, `gantt_database.db` e preferências) permanecem no dispositivo local (`%LOCALAPPDATA%\EisenhowerOrganizer` ou `%LOCALAPPDATA%\Lumen`), sem contas obrigatórias e sem telemetria. Comunicações de rede ocorrem estritamente sob demanda quando acionadas pelo usuário (download de peso `.gguf` via Hugging Face com retomada HTTP Range, servidor local Ollama `127.0.0.1:11434`, vozes online opcionais Edge TTS no Audiolivro ou servidor HTTP/Webhooks locais opcionais no Gantt).

## 2. Destaques Técnicos Consolidados na Revisão `2026.11.22.0`

- **Migração Integral para C++23, Qt 6.12.0 e Dual Toolchain Pure MSVC + MinGW 15.1.0:**
  - Código padronizado em **C++23 estrito** (`CMAKE_CXX_STANDARD 23`, compatibilidade limpa sem uso de palavras-chave reservadas como `concept`).
  - Atualização de todas as presets e tarefas para **Qt 6.12.0** (`msvc2022_64` e `mingw_64`), **Pure MSVC x64** (`Visual Studio 18 2026`, `MSVC 14.52.37025`, SDK `10.0.28000.0`, árvores `build/build_*_msvc`) e **MinGW-w64 `15.1.0` + Ninja** (`build/build_*_mingw`).
- **Distribuições Externas Limpas & Instaladores Inno Setup Compactos (Sem `vc_redist.x64.exe`):**
  - Implantação em `D:\MISCELANEAS\VSCode\EisenhowerOrganizer\Eisenhower_Organizer_cpp_{msvc,mingw}` e `D:\MISCELANEAS\VSCode\Lumen\Lumen_cpp_{msvc,mingw}` contendo estritamente executáveis, traduções `.qm`, ícones de runtime, `assets/` em tempo de execução (`cmake/copy_app_assets.cmake`) e DLLs importadas.
  - Scripts `library/Eisenhower_Organizer*.iss` e `library/Lumen*.iss` padronizados com *app-local CRT deployment* (5 DLLs VC++ Runtime no MSVC, sem `vc_redist.x64.exe`), mantendo os instaladores MSVC (~`66,8 MB`) equivalentes/menores aos instaladores MinGW (~`73,3 MB`).
- **Suíte Unificada de Engenharia de Software (`tests/` — 11 Executáveis CTest):**
  - Toda a cobertura de testes foi consolidada em [`tests/`](tests/CMakeLists.txt) e compilada exclusivamente em `build/build_suite_testes_msvc` e `build/build_suite_testes_mingw` (tarefas VS Code `"Qt6: Compilar e Executar SUITE (MSVC)"` e `"Qt6: Compilar e Executar SUITE (MinGW)"`), **sem distribuição externa** e com trava CMake `FATAL_ERROR` impedindo recriação de `build_tests_{msvc,mingw}`:
    - `test_core_utils`, `test_store_models`, `test_ui_host`, `test_plugin_pomodoro`, `test_plugin_feynman`, `test_plugin_mapamental`, `test_plugin_modelo`, `test_plugin_audiolivro`, `test_plugin_agenda`, `test_plugin_gantt` e `test_frameworks_plugins` (**11/11 aprovados** em MSVC e MinGW).
- **Arquitetura Leve de IA Local Sob Demanda (`Modelo`):**
  - Remoção do peso pré-embutido de ~2,17 GB dos instaladores; gerenciamento interativo de download via `ModelDownloadDialog` (Hugging Face `qwen2.5-3b-instruct-q4_k_m.gguf` com HTTP Range pausa/retomada e exclusão física), runtime offline `llama.cpp b8987` (`CPU` e `Vulkan`), assistente/detecção automática de `Ollama` (`OllamaInstrucoesDialog`) e fallback heurístico transparente no `Mapa Mental`.
- **Evolução Acumulada dos 7 Módulos (Histórico Técnico `2026.5.x` – `2026.11.22.0`):**
  - Consolidação nativa de `Eisenhower`, `Pomodoro`, `Feynman`, `Mapa Mental`, `Audiolivro`, `Agenda` (`listas.db` + listas em árvore + vínculo de arquivos) e `Cronograma de Gantt BIM 4D` (`CpmEngine`, `DependencyValidator`, `Bim4DService`, `MsProjectXmlConnector`, `CsvExchangeService`, `FieldProgressService`, `AuditService`, `SecurityService` RBAC/rate-limit);
  - Layout Matriz `2x2` e abas verticais/horizontais recolhíveis (rotação 90°), lista `Em Progresso` em todos os quadrantes, `EditTaskDialog` responsivo com cálculo dinâmico de progresso (`TaskProgressHelper`), `CycleMismatchDialog` no Pomodoro, sincronização instantânea em segundo plano, logs centralizados (`LogManager`), tradução dinâmica completa em tempo de execução e fontes/cores configuráveis por módulo.

## 3. Resumo Estrutural das Camadas do Sistema

| Camada | Localização | Responsabilidade Técnica |
|---|---|---|
| **Aplicação Host** | `source/eisenhower_app.*`, `source/app/` | Orquestração principal, inicialização de identidade (`AppConfig`), barra de menus, abas de módulos, adição/edição de tarefas, temas e tradução dinâmica |
| **Persistência e Núcleo** | `source/utils/`, `source/models/` | `UnifiedTaskStore` (`QLockFile` + `QSaveFile`), `TrialManager`, `LogManager`, `PluginRegistry`, `TaskProgressHelper`, modelos de tarefas/filtros/recorrências |
| **Serviços e Diálogos UI** | `source/services/`, `source/ui/` | `CalendarDialog`, `EditTaskDialog`, `OpcoesSobre` (`source/assets/`), Gerenciador de Plugins, Gerenciador de Logs, exportação XLSX/PDF/CSV/JSON |
| **Módulos Integrados & IA** | `source/modules/` | `pomodoro/`, `feynman/`, `mapamental/`, `modelo/` (`llama.cpp` + `Ollama` + downloader), `audiolivro/`, `agenda/`, `gantt/` |
| **Internacionalização** | `source/language/` | Catálogos `.ts` e binários `.qm` (`pt_BR`, `en_US`) para host e todos os módulos |
| **Governança e Assets** | `source/assets/` | Documentação regulatória por produto (`eisenhower/`, `lumen/`) e licenças de terceiros (`LICENSES/` incluindo `GNU AGPLv3`) |
| **Utilitários Auxiliares** | `mocks/` | `find_VersionEditor`, `CompileTranslationsGUI`, `CompileTranslationsTerminal`, scripts de limpeza |
| **Testes Automatizados** | `tests/` | Suíte CTest com 11 executáveis Qt Test para host, persistência, UI, 7 módulos e runtimes Qt 6.12.0 |

## 4. Regime de Duplo Licenciamento e Escopo Auxiliar ("AS IS")

1. **Repositório Público (`GNU AGPLv3`):** Todo código-fonte C++23, scripts CMake, utilitários e testes seguem [`LICENSE`](LICENSE) (**GNU Affero General Public License v3**). Desenvolvedores têm autorização expressa do autor para alterar, remover ou adaptar `source/assets/` em seus próprios forks, desde que removam as marcas exclusivas `EISENHOWER ORGANIZER` e `LÚMEN`.
2. **Edições Oficiais Microsoft Store (`EULA` + `CLC`):** Os binários assinados pelo autor publicados na Microsoft Store (`9P289X0185C3` e `9N70CLLMVRPN`) operam sob o [EULA Oficial](source/assets/eisenhower/EULA/EULA_pt_BR.txt) e o [Contrato de Licenciamento Comercial](source/assets/eisenhower/CLC/CLC_pt_BR.txt), sendo proibida a revenda comercial não autorizada por terceiros.
3. **Natureza Auxiliar ("AS IS"):** Aplicação fornecida no estado em que se encontra, para apoio pessoal, acadêmico e de planejamento (inclusive Gantt BIM 4D e síntese de IA local), sem substituir conferência independente nem responsabilidade técnica legalmente habilitada (**ART/RRT**). Veja [`docs/licensing.md`](docs/licensing.md#ptbr).

</details>

---

<h2 id="enus">🇺🇸 English (US)</h2>

<details>
<summary>Click to expand / collapse Technical Observation in English</summary>

# TECHNICAL OBSERVATION AND STRATEGIC POSITIONING — EISENHOWER ORGANIZER / LÚMEN

Documented Version: `2026.11.22.0`
Technical Snapshot Date: `November 22, 2026`
Repository: `Eisenhower_Organizer_Lumen_cpp`
Author Identified in Project: `Fernando Nillsson Cidade`
Repository License (Source Code): `GNU AGPLv3 (GNU Affero General Public License v3)`
Official Microsoft Store Edition License: `Author's Official EULA + CLC`

## 1. Current Project State & Strategic Positioning

The **Eisenhower Organizer / Lúmen** repository provides a native Windows desktop productivity, active study, cognitive accessibility, and project scheduling suite engineered in **C++23 with Qt 6.12.0**, configured to build two official product identities from a unified modular architecture (`BUILD_TARGET=EISENHOWER` and `BUILD_TARGET=LUMEN`):

- **EISENHOWER ORGANIZER (`EisenhowerOrganizer.exe` — Store ID `9P289X0185C3`):** Matrix task prioritization, Pomodoro execution, academic activity planning, and 4D BIM project scheduling.
- **LÚMEN (`Lumen.exe` — Store ID `9N70CLLMVRPN`):** Study and cognitive-accessibility platform designed with special care for ADHD, dyslexia, and ASD learners, bundling all 7 native workspaces and assistive tools (Focus Ruler, synchronized speech highlighting, simplified layouts, and on-demand local AI).

The platform is architected on a **Local-First** foundation: all user data (`tarefas.json`, `conceitos_feynman.json`, `mapa_mental_autosave.json`, `listas.db`, `gantt_database.db`, and user settings) resides locally under `%LOCALAPPDATA%\EisenhowerOrganizer` or `%LOCALAPPDATA%\Lumen`, with zero mandatory accounts and zero telemetry. Network connections occur exclusively upon explicit user action (on-demand Hugging Face `.gguf` download with HTTP Range resume, local `127.0.0.1:11434` Ollama communication, optional Edge TTS neural voices in Audiobook, or user-enabled local Gantt REST/Webhook endpoints).

## 2. Consolidated Technical Highlights in Revision `2026.11.22.0`

- **Full Migration to C++23, Qt 6.12.0 & Dual Toolchains (Pure MSVC + MinGW 15.1.0):**
  - Strict **C++23** standard (`CMAKE_CXX_STANDARD 23`), audited against reserved keywords such as `concept`.
  - Presets and VS Code tasks updated for **Qt 6.12.0** (`msvc2022_64` and `mingw_64`), **Pure MSVC x64** (`Visual Studio 18 2026`, `MSVC 14.52.37025`, Windows SDK `10.0.28000.0`, `build/build_*_msvc`), and **MinGW-w64 `15.1.0` + Ninja** (`build/build_*_mingw`).
- **Clean External Distributions & Compact Inno Setup Installers (No `vc_redist.x64.exe`):**
  - Lean output packages under `D:\MISCELANEAS\VSCode\EisenhowerOrganizer\Eisenhower_Organizer_cpp_{msvc,mingw}` and `D:\MISCELANEAS\VSCode\Lumen\Lumen_cpp_{msvc,mingw}` shipping only executables, `.qm` translations, runtime icons, required runtime `assets/` (`cmake/copy_app_assets.cmake`), and imported runtime DLLs.
  - Inno Setup scripts (`library/Eisenhower_Organizer*.iss`, `library/Lumen*.iss`) enforce app-local VC++ CRT DLLs on MSVC without embedding `vc_redist.x64.exe`, keeping MSVC installers (~`66.8 MB`) smaller than MinGW installers (~`73.3 MB`).
- **Unified Software Engineering Test Suite (`tests/` — 11 CTest Executables):**
  - All unit and integration verification lives inside [`tests/`](tests/CMakeLists.txt) and compiles strictly under `build/build_suite_testes_msvc` and `build/build_suite_testes_mingw` (VS Code tasks `"Qt6: Compilar e Executar SUITE (MSVC)"` and `"Qt6: Compilar e Executar SUITE (MinGW)"`), with **no external distribution** and a `FATAL_ERROR` CMake guard blocking legacy `build_tests_{msvc,mingw}` paths: **11/11 tests passing** on both MSVC and MinGW.
- **Lightweight On-Demand Local AI Architecture (`Modelo`):**
  - Removes the ~2.17 GB pre-bundled weight bloat from installers; introduces `ModelDownloadDialog` (Hugging Face `qwen2.5-3b-instruct-q4_k_m.gguf` with HTTP Range pause/resume/deletion), `llama.cpp b8987` CPU/Vulkan runtimes, `OllamaInstrucoesDialog` local probing (`localhost:11434`), and seamless heuristic NLP/TF-IDF fallback in `Mapa Mental`.

## 3. Dual-Licensing Model & Auxiliary Disclaimer ("AS IS")

1. **Public Repository (`GNU AGPLv3`):** All C++23 source code, CMake configuration, utilities, and test suites follow [`LICENSE`](LICENSE) (**GNU Affero General Public License v3**). Fork maintainers have express permission from the author to edit, delete, or adapt `source/assets/` as long as the exclusive brands `EISENHOWER ORGANIZER` and `LÚMEN` are removed.
2. **Official Microsoft Store Editions (`EULA` + `CLC`):** Signed packages published by the author on the Microsoft Store (`9P289X0185C3` and `9N70CLLMVRPN`) operate under the author's [Official EULA](source/assets/eisenhower/EULA/EULA_en_US.txt) and [Commercial Licensing Agreement](source/assets/eisenhower/CLC/CLC_en_US.txt); third-party commercial resale or trademark misappropriation is prohibited.
3. **Auxiliary Scope ("AS IS"):** Software and modules (including Local AI synthesis and 4D BIM Gantt CPM scheduling) are auxiliary tools supplied "AS IS" without warranties and do not replace independent verification or legally licensed engineering oversight (**ART/RRT**). See [`docs/licensing.md`](docs/licensing.md#enus).

</details>

---

<h2 id="tree">🌳 Estrutura do Repositório / Repository Directory Tree</h2>

<details>
<summary>Clique para expandir / recolher a árvore principal / Click to expand directory tree</summary>

```text
Eisenhower_Organizer_Lumen_cpp/
├── .github/
│   ├── ISSUE_TEMPLATE/         # bug_report.md, feature_request.md, question.md
│   ├── PULL_REQUEST_TEMPLATE.md
│   └── workflows/              # update-readme-version.yml
├── .vscode/                    # tasks.json (hosts, 7 plugins, mocks, Qt6: Compilar e Executar SUITE (MSVC/MinGW))
├── cmake/                      # copy_app_assets.cmake, helpers de deploy e sanitização
├── docs/                       # Documentação técnica bilíngue (9 guias: architecture, licensing, privacy, etc.)
├── images/
│   └── interfaces/             # Capturas de tela das interfaces do aplicativo
├── library/                    # Scripts Inno Setup 7 (Eisenhower_Organizer*.iss, Lumen*.iss), manifests AppxManifest.xml e notas técnicas
├── mocks/                      # find_VersionEditor (256 arquivos), CompileTranslationsGUI/Terminal, limpeza de builds
├── source/
│   ├── app/                    # Implementações fragmentadas do EisenhowerMatrixApp
│   ├── assets/                 # Textos legais/institucionais separados para eisenhower/ e lumen/ + LICENSES/
│   ├── language/               # Catálogos .ts / .qm (pt_BR e en_US)
│   ├── models/                 # Modelos de dados e estado de tarefas
│   ├── modules/                # agenda/, audiolivro/, feynman/, gantt/, mapamental/, modelo/, pomodoro/
│   ├── services/               # Serviços transversais e construção de diálogos Sobre/Configurações
│   ├── ui/                     # CalendarDialog, EditTaskDialog, OpcoesSobre, estilos visuais
│   ├── utils/                  # AppConfig, UnifiedTaskStore, TrialManager, LogManager, TaskProgressHelper
│   └── windows_app_icon.rc.in  # Template de recurso Windows compartilhado para ícones dos executáveis
├── tests/                      # Suíte CTest com 11 executáveis Qt Test (sem distribuição externa)
├── CMakeLists.txt
├── CMakePresets.json
├── CODE_OF_CONDUCT.md
├── CONTRIBUTING.md
├── LICENSE                     # GNU AGPLv3 oficial
├── MANUAL.md                   # Manual operacional completo (pt-BR / en-US)
├── OBSERVACAO.md               # Este snapshot técnico
├── README.md                   # Portal principal do repositório
└── SECURITY.md                 # Política de divulgação responsável e garantias locais
```

</details>
