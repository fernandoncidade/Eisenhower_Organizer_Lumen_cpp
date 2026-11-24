# Notas de Lançamento / Release Notes — EISENHOWER ORGANIZER / LÚMEN v2026.11.22.0

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details open>
<summary>Clique para expandir / recolher notas de lançamento em português</summary><br>

# EISENHOWER ORGANIZER / LÚMEN — Versão 2026.11.22.0

**Data de Lançamento:** `22 de novembro de 2026`  
**Autor:** Fernando Nillsson Cidade (`linceu_lighthouse@outlook.com`)  
**Repositório:** [Eisenhower_Organizer_Lumen_cpp](https://github.com/fernandoncidade/Eisenhower_Organizer_Lumen_cpp)  
**Duplo Licenciamento:** `GNU AGPLv3` (Código-Fonte Aberto) / `EULA Oficial + CLC` (Microsoft Store: `9P289X0185C3` e `9N70CLLMVRPN`)  

---

## Principais Entregas Consolidadas

### 1. IA Local Sob Demanda (`Modelo`: Qwen 2.5 GGUF + Ollama)
- **Instalador Mais Leve (> 2 GB reduzidos):** Eliminação do modelo pré-embutido obrigatório no pacote de instalação.
- **Gerenciador Interativo de Download (`ModelDownloadDialog`):** Download direto do repositório oficial `Qwen/Qwen2.5-3B-Instruct-GGUF` (~2,17 GB) no Hugging Face com barra de progresso, velocidade (MB/s), pausa e retomada resilientes via HTTP Range, abertura da pasta e exclusão física imediata.
- **Status Dinâmico & Assistente Ollama (`OllamaInstrucoesDialog`):** Indicadores `[Instalado]` / `[Não instalado]` em tempo real no menu do Mapa Mental, comandos copiáveis com 1 clique (`llama3.2`, `qwen2.5:3b`, etc.), teste de conexão em `http://localhost:11434`, execução `llama.cpp b8987` (CPU/Vulkan) e contingência heurística automática caso nenhum LLM esteja presente.

### 2. Modernização C++23, Qt 6.12.0, Pure MSVC & MinGW-w64 15.1.0
- **Migração Integral para C++23 e Qt 6.12.0:** Padronização estrita do código para C++23 (`CMAKE_CXX_STANDARD 23`, incluindo higienização de identificadores como `concept` nos 7 módulos para conformidade MSVC/GCC C++23).
- **Builds Pure MSVC (`build_*_msvc`) e MinGW (`build_*_mingw`):** Gerador nativo `"Visual Studio 18 2026"` (`MSVC 14.52.37025`, Windows SDK `10.0.28000.0`) e `MinGW-w64 15.1.0` + Ninja `1.13.2`.
- **Distribuições Externas Limpas e Instaladores Enxutos (`Inno Setup`):** Pastas externas contendo estritamente os arquivos necessários ao funcionamento dos binários, e instaladores MSVC utilizando *app-local CRT deployment* (5 DLLs VC++ Runtime, **sem embutir `vc_redist.x64.exe`**), reduzindo o instalador MSVC para ~`66,8 MB` (menor que o MinGW de ~`73,3 MB`).

### 3. Suíte Unificada de Engenharia de Software (`tests/` — 11 Executáveis CTest)
- Criação e unificação completa da suíte em [`tests/`](../tests/CMakeLists.txt) compilada exclusivamente em `build/build_suite_testes_msvc` e `build/build_suite_testes_mingw` (tarefas `.vscode/tasks.json`: `"Qt6: Compilar e Executar SUITE (MSVC)"` e `"Qt6: Compilar e Executar SUITE (MinGW)"`), **sem distribuição externa** e com bloqueio CMake contra os antigos diretórios `build_tests_{msvc,mingw}`:
  - `test_core_utils`, `test_store_models`, `test_ui_host`, `test_plugin_pomodoro`, `test_plugin_feynman`, `test_plugin_mapamental`, `test_plugin_modelo`, `test_plugin_audiolivro`, `test_plugin_agenda`, `test_plugin_gantt`, `test_frameworks_plugins` (**11/11 aprovados em MSVC e MinGW**).

### 4. Evoluções dos 7 Módulos Nativos Integrados
- **Eisenhower Host:** Layouts `Matriz 2x2`, `Abas Vertical` e `Abas Horizontal` (colunas recolhíveis com rotação 90° e contorno contínuo), lista `Em Progresso` bidirecional, `EditTaskDialog` responsivo (`TaskProgressHelper`), selos de ciclo de vida no Calendário com abertura por duplo clique e salvamento assíncrono instantâneo.
- **Pomodoro:** Kanban sincronizado, modo automático com alarme configurável e `CycleMismatchDialog` de prevenção de divergência de metas.
- **Feynman & Mapa Mental:** Exclusão em lote preservando seleção múltipla, atualização instantânea de nós, fontes/cores independentes por módulo e importadores EPUB/DOCX/XLSX/PDF/CSV/JSON/XML/SQLite.
- **Audiolivro:** Catálogo estabilizado (300+ vozes com `QLocale`), realce sincronizado palavra/frase/parágrafo e Régua de Foco.
- **Agenda & Gantt BIM 4D:** Agendas personalizadas em árvore aninhada (`listas.db`), painel `Bim4DSimulationPanel`, conector MS Project XML (`MsProjectXmlConnector`), mapeador CSV (`CsvExchangeService`), calendário de dias úteis/feriados, `FieldProgressService`, `AuditService`, servidor REST + Webhooks + `SecurityService` RBAC e integração direta no menu de contexto (`Ctrl+Alt+G`).

### 5. Duplo Licenciamento e Conformidade Documental
- Implementação completa do modelo de **Duplo Licenciamento** (**GNU AGPLv3** no código-fonte público em [`LICENSE`](../LICENSE) + liberdade em `source/assets/` para forks + **EULA Oficial + CLC** nas edições Microsoft Store e comercial) em todo o repositório, `docs/*.md` e nos documentos internos de `source/assets/eisenhower/` e `source/assets/lumen/`.
- Padronização estrutural do repositório: centralização de scripts Inno Setup, manifestos MSIX (`E-AppxManifest.xml` / `L-AppxManifest.xml`) e notas técnicas em `library/`, consolidação de capturas de telas em `images/interfaces/`, movimentação do template de ícone Windows para [`source/windows_app_icon.rc.in`](../source/windows_app_icon.rc.in), remoção do guia redundante `docs/cli-reference.md` e evolução do utilitário [`find_VersionEditor.cpp`](../mocks/find_VersionEditor.cpp) cobrindo todos os `256` arquivos do projeto com renomeação automática de `docs/release-v*.md`.

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details open>
<summary>Click to expand / collapse release notes in English</summary><br>

# EISENHOWER ORGANIZER / LÚMEN — Version 2026.11.22.0

**Release Date:** `November 22, 2026`  
**Author:** Fernando Nillsson Cidade (`linceu_lighthouse@outlook.com`)  
**Repository:** [Eisenhower_Organizer_Lumen_cpp](https://github.com/fernandoncidade/Eisenhower_Organizer_Lumen_cpp)  
**Dual Licensing:** `GNU AGPLv3` (Open-Source Repository) / `Official EULA + CLC` (Microsoft Store: `9P289X0185C3` & `9N70CLLMVRPN`)  

---

## Major Consolidated Deliverables

### 1. On-Demand Local AI (`Modelo`: Qwen 2.5 GGUF + Ollama)
- **Slimmer Installers (> 2 GB smaller):** Removed mandatory pre-bundled ~2.17 GB model weights from installer payloads.
- **Interactive Download Manager (`ModelDownloadDialog`):** Downloads `Qwen/Qwen2.5-3B-Instruct-GGUF` directly from Hugging Face with real-time speed/ETA, HTTP Range pause & resume, explorer reveal, and safe disk deletion.
- **Live Menu Status & Ollama Guide (`OllamaInstrucoesDialog`):** Dynamic `[Installed]` / `[Not installed]` menu badges, 1-click CLI command copy (`llama3.2`, `qwen2.5:3b`), `http://localhost:11434` connectivity check, `llama.cpp b8987` CPU/Vulkan inference, and seamless heuristic fallback.

### 2. C++23, Qt 6.12.0, Pure MSVC & MinGW-w64 15.1.0 Modernization
- **Strict C++23 & Qt 6.12.0 Upgrade:** Standardized across host, 7 modules, `mocks/`, and `tests/`, eliminating reserved C++23 keywords (`concept`) across module interfaces.
- **Pure MSVC (`build_*_msvc`) & MinGW (`build_*_mingw`):** Native `"Visual Studio 18 2026"` (`MSVC 14.52.37025`, Windows SDK `10.0.28000.0`) and `MinGW-w64 15.1.0` + Ninja `1.13.2`.
- **Clean External Distributions & Compact Installers:** Lean deployment folders and Inno Setup packages using app-local VC++ CRT DLLs on MSVC without embedding `vc_redist.x64.exe` (~`66.8 MB` MSVC vs ~`73.3 MB` MinGW).

### 3. Unified Software Engineering Test Suite (`tests/` — 11 CTest Executables)
- Unified 11-executable Qt Test suite inside `build/build_suite_testes_msvc` and `build/build_suite_testes_mingw` (VS Code tasks `"Qt6: Compilar e Executar SUITE (MSVC)"` and `"Qt6: Compilar e Executar SUITE (MinGW)"`), strictly local with zero external distribution and **11/11 tests passing** on both toolchains.

### 4. Dual Licensing & Complete Documentation Suite
- Full adoption of **GNU AGPLv3** ([`LICENSE`](../LICENSE)) for public source code with developer freedom over `source/assets/` in open-source forks, paired with the author's **Official EULA + CLC** for signed Microsoft Store editions (`9P289X0185C3`, `9N70CLLMVRPN`) and complete bilingual `docs/` guides.
- Repository structure standardization: centralizing Inno Setup scripts, MSIX manifests (`E-AppxManifest.xml` / `L-AppxManifest.xml`), and technical notes under `library/`, moving UI screenshots to `images/interfaces/`, relocating the shared Windows icon resource template to [`source/windows_app_icon.rc.in`](../source/windows_app_icon.rc.in), removing the redundant `docs/cli-reference.md`, and upgrading [`find_VersionEditor.cpp`](../mocks/find_VersionEditor.cpp) to track all `256` project targets with automatic `docs/release-v*.md` file renaming.

</details>
