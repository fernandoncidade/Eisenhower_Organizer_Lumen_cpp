# Guia de Contribuição / Contributing Guide — EISENHOWER ORGANIZER / LÚMEN

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher as instruções em português</summary><br>

# Contribuindo com o EISENHOWER ORGANIZER / LÚMEN

**Versão:** `2026.11.22.0`  
**Data:** `22 de novembro de 2026`  

Agradecemos profundamente seu interesse em contribuir com os projetos **EISENHOWER ORGANIZER** e **LÚMEN**! Sua colaboração fortalece ferramentas livres, acessíveis, locais e tecnicamente sólidas para organização pessoal, estudo ativo, leitura assistida e planejamento de cronogramas.

> [!NOTE]
> **Sobre o Desenvolvimento, Suporte, Duplo Licenciamento e Contribuições Externas:**  
> Este projeto foi originalmente concebido, desenvolvido e mantido por **Fernando Nillsson Cidade** como uma iniciativa pessoal e acadêmica em programação C++23, acessibilidade cognitiva e engenharia civil/planejamento de projetos.  
> O envio de sugestões, abertura de *Issues* e submissão de *Pull Requests (PRs)* são sempre bem-vindos! No entanto, ressalta-se que a análise de código e incorporação de contribuições ocorrem no próprio ritmo do autor, em regime voluntário e sem compromisso de prazo ou SLA. Caso um chamado ou PR demore para ser respondido ou não seja incorporado à branch principal, compreenda a limitação de tempo: nos termos da licença **GNU AGPLv3** ([`LICENSE`](LICENSE)), você possui total liberdade para criar um *Fork* independente deste repositório e evoluir suas próprias compilações, inclusive alterando livremente `source/assets/` caso remova as marcas exclusivas do autor (`EISENHOWER ORGANIZER` e `LÚMEN`).  
> O aplicativo é fornecido como **ferramenta auxiliar** no estado em que se encontra (**"AS IS"**) e exige conferência independente pelo usuário em prazos críticos e cronogramas técnicos ([`README.md`](README.md#ptbr), [`docs/licensing.md`](docs/licensing.md#ptbr) e [EULA](source/assets/eisenhower/EULA/EULA_pt_BR.txt)).

---

## 1. Código de Conduta

Ao participar deste projeto, você concorda em cumprir nosso [Código de Conduta](CODE_OF_CONDUCT.md). Trate todos os colaboradores com respeito, empatia, acessibilidade e profissionalismo.

---

## 2. Formas de Contribuir

Você pode colaborar com o ecossistema **Eisenhower Organizer / Lúmen** de diversas maneiras:
- **Relatar Bugs:** Identifique falhas de persistência (`UnifiedTaskStore`), inconsistências de sincronização entre módulos, defeitos de cálculo CPM/DAG no módulo Gantt, regressões de acessibilidade no Audiolivro ou falhas de compilação MSVC/MinGW.
- **Propor Melhorias Arquiteturais e Funcionais:** Sugira evoluções nos 7 módulos integrados (`Eisenhower`, `Pomodoro`, `Feynman`, `Mapa Mental`, `Audiolivro`, `Agenda`, `Gantt & BIM 4D`) ou no módulo auxiliar de IA Local `Modelo` (`Qwen 2.5 GGUF` / `Ollama`). Consulte [`docs/suggested-issues.md`](docs/suggested-issues.md#ptbr).
- **Melhorar a Documentação e Acessibilidade:** Aprimore [`docs/architecture.md`](docs/architecture.md#ptbr), [`docs/user-guide.md`](docs/user-guide.md#ptbr), [`docs/compliance.md`](docs/compliance.md#ptbr) ou o [`MANUAL.md`](MANUAL.md).
- **Ampliar Cobertura da Suíte de Testes (`tests/`):** Adicione casos unitários e de integração em Qt Test nos 11 alvos de engenharia de software da suíte unificada (`Qt6: Compilar e Executar SUITE (MSVC)` e `Qt6: Compilar e Executar SUITE (MinGW)`).
- **Internacionalização (i18n):** Verifique chaves `.ts`, traduções dinâmicas em `changeEvent(QEvent::LanguageChange)` e compilação com `CompileTranslationsTerminal.exe`.

---

## 3. Configuração do Ambiente de Desenvolvimento

### 3.1. Pré-requisitos
- **Sistema Operacional:** Windows 10 / 11 x64 (`17763+`)
- **Compilador C++23:**
  - **MSVC x64 Puro:** Visual Studio 18 2026 BuildTools (`MSVC 14.52.37025`, Windows SDK `10.0.28000.0`, gerador `"Visual Studio 18 2026"`)
  - **MinGW-w64 x64:** GCC `15.1.0` (`C:\Qt\Tools\mingw1510_64`) + **Ninja**
- **CMake:** Versão `3.28` ou superior
- **Qt `6.12.0` (`msvc2022_64` e `mingw_64`):** `Core`, `Gui`, `Widgets`, `OpenGL`, `OpenGLWidgets`, `Pdf`, `PdfWidgets`, `Sql`, `Multimedia`, `Network`, `WebSockets`, `Test` e `LinguistTools` (`lrelease.exe` / `windeployqt.exe`).

### 3.2. Compilação e Testes via `CMakePresets.json`

Todas as árvores de build são isoladas sob `build/` e definidas em [`CMakePresets.json`](CMakePresets.json):

```powershell
# Compilar os hosts principais (Eisenhower Organizer e Lúmen) em MSVC e MinGW
cmake --preset host-qt6-msvc-release && cmake --build --preset host-qt6-msvc-release
cmake --preset host-lumen-qt6-msvc-release && cmake --build --preset host-lumen-qt6-msvc-release
cmake --preset host-qt6-mingw-release && cmake --build --preset host-qt6-mingw-release
cmake --preset host-lumen-qt6-mingw-release && cmake --build --preset host-lumen-qt6-mingw-release

# Compilar utilitários auxiliares (mocks/)
cmake --preset mocks-msvc-release && cmake --build --preset mocks-msvc-release
cmake --preset mocks-mingw-release && cmake --build --preset mocks-mingw-release

# Compilar e executar integralmente a Suíte de Testes Unificada (11 executáveis CTest, sem distribuição externa)
cmake --preset suite-testes-msvc-release && cmake --build --preset suite-testes-msvc-release --parallel
ctest --preset suite-testes-msvc-release --output-on-failure

cmake --preset suite-testes-mingw-release && cmake --build --preset suite-testes-mingw-release --parallel
ctest --preset suite-testes-mingw-release --output-on-failure
```

> [!IMPORTANT]
> Nunca recrie os antigos diretórios `build/build_tests_msvc` ou `build/build_tests_mingw`. Toda a suíte de testes é unificada exclusivamente em `build/build_suite_testes_msvc` e `build/build_suite_testes_mingw` (reforçado por trava `FATAL_ERROR` em [`tests/CMakeLists.txt`](tests/CMakeLists.txt)), e **não possui distribuição externa**.

*(Veja também [`CMakePresets.json`](CMakePresets.json) e as tarefas prontas do VS Code em [`.vscode/tasks.json`](.vscode/tasks.json), incluindo `"Qt6: Compilar e Executar SUITE (MSVC)"` e `"Qt6: Compilar e Executar SUITE (MinGW)"`.)*

---

## 4. Padrões de Código e Diretrizes Arquiteturais

1. **C++23 Estrito:** Evite identificadores reservados da linguagem (ex.: `concept`, `requires`) como nomes de variáveis/membros — compatibilidade obrigatória tanto com `/std:c++23latest`/`c++23` no MSVC `14.52` quanto com `-std=c++23` no GCC `15.1.0`.
2. **Persistência Atômica e Segurança:** Utilize sempre `QLockFile` + `QSaveFile` em atualizações do `UnifiedTaskStore` (`tarefas.json`), queries parametrizadas em SQLite (`Agenda` e `Gantt`) e `QFileInfo::exists()` antes de abrir arquivos externos vinculados.
3. **Tradução Dinâmica sem Reinício:** Novos componentes gráficos devem implementar `changeEvent(QEvent::LanguageChange)` (usando chaves normalizadas em `source/language/*.ts`), compiladas com `CompileTranslationsTerminal.exe` antes do commit.
4. **Identidade Dual (`AppConfig`):** Qualquer código no host ou em módulos compartilhados deve respeitar `BUILD_TARGET=EISENHOWER` e `BUILD_TARGET=LUMEN` (nomes de produto, `%LOCALAPPDATA%`, logs e `source/assets/{eisenhower,lumen}/`).
5. **Distribuição Externa Limpa:** Não inclua binários de modelo `.gguf`, `vc_redist.x64.exe`, documentação de desenvolvimento (`docs/*.md`) ou artefatos de teste (`tests/`) nos scripts de deploy binário (`cmake/copy_app_assets.cmake` e `library/*.iss`).

---

## 5. Processo de Envio de Pull Requests e Licença de Contribuições

1. Crie uma branch a partir da branch principal (`git checkout -b feat/minha-melhoria`).
2. Atualize/adicione testes em [`tests/`](tests/) e valide **ambos** os toolchains (`suite-testes-msvc-release` e `suite-testes-mingw-release` com 11/11 aprovados).
3. Caso altere versões ou textos institucionais em `source/assets/`, mantenha `eisenhower/` e `lumen/` sincronizados e valide com `find_VersionEditor.exe`.
4. Ao submeter um Pull Request, você concorda que suas contribuições ao código-fonte público serão licenciadas sob a **GNU AGPLv3** ([`LICENSE`](LICENSE)).

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse instructions in English</summary><br>

# Contributing to EISENHOWER ORGANIZER / LÚMEN

**Version:** `2026.11.22.0`  
**Date:** `November 22, 2026`  

Thank you for your interest in contributing to **EISENHOWER ORGANIZER** and **LÚMEN**! Your contributions help advance open, accessible, local-first, and technically robust software for task management, active study, assistive reading, and project scheduling.

> [!NOTE]
> **Development Context, Support, Dual Licensing & External Contributions:**  
> This project was conceived, developed, and maintained by **Fernando Nillsson Cidade** as a personal and academic initiative in C++23 engineering, cognitive accessibility, and civil engineering scheduling.  
> Suggestions, *Issues*, and *Pull Requests (PRs)* are always welcome! Please note that code review and merging occur at the author's own pace, strictly on a voluntary best-effort basis with no SLA or timeframe commitment. Under **GNU AGPLv3** ([`LICENSE`](LICENSE)), you have complete freedom to fork this repository and evolve your own builds independently, including freely modifying `source/assets/` as long as the author's exclusive brand names (`EISENHOWER ORGANIZER` and `LÚMEN`) and official logos are removed.  
> The software is supplied as an **auxiliary tool** **"AS IS"** and requires independent verification by the user for critical deadlines and technical schedules ([`README.md`](README.md#enus), [`docs/licensing.md`](docs/licensing.md#enus), and [EULA](source/assets/eisenhower/EULA/EULA_en_US.txt)).

---

## 1. Code of Conduct

By participating in this project, you agree to abide by our [Code of Conduct](CODE_OF_CONDUCT.md). Treat all contributors with respect, empathy, accessibility awareness, and professionalism.

---

## 2. Ways to Contribute

You can contribute to **Eisenhower Organizer / Lúmen** in several ways:
- **Bug Reports:** Report persistence defects (`UnifiedTaskStore`), cross-module synchronization issues, CPM/DAG calculation discrepancies in Gantt, accessibility regressions in Audiobook, or MSVC/MinGW build issues.
- **Architectural & Feature Proposals:** Propose improvements across the 7 integrated modules (`Eisenhower`, `Pomodoro`, `Feynman`, `Mapa Mental`, `Audiolivro`, `Agenda`, `Gantt & BIM 4D`) or the local AI `Modelo` service (`Qwen 2.5 GGUF` / `Ollama`). See [`docs/suggested-issues.md`](docs/suggested-issues.md#enus).
- **Documentation & Accessibility:** Enhance [`docs/architecture.md`](docs/architecture.md#enus), [`docs/user-guide.md`](docs/user-guide.md#enus), [`docs/compliance.md`](docs/compliance.md#enus), or [`MANUAL.md`](MANUAL.md).
- **Test Suite Coverage (`tests/`):** Expand unit and integration tests across all 11 Qt Test targets in the unified suite (`Qt6: Compilar e Executar SUITE (MSVC)` and `Qt6: Compilar e Executar SUITE (MinGW)`).
- **Internationalization (i18n):** Audit `.ts` catalogs, runtime `changeEvent(QEvent::LanguageChange)` refreshes, and compilation via `CompileTranslationsTerminal.exe`.

---

## 3. Development Environment Setup

### 3.1. Prerequisites
- **Operating System:** Windows 10 / 11 x64 (`17763+`)
- **C++23 Compiler:**
  - **Pure MSVC x64:** Visual Studio 18 2026 BuildTools (`MSVC 14.52.37025`, Windows SDK `10.0.28000.0`, `"Visual Studio 18 2026"` generator)
  - **MinGW-w64 x64:** GCC `15.1.0` (`C:\Qt\Tools\mingw1510_64`) + **Ninja**
- **CMake:** Version `3.28` or newer
- **Qt `6.12.0` (`msvc2022_64` & `mingw_64`):** `Core`, `Gui`, `Widgets`, `OpenGL`, `OpenGLWidgets`, `Pdf`, `PdfWidgets`, `Sql`, `Multimedia`, `Network`, `WebSockets`, `Test`, and `LinguistTools` (`lrelease.exe` / `windeployqt.exe`).

### 3.2. Building and Testing via `CMakePresets.json`

All build trees live isolated under `build/` and are defined in [`CMakePresets.json`](CMakePresets.json):

```powershell
# Build both main hosts (Eisenhower Organizer and Lúmen) with MSVC and MinGW
cmake --preset host-qt6-msvc-release && cmake --build --preset host-qt6-msvc-release
cmake --preset host-lumen-qt6-msvc-release && cmake --build --preset host-lumen-qt6-msvc-release
cmake --preset host-qt6-mingw-release && cmake --build --preset host-qt6-mingw-release
cmake --preset host-lumen-qt6-mingw-release && cmake --build --preset host-lumen-qt6-mingw-release

# Build auxiliary tools (mocks/)
cmake --preset mocks-msvc-release && cmake --build --preset mocks-msvc-release
cmake --preset mocks-mingw-release && cmake --build --preset mocks-mingw-release

# Build and run the full Unified Test Suite (11 CTest targets, no external distribution)
cmake --preset suite-testes-msvc-release && cmake --build --preset suite-testes-msvc-release --parallel
ctest --preset suite-testes-msvc-release --output-on-failure

cmake --preset suite-testes-mingw-release && cmake --build --preset suite-testes-mingw-release --parallel
ctest --preset suite-testes-mingw-release --output-on-failure
```

> [!IMPORTANT]
> Never recreate the legacy `build/build_tests_msvc` or `build/build_tests_mingw` directories. The entire test suite is unified exclusively inside `build/build_suite_testes_msvc` and `build/build_suite_testes_mingw` (enforced by a `FATAL_ERROR` guard in [`tests/CMakeLists.txt`](tests/CMakeLists.txt)) with **zero external distribution**.

*(See also [`CMakePresets.json`](CMakePresets.json) and ready-to-run VS Code tasks in [`.vscode/tasks.json`](.vscode/tasks.json), including `"Qt6: Compilar e Executar SUITE (MSVC)"` and `"Qt6: Compilar e Executar SUITE (MinGW)"`.)*

---

## 4. Coding Standards & Architectural Guidelines

1. **Strict C++23:** Avoid C++20/23 reserved keywords (e.g., `concept`, `requires`) as identifiers — code must build cleanly on both MSVC `14.52` (`/std:c++23latest`) and GCC `15.1.0` (`-std=c++23`).
2. **Atomic Persistence & Safety:** Always use `QLockFile` + `QSaveFile` for `UnifiedTaskStore` (`tarefas.json`) writes, parameterized SQLite queries in `Agenda` and `Gantt`, and `QFileInfo::exists()` checks before launching linked external files.
3. **Live Runtime Translation:** UI widgets must implement `changeEvent(QEvent::LanguageChange)` backed by normalized keys in `source/language/*.ts`, compiled with `CompileTranslationsTerminal.exe` before submission.
4. **Dual Identity (`AppConfig`):** Shared host and module code must respect `BUILD_TARGET=EISENHOWER` and `BUILD_TARGET=LUMEN` (product names, `%LOCALAPPDATA%` paths, logs, and `source/assets/{eisenhower,lumen}/`).
5. **Clean External Distribution:** Keep `.gguf` weights, `vc_redist.x64.exe`, repository docs (`docs/*.md`), and test binaries (`tests/`) out of external binary distributions (`cmake/copy_app_assets.cmake` and `library/*.iss`).

---

## 5. Pull Request Process & Contribution Licensing

1. Create a branch from the main branch (`git checkout -b feat/my-improvement`).
2. Add or update unit/integration tests under [`tests/`](tests/) and verify **both** toolchains pass 11/11 tests (`suite-testes-msvc-release` and `suite-testes-mingw-release`).
3. If updating versions or institutional asset files under `source/assets/`, keep both `eisenhower/` and `lumen/` synchronized and check with `find_VersionEditor.exe`.
4. By submitting a Pull Request, you agree that your contributions to the public source code are licensed under **GNU AGPLv3** ([`LICENSE`](LICENSE)).

</details>
