<!-- Multilanguage README.md for Eisenhower_Organizer_Lumen_cpp -->

# EISENHOWER ORGANIZER & LÚMEN — Suíte Modular de Produtividade, Estudo Ativo, Leitura Acessível e Cronograma Gantt BIM 4D

**Plataforma desktop nativa em Windows (dupla identidade: `EisenhowerOrganizer.exe` e `Lumen.exe`) desenvolvida em C++23 moderno com Qt 6.12.0, compilada nativamente com Pure MSVC x64 (`Visual Studio 18 2026` / `MSVC 14.52`) e MinGW-w64 (`GCC 15.1.0`), com distribuições externas limpas e instaladores Inno Setup compactos (`app-local CRT`, sem `vc_redist.x64.exe`). Reúne 7 módulos nativos integrados (`Eisenhower`, `Pomodoro`, `Feynman`, `Mapa Mental`, `Audiolivro`, `Agenda` e `Cronograma de Gantt BIM 4D`), módulo auxiliar de IA Local sob demanda `Modelo` (`Qwen 2.5 GGUF` via `llama.cpp` CPU/Vulkan e `Ollama`), persistência atômica Local-First (`UnifiedTaskStore` / SQLite) e suíte automatizada de 11 alvos CTest (`pt-BR` / `en-US`).**

[![Version](https://img.shields.io/badge/Version-v2026.11.22.0-blue.svg)](docs/release-v2026.11.22.0.md)
[![C++23](https://img.shields.io/badge/C%2B%2B-23-00599C?logo=c%2B%2B)](CMakeLists.txt)
[![Qt 6.12.0](https://img.shields.io/badge/Qt-6.12.0-41CD52?logo=qt)](CMakeLists.txt)
[![Toolchains](https://img.shields.io/badge/Toolchain-Pure_MSVC_14.52_%7C_MinGW_15.1.0-0078D4)](CMakePresets.json)
[![CTest](https://img.shields.io/badge/CTest_Suite-11%2F11_Passing-2ea44f.svg)](tests/CMakeLists.txt)
[![Eisenhower Store](https://img.shields.io/badge/Store-Eisenhower_Organizer-0078D4?logo=microsoft)](https://apps.microsoft.com/detail/9P289X0185C3)
[![Lúmen Store](https://img.shields.io/badge/Store-L%C3%BAmen-0078D4?logo=microsoft)](https://apps.microsoft.com/detail/9N70CLLMVRPN)
[![License GNU AGPLv3](https://img.shields.io/badge/License-GNU_AGPLv3-blue.svg)](LICENSE)
[![Official Store EULA](https://img.shields.io/badge/License-Official_Store_EULA_%2B_CLC-purple.svg)](docs/licensing.md)
[![Local-First](https://img.shields.io/badge/Privacy-Local--First-success.svg)](docs/privacy-policy.md)

<p align="center">
  <a href="docs/user-guide.md">📖 Guia do Usuário / User Guide</a> •
  <a href="docs/architecture.md">🏗️ Arquitetura / Architecture</a> •
  <a href="docs/compliance.md">📋 Conformidade / Compliance</a> •
  <a href="docs/security-model.md">🛡️ Privacidade &amp; Segurança</a> •
  <a href="docs/licensing.md">⚖️ Duplo Licenciamento / Dual Licensing</a><br>
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> •
  <a href="#enus">🇺🇸 English (US)</a>
</p>

---

## <a id="ptbr"></a><a id="português-br"></a>🇧🇷 Português (BR)

> **Observação:** Este repositório refere-se à versão **v2026.11.22.0** do projeto EISENHOWER ORGANIZER. Apoie o projeto e adquira a edição oficial por meio do link: [Instalar via Microsoft Store](https://apps.microsoft.com/detail/9P289X0185C3)

> **Observação:** Este repositório refere-se à versão **v2026.11.22.0** do 📚 Projeto Lúmen. Apoie o projeto e adquira a edição oficial através do link: [Instalar via Microsoft Store](https://apps.microsoft.com/detail/9N70CLLMVRPN)

<details>
<summary><b>Clique para expandir / recolher a documentação em Português (BR)</b></summary><br>

# EISENHOWER ORGANIZER / LÚMEN — Organizador de Tarefas, Estudos, Acessibilidade e Projetos

Versão: v2026.11.22.0<br>
Data técnica desta revisão: 22 de novembro de 2026<br>
Autor: Fernando Nillsson Cidade<br>

## Visão Geral e Dupla Identidade

O **EISENHOWER ORGANIZER / LÚMEN** é uma plataforma desktop nativa para Windows, construída em **C++23 com Qt 6.12.0**, concebida originalmente para apoiar organização pessoal, ensino, leitura acessível (especialmente para usuários com TDAH, dislexia e TEA), estudo ativo e controle de cronogramas.

A partir da mesma base de código unificada (`BUILD_TARGET=EISENHOWER` ou `LUMEN` via [`AppConfig`](source/utils/AppConfig.hpp)), o projeto gera dois aplicativos distintos com armazenamento independente e tradução dinâmica completa em tempo de execução (`pt-BR` / `en-US`):

| Identidade (`BUILD_TARGET`) | Executável / Banco Local | Foco Principal | Loja Oficial (Microsoft Store) |
|---|---|---|---|
| **EISENHOWER ORGANIZER** (`EISENHOWER`) | `EisenhowerOrganizer.exe`<br>`%LOCALAPPDATA%\EisenhowerOrganizer` | Produtividade matricial, execução focada, agendas e cronogramas de projetos / engenharia | [Microsoft Store (`9P289X0185C3`)](https://apps.microsoft.com/detail/9P289X0185C3) |
| **LÚMEN** (`LUMEN`) | `Lumen.exe`<br>`%LOCALAPPDATA%\Lumen` | Plataforma de estudos e acessibilidade cognitiva (TDAH, dislexia, TEA) com todos os módulos integrados | [Microsoft Store (`9N70CLLMVRPN`)](https://apps.microsoft.com/detail/9N70CLLMVRPN) |

---

### ⚖️ Manifesto e Autorização Legal do Titular dos Direitos Autorais (Duplo Licenciamento)

> **Declaração Expressa de Fernando Nillsson Cidade:**  
> Eu, **Fernando Nillsson Cidade**, na qualidade de único autor, criador original e titular exclusivo dos direitos autorais dos projetos **EISENHOWER ORGANIZER** e **LÚMEN**, adoto um modelo transparente de **Duplo Licenciamento**:  
> 1. **Código-Fonte Público (Open Source sob GNU AGPLv3):** Autorizo o uso, estudo, cópia, modificação, compilação e distribuição do código-fonte público deste repositório nos exatos termos do arquivo **[LICENSE](LICENSE)** (**GNU Affero General Public License v3 — GNU AGPLv3**, identificador SPDX `AGPL-3.0-only`), ressalvada a proteção nominal de nomes e marcas sob a Seção 7(e) (`"EISENHOWER ORGANIZER"` e `"LÚMEN"`). Qualquer obra derivada deve preservar a autoria original e disponibilizar o código-fonte correspondente sob a mesma licença **GNU AGPLv3** (incluindo uso em rede conforme Seção 13).  
> 2. **Liberdade para Documentação Institucional em `source/assets/`:** Os documentos institucionais e contratuais mantidos em [`source/assets/eisenhower/`](source/assets/eisenhower/COPYRIGHT/AVISO%20DE%20COPYRIGHT%20E%20MARCA%20REGISTRA_pt_BR.txt) e [`source/assets/lumen/`](source/assets/lumen/COPYRIGHT/AVISO%20DE%20COPYRIGHT%20E%20MARCA%20REGISTRA_pt_BR.txt) (`ABOUT`, `CLC`, `COPYRIGHT`, `EULA`, `LICENSES`, `NOTICES`, `PRIVACY_POLICY` e `RELEASE`) foram elaborados para viabilizar a publicação oficial na Microsoft Store. Desenvolvedores e colaboradores que criarem forks sob a **GNU AGPLv3** têm **total liberdade** para editar, remover, substituir ou adaptar tais textos às suas próprias compilações, desde que removam as marcas e sinais distintivos exclusivos do autor para não gerar confusão com os produtos oficiais e observem a legislação vigente e as licenças de terceiros.  
> 3. **Binários Oficiais Assinados, Edições Microsoft Store e Contrato Comercial (`EULA` + `CLC`):** As edições compiladas, assinadas digitalmente e publicadas pelo autor na Microsoft Store ([Eisenhower Organizer](https://apps.microsoft.com/detail/9P289X0185C3) e [Lúmen](https://apps.microsoft.com/detail/9N70CLLMVRPN), com avaliação de 7 dias via `TrialManager` e planos comerciais descritos no `CLC`) são regidas pelo respectivo **[EULA Oficial](source/assets/eisenhower/EULA/EULA_pt_BR.txt)** e **[Contrato de Licenciamento Comercial — CLC](source/assets/eisenhower/CLC/CLC_pt_BR.txt)**. É expressamente vedado a terceiros revender, republicar ou explorar comercialmente os pacotes binários oficiais assinados pelo autor ou usar as marcas `EISENHOWER ORGANIZER` e `LÚMEN` sem autorização escrita.  
> 4. **Canal de Suporte, Independência da Microsoft e Isenção Legal ("AS IS"):** A **Microsoft NÃO presta suporte ao cliente nem cobre estes aplicativos sob a Política de Privacidade da Microsoft**. Todo eventual suporte técnico do autor é prestado **exclusivamente por e-mail (`linceu_lighthouse@outlook.com`) e pelas abas Issues/Discussions do GitHub**, **em regime voluntário de melhores esforços (best-effort), conforme disponibilidade e sem obrigação de urgência, prazo ou SLA**. Os aplicativos e seus módulos são fornecidos **"NO ESTADO EM QUE SE ENCONTRAM" ("AS IS")** como ferramentas **auxiliares** de produtividade, estudo, acessibilidade e apoio ao planejamento, sujeitas a limitações e eventuais falhas no código-fonte, algoritmos ou modelos de IA local, exigindo conferência independente pelo usuário. Consulte [`docs/licensing.md`](docs/licensing.md#ptbr).

---

### 🧩 Módulos Nativos Integrados e IA Local Sob Demanda

A arquitetura atual consolida 7 áreas de trabalho nativas integradas e o serviço complementar de IA Local `Modelo` diretamente no host (mantendo também alvos modulares/DLL independentes em `source/modules/` e verificação contratual em `tests/`):

| Módulo | Responsabilidade Principal | Tecnologias / Persistência |
|---|---|---|
| **1. Eisenhower (Host)** | Matriz de 4 quadrantes (`Pendente`, `Em Progresso`, `Concluída`), layouts `Abas Vertical/Horizontal` (colunas recolhíveis com rótulo rotacionado em 90°), `Quadrantes` e `Matriz 2x2`, calendário global, recorrências, lembretes locais, importação/exportação `JSON`/`CSV`/`XLSX`/`PDF` | `UnifiedTaskStore` (`QLockFile` + `QSaveFile`, `tarefas.json` + `.bak`), `EditTaskDialog` responsivo |
| **2. Pomodoro** | Temporizador configurável de foco/pausa curta/pausa longa, modo automático com alarme WAV, quadro Kanban (`A Fazer`/`Em Progresso`/`Concluído`), detecção de divergência de metas (`CycleMismatchDialog`) | `Qt6::Multimedia`, sincronização bidirecional via `EventBus` |
| **3. Feynman** | Estudo ativo em 4 etapas (`Conceito`, `Explique com Simplicidade`, `Lacunas Identificadas`, `Revisão`), prioridades, nível de domínio, revisões espaçadas e vínculos externos | Persistência assíncrona via worker (`conceitos_feynman.json`) |
| **4. Mapa Mental** | Grafo visual de conceitos, layouts `Vertical`/`Horizontal`/`Radial`/`Árvore`, visualização focada, importação de documentos (`PDF`, `EPUB`, `DOC/DOCX`, `TXT`, `MD`, `CSV`, `XLSX`, `JSON`, `DB/SQLite`, `XML`), extração heurística NLP/TF-IDF, JSON + exportação PNG | `Qt6::OpenGLWidgets`, `Qt6::Pdf`, `Qt6::CorePrivate::QZipReader`, `mapa_mental_autosave.json` |
| **5. Modelo (IA Local)** | Download sob demanda do `qwen2.5-3b-instruct-q4_k_m.gguf` (~2,17 GB) via Hugging Face com HTTP Range (pausa/retomada/exclusão), inferência offline `llama.cpp b8987` (CPU/Vulkan) e descoberta/assistente `Ollama` (`localhost:11434`) | `ModelDownloadDialog`, `OllamaInstrucoesDialog`, fallback heurístico automático |
| **6. Audiolivro** | Leitor acessível de texto e PDF, marcações, busca, realce sincronizado (palavra/frase/parágrafo), **Régua de Foco** linha a linha, vozes SAPI offline e suporte opcional a Edge TTS (300+ vozes) | `Qt6::Pdf`, `Qt6::PdfWidgets`, `Qt6::WebSockets`, `audiolivro_config.json` |
| **7. Agenda** | Registro de atividades acadêmicas e personalizadas, listas aninhadas em cascata (formato árvore), cores `Preto`/`Coloridas` por módulo e vínculo de arquivos externos | `Qt6::Sql` (`QSQLITE`, `listas.db`), `agenda_config.json`, `agendas/*.json` |
| **8. Cronograma de Gantt BIM 4D** | Estrutura EAP/WBS hierárquica, validação DAG (`DependencyValidator` sem ciclos para `TI`/`II`/`TT`/`IT` + *lag*), Método do Caminho Crítico (`CpmEngine` `ES/EF/LS/LF/Folga`), baselines, diário de obra (`FieldProgressService`), simulação 4D (`Bim4DService`), MS Project XML, mapeador CSV, auditoria e API REST/Webhooks locais opcionais com RBAC/rate-limit | `Qt6::Sql` (`gantt_database.db`), `AuditService`, `SecurityService` |

---

### 📌 Ficha Técnica e Especificações de Engenharia

- **Linguagem e Framework:** **C++23 estrito** (`CMAKE_CXX_STANDARD 23`, `CMAKE_CXX_EXTENSIONS OFF`) + **Qt 6.12.0** (`Core`, `Gui`, `Widgets`, `OpenGL`, `OpenGLWidgets`, `Pdf`, `PdfWidgets`, `Sql`, `Multimedia`, `Network`, `WebSockets`, `Test`, `LinguistTools`).
- **Compiladores Oficiais Suportados no Windows 10/11 x64:**
  - **Pure MSVC x64:** `Visual Studio 18 2026` BuildTools (`MSVC 14.52.37025`, Windows SDK `10.0.28000.0`, gerador `"Visual Studio 18 2026"`, árvores `build\build_*_msvc`).
  - **MinGW-w64 x64:** `GCC 15.1.0` (`C:\Qt\Tools\mingw1510_64`) + `Ninja` (árvores `build\build_*_mingw`).
- **Distribuições Externas Limpas e Instaladores Compactos (`Inno Setup`):**
  - Pastas de saída enxutas em `D:\MISCELANEAS\VSCode\EisenhowerOrganizer\Eisenhower_Organizer_cpp_{msvc,mingw}` e `D:\MISCELANEAS\VSCode\Lumen\Lumen_cpp_{msvc,mingw}`, contendo exclusivamente os binários, traduções `.qm`, ícones necessários, documentos de `source/assets/` consumidos em tempo de execução e dependências de runtime estritas.
  - Scripts Inno Setup em `library/` (`Eisenhower_Organizer.iss`, `Eisenhower_Organizer_MSVC.iss`, `Eisenhower_Organizer_MinGW.iss`, `Lumen.iss`, `Lumen_MSVC.iss`, `Lumen_MinGW.iss`) configurados para **app-local CRT deployment** (5 DLLs do VC++ Runtime `msvcp140*.dll`/`vcruntime140*.dll` no MSVC, **sem embutir `vc_redist.x64.exe`**), garantindo instaladores enxutos (~`66,8 MB` MSVC vs ~`73,3 MB` MinGW) sem inflação artificial de ~18 MB.
- **Suíte Unificada de Testes (`tests/` — 11 Executáveis CTest):**
  - Builds locais dedicadas e isoladas em `build/build_suite_testes_msvc` e `build/build_suite_testes_mingw` (**sem distribuição externa**), integrando todas as frentes: `test_core_utils`, `test_store_models`, `test_ui_host`, `test_plugin_pomodoro`, `test_plugin_feynman`, `test_plugin_mapamental`, `test_plugin_modelo`, `test_plugin_audiolivro`, `test_plugin_agenda`, `test_plugin_gantt` e `test_frameworks_plugins`.

---

### <a id="microsoft-store-privacidade-suporte"></a>🛡️ Distribuição Microsoft Store, Privacidade e Conformidade

> [!IMPORTANT]
> **Conformidade Oficial Microsoft Store (Políticas 10.2 e 10.5):**
> - **Política de Privacidade Independente:** **A Microsoft NÃO fornece uma política de privacidade padrão para o EISENHOWER ORGANIZER nem para o LÚMEN, e estes aplicativos NÃO são cobertos pela [Política de Privacidade da Microsoft](https://www.microsoft.com/pt-br/privacy/privacystatement).** Todo tratamento de dados é regido estritamente pela [Política de Privacidade do Projeto](docs/privacy-policy.md#ptbr) (e pelos textos em `source/assets/{eisenhower,lumen}/PRIVACY_POLICY/`).
> - **Suporte do Desenvolvedor Independente:** **A Microsoft NÃO presta suporte técnico aos usuários destes aplicativos.** Todo suporte é atendido pelo autor (`Fernando Nillsson Cidade`) por via eletrônica (`linceu_lighthouse@outlook.com` / [GitHub Issues](https://github.com/fernandoncidade/Eisenhower_Organizer_Lumen_cpp/issues)), em regime voluntário e sem SLA.
> - **Arquitetura Local-First com Rede Estritamente Sob Demanda:** Tarefas, conceitos, notas, bancos SQLite (`listas.db`, `gantt_database.db`), mapas mentais e logs permanecem localmente em `%LOCALAPPDATA%\EisenhowerOrganizer` e `%LOCALAPPDATA%\Lumen`. Não há conta obrigatória nem telemetria automática. Conexões externas ocorrem apenas quando acionadas voluntariamente pelo usuário (download GGUF do Hugging Face via HTTPS, `Ollama` local em `127.0.0.1:11434`, síntese opcional Edge TTS no Audiolivro ou servidor HTTP/Webhooks locais opcionais no Gantt).

| Disposição Regulatória | Referência no Repositório (`pt-BR`) |
|---|---|
| **Política de Privacidade Própria** | [docs/privacy-policy.md](docs/privacy-policy.md#ptbr) · [Asset Eisenhower](source/assets/eisenhower/PRIVACY_POLICY/Privacy_Policy_pt_BR.txt) · [Asset Lúmen](source/assets/lumen/PRIVACY_POLICY/Privacy_Policy_pt_BR.txt) |
| **Duplo Licenciamento e CLC** | [docs/licensing.md](docs/licensing.md#ptbr) · [GNU AGPLv3](LICENSE) · [EULA Oficial](source/assets/eisenhower/EULA/EULA_pt_BR.txt) · [CLC](source/assets/eisenhower/CLC/CLC_pt_BR.txt) |
| **Site Oficial / Repositório** | [https://github.com/fernandoncidade/Eisenhower_Organizer_Lumen_cpp](https://github.com/fernandoncidade/Eisenhower_Organizer_Lumen_cpp) |
| **Suporte Técnico (Best-Effort)** | `linceu_lighthouse@outlook.com` · [GitHub Issues](https://github.com/fernandoncidade/Eisenhower_Organizer_Lumen_cpp/issues) |

---

### <a id="quick-start-compilacao"></a>🚀 Compilação, Testes e Implantação a partir do Código-Fonte

#### Pré-requisitos
- Windows 10 / 11 x64 (`17763+`);
- **CMake 3.28+** e **Ninja**;
- **Qt 6.12.0** (`C:\Qt\6.12.0\msvc2022_64` e/ou `C:\Qt\6.12.0\mingw_64`);
- **Visual Studio 18 2026 BuildTools** (`MSVC 14.52.37025`, Windows SDK `10.0.28000.0`) e/ou **MinGW-w64 `15.1.0`** (`C:\Qt\Tools\mingw1510_64`).

#### Comandos via `CMakePresets.json`
```powershell
# Clonar o repositório
git clone https://github.com/fernandoncidade/Eisenhower_Organizer_Lumen_cpp.git
cd Eisenhower_Organizer_Lumen_cpp

# 1. Compilar e implantar hosts limpos (MSVC e MinGW)
cmake --preset host-qt6-msvc-release && cmake --build --preset host-qt6-msvc-release --parallel
cmake --preset host-lumen-qt6-msvc-release && cmake --build --preset host-lumen-qt6-msvc-release --parallel
cmake --preset host-qt6-mingw-release && cmake --build --preset host-qt6-mingw-release --parallel
cmake --preset host-lumen-qt6-mingw-release && cmake --build --preset host-lumen-qt6-mingw-release --parallel

# 2. Compilar e executar toda a Suíte de Testes (11 executáveis CTest em MSVC e MinGW)
cmake --preset suite-testes-msvc-release && cmake --build --preset suite-testes-msvc-release --parallel
ctest --preset suite-testes-msvc-release --output-on-failure

cmake --preset suite-testes-mingw-release && cmake --build --preset suite-testes-mingw-release --parallel
ctest --preset suite-testes-mingw-release --output-on-failure

# 3. Compilar utilitários auxiliares (mocks/) e validar traduções (.ts -> .qm)
cmake --preset mocks-msvc-release && cmake --build --preset mocks-msvc-release --parallel
.\build\build_mocks_msvc\Release\CompileTranslationsTerminal.exe --all
```

*(Consulte [`CMakePresets.json`](CMakePresets.json), [`.vscode/tasks.json`](.vscode/tasks.json) e os scripts em [`library/`](library/Eisenhower_Organizer.iss) para os presets de plugins, automações de compilação e geração dos instaladores Inno Setup.)*

---

### 🏛️ Escopo Auxiliar e Isenção Legal ("AS IS")

Este aplicativo — tanto nas compilações livres a partir do código-fonte sob **GNU AGPLv3** quanto nas distribuições binárias oficiais — constitui **ferramenta auxiliar** de organização pessoal, estudo ativo, leitura assistida e apoio a cronogramas de projetos, sendo fornecido **"NO ESTADO EM QUE SE ENCONTRA" ("AS IS")**, com eventuais limitações e riscos de erros algorítmicos, sem garantias explícitas ou implícitas (Lei nº 9.609/1998, Lei nº 9.610/1998, Código Civil, CDC e Seções 15–16 da [GNU AGPLv3](LICENSE)).

- **IA Local (`Qwen 2.5` / `Ollama`) e NLP:** Sínteses conceituais e mapas mentais gerados são auxiliares e probabilísticos, exigindo revisão crítica do usuário.
- **Leitura Assistida (`Audiolivro`):** Recurso de acessibilidade de leitura que não constitui equipamento ou tratamento médico/clínico.
- **Cronograma de Gantt & Engenharia 4D BIM:** Cálculos de Caminho Crítico (CPM), dependências DAG, linha de base e simulação 4D servem exclusivamente ao apoio ao planejamento e **jamais substituem** verificação técnica formal nem a supervisão de profissional legalmente habilitado com emissão de **ART (CREA)** ou **RRT (CAU)** (Leis nº 5.194/1966, nº 6.496/1977 e nº 12.378/2010). Caso o usuário utilize resultados exclusivamente para decisões críticas ou de produção, assume integral e exclusiva responsabilidade.

---

### 📚 Índice Completo da Documentação

- 🏛️ [Arquitetura do Sistema e Subsistemas C++23](docs/architecture.md#ptbr)
- 📖 [Guia do Usuário](docs/user-guide.md#ptbr) · [Manual Operacional Completo (`MANUAL.md`)](MANUAL.md)
- 📋 [Conformidade Algorítmica, Acessibilidade e Validação CTest](docs/compliance.md#ptbr)
- ⚖️ [Duplo Licenciamento, `source/assets/` e Checklist Store](docs/licensing.md#ptbr)
- 🔒 [Política de Privacidade Independente (LGPD / Store 10.5)](docs/privacy-policy.md#ptbr)
- 🛡️ [Modelo de Segurança, Persistência Atômica e Ameaças](docs/security-model.md#ptbr)
- 🚀 [Notas de Lançamento v2026.11.22.0](docs/release-v2026.11.22.0.md#ptbr)
- 🔍 [Observação Técnica e Snapshot (`OBSERVACAO.md`)](OBSERVACAO.md#ptbr)
- 💡 [Backlog Técnico e Sugestões de Issues](docs/suggested-issues.md#ptbr)
- 🔬 [Auditoria Técnica, ADRs, Matriz V&V e Roadmap](docs/technical-audit-and-roadmap.md#ptbr)
- 🤝 [Guia de Contribuição (`CONTRIBUTING.md`)](CONTRIBUTING.md#ptbr) · [Código de Conduta (`CODE_OF_CONDUCT.md`)](CODE_OF_CONDUCT.md#ptbr) · [Política de Segurança (`SECURITY.md`)](SECURITY.md#ptbr)

</details>

---

## <a id="enus"></a><a id="english-us"></a>🇺🇸 English (US)

> **Note:** This repository refers to version **v2026.11.22.0** of the EISENHOWER ORGANIZER project. Support the project and get the official edition via: [Install from Microsoft Store](https://apps.microsoft.com/detail/9P289X0185C3)

> **Note:** This repository refers to version **v2026.11.22.0** of the 📚 Lúmen Project. Support the project and get the official edition via: [Install from Microsoft Store](https://apps.microsoft.com/detail/9N70CLLMVRPN)

<details>
<summary><b>Click to expand / collapse English (US) documentation</b></summary><br>

# EISENHOWER ORGANIZER / LÚMEN — Task Organizer, Active Study, Accessible Reader & 4D BIM Gantt Suite

Version: v2026.11.22.0<br>
Technical revision date: November 22, 2026<br>
Author: Fernando Nillsson Cidade<br>

## Overview & Dual Product Identity

**EISENHOWER ORGANIZER / LÚMEN** is a native Windows desktop platform engineered in **modern C++23 with Qt 6.12.0**, originally designed to support personal task organization, active learning, accessible reading (with dedicated care for ADHD, dyslexia, and ASD users), and project timeline management.

Built from a single unified codebase (`BUILD_TARGET=EISENHOWER` or `LUMEN` via [`AppConfig`](source/utils/AppConfig.hpp)), the project produces two independent applications with isolated local storage directories and seamless runtime language switching (`pt-BR` / `en-US`):

| Identity (`BUILD_TARGET`) | Executable / Persistent Storage | Primary Focus | Official Microsoft Store |
|---|---|---|---|
| **EISENHOWER ORGANIZER** (`EISENHOWER`) | `EisenhowerOrganizer.exe`<br>`%LOCALAPPDATA%\EisenhowerOrganizer` | Quadrant-based productivity, Pomodoro execution, academic schedules, and project/engineering timelines | [Microsoft Store (`9P289X0185C3`)](https://apps.microsoft.com/detail/9P289X0185C3) |
| **LÚMEN** (`LUMEN`) | `Lumen.exe`<br>`%LOCALAPPDATA%\Lumen` | Study & cognitive-accessibility platform (ADHD, dyslexia, ASD) integrating all 7 workspaces | [Microsoft Store (`9N70CLLMVRPN`)](https://apps.microsoft.com/detail/9N70CLLMVRPN) |

---

### ⚖️ Copyright Holder Manifesto & Dual-Licensing Model

> **Express Statement by Fernando Nillsson Cidade:**  
> I, **Fernando Nillsson Cidade**, as the sole author, original creator, and exclusive copyright holder of **EISENHOWER ORGANIZER** and **LÚMEN**, adopt a transparent **Dual-Licensing** framework:  
> 1. **Public Source Code (Open Source under GNU AGPLv3):** I authorize the use, study, copying, modification, compilation, and distribution of the public source code in this repository strictly under **[LICENSE](LICENSE)** (**GNU Affero General Public License v3 — GNU AGPLv3**, SPDX identifier `AGPL-3.0-only`), reserving name and trademark rights under Section 7(e) (`"EISENHOWER ORGANIZER"` and `"LÚMEN"`). All derivative works must retain original authorship attribution and provide corresponding source code under **GNU AGPLv3** (including network interaction obligations under Section 13).  
> 2. **Full Developer Freedom on `source/assets/`:** Compliance documents stored under [`source/assets/eisenhower/`](source/assets/eisenhower/COPYRIGHT/COPYRIGHT%20AND%20TRADEMARK%20NOTICE_en_US.txt) and [`source/assets/lumen/`](source/assets/lumen/COPYRIGHT/COPYRIGHT%20AND%20TRADEMARK%20NOTICE_en_US.txt) (`ABOUT`, `CLC`, `COPYRIGHT`, `EULA`, `LICENSES`, `NOTICES`, `PRIVACY_POLICY`, and `RELEASE`) were written to support official Microsoft Store distribution. Fork maintainers under **GNU AGPLv3** are **expressly free** to edit, remove, replace, or adapt these files for their own builds, provided the author's exclusive brand names and official logos are removed to prevent confusion with official Store editions and local laws plus third-party licenses are respected.  
> 3. **Author's Signed Store Binaries & Commercial Licensing (`EULA` + `CLC`):** Official compiled, digitally signed packages distributed by the author on the Microsoft Store ([Eisenhower Organizer](https://apps.microsoft.com/detail/9P289X0185C3) and [Lúmen](https://apps.microsoft.com/detail/9N70CLLMVRPN), featuring a 7-day trial managed by `TrialManager` and commercial tiers specified in `CLC`) are governed by the author's **[Official EULA](source/assets/eisenhower/EULA/EULA_en_US.txt)** and **[Commercial Licensing Agreement — CLC](source/assets/eisenhower/CLC/CLC_en_US.txt)**. Third parties are strictly prohibited from reselling, republishing, or commercially exploiting the author's signed Store binaries or misappropriating the `EISENHOWER ORGANIZER` or `LÚMEN` marks.  
> 4. **Support Channel, Microsoft Independence & Legal Disclaimer ("AS IS"):** **Microsoft does NOT provide customer support and does NOT cover these applications under Microsoft's Privacy Statement.** All developer support is offered **exclusively via email (`linceu_lighthouse@outlook.com`) and GitHub Issues/Discussions** on a **voluntary best-effort basis with no SLA or response-time guarantee**. Software and modules are delivered **"AS IS"** as **auxiliary** tools for productivity, study, reading accessibility, and scheduling support, subject to software limitations and independent human verification. See [`docs/licensing.md`](docs/licensing.md#enus).

---

### 🧩 Integrated Native Modules & On-Demand Local AI

The architecture integrates 7 workspaces and the on-demand Local AI `Modelo` service directly into each host executable (while also maintaining standalone modular DLL targets under `source/modules/` and contract verification in `tests/`):

| Module | Core Capabilities | Technical Stack / Persistence |
|---|---|---|
| **1. Eisenhower (Host)** | 4-quadrant task matrix (`Pending`, `In Progress`, `Completed`), `Vertical/Horizontal Tabbed` layouts (collapsible 90°-rotated columns), `Quadrants`, and `2x2 Matrix`, global calendar, recurrences, local reminders, `JSON`/`CSV`/`XLSX`/`PDF` import/export | `UnifiedTaskStore` (`QLockFile` + `QSaveFile`, `tarefas.json` + `.bak`), responsive `EditTaskDialog` |
| **2. Pomodoro** | Focus/short-break/long-break timer, auto-advance cycles with WAV alarms, Kanban board (`Todo`/`Doing`/`Done`), cycle goal discrepancy detection (`CycleMismatchDialog`) | `Qt6::Multimedia`, bidirectional `EventBus` sync |
| **3. Feynman** | 4-stage active recall (`Concept`, `Explain Simply`, `Knowledge Gaps`, `Final Review`), priority/mastery filters, spaced reviews, linked external files | Background worker persistence (`conceitos_feynman.json`) |
| **4. Mind Map** | Visual concept graph, `Vertical`/`Horizontal`/`Radial`/`Tree` layouts, focus mode, document extractors (`PDF`, `EPUB`, `DOC/DOCX`, `TXT`, `MD`, `CSV`, `XLSX`, `JSON`, `DB/SQLite`, `XML`), heuristic NLP/TF-IDF summarizer, JSON + PNG export | `Qt6::OpenGLWidgets`, `Qt6::Pdf`, `Qt6::CorePrivate::QZipReader`, `mapa_mental_autosave.json` |
| **5. Modelo (Local AI)** | On-demand `qwen2.5-3b-instruct-q4_k_m.gguf` (~2.17 GB) Hugging Face downloader with HTTP Range pause/resume/delete, offline `llama.cpp b8987` CPU/Vulkan inference, and `Ollama` (`localhost:11434`) assistant & model discovery | `ModelDownloadDialog`, `OllamaInstrucoesDialog`, seamless heuristic fallback |
| **6. Audiobook** | Accessible text and PDF reader, bookmarks, search, live word/sentence/paragraph speech highlight, line-by-line **Focus Ruler**, offline SAPI voices, and optional Edge TTS online neural voices | `Qt6::Pdf`, `Qt6::PdfWidgets`, `Qt6::WebSockets`, `audiolivro_config.json` |
| **7. Agenda** | Academic assessment and custom nested-tree checklist planner, `Black`/`Colored` display modes configurable globally or per module, linked external files | `Qt6::Sql` (`QSQLITE`, `listas.db`), `agenda_config.json`, `agendas/*.json` |
| **8. 4D BIM Gantt Schedule** | Hierarchical WBS/EAP tree, DAG cycle validation (`DependencyValidator` for `FS`/`SS`/`FF`/`SF` + lag), Critical Path Method (`CpmEngine` `ES/EF/LS/LF/Float`), baselines, field progress log (`FieldProgressService`), `Bim4DService` playback, MS Project XML, CSV mapper, audit log, optional local REST API/Webhooks with RBAC/rate-limit | `Qt6::Sql` (`gantt_database.db`), `AuditService`, `SecurityService` |

---

### 📌 Technical Specifications & Build Architecture

- **Standard & Framework:** **Strict C++23** (`CMAKE_CXX_STANDARD 23`, `CMAKE_CXX_EXTENSIONS OFF`) + **Qt 6.12.0** (`Core`, `Gui`, `Widgets`, `OpenGL`, `OpenGLWidgets`, `Pdf`, `PdfWidgets`, `Sql`, `Multimedia`, `Network`, `WebSockets`, `Test`, `LinguistTools`).
- **Supported Windows 10/11 x64 Toolchains:**
  - **Pure MSVC x64:** `Visual Studio 18 2026` BuildTools (`MSVC 14.52.37025`, Windows SDK `10.0.28000.0`, `"Visual Studio 18 2026"` generator, `build\build_*_msvc`).
  - **MinGW-w64 x64:** `GCC 15.1.0` (`C:\Qt\Tools\mingw1510_64`) + `Ninja` (`build\build_*_mingw`).
- **Clean External Distributions & Compact Inno Setup Installers:**
  - Slim deployment trees in `D:\MISCELANEAS\VSCode\EisenhowerOrganizer\Eisenhower_Organizer_cpp_{msvc,mingw}` and `D:\MISCELANEAS\VSCode\Lumen\Lumen_cpp_{msvc,mingw}` containing strictly required executables, `.qm` translations, icons, runtime docs under `assets/`, and direct runtime DLL dependencies.
  - Inno Setup scripts under `library/` (`Eisenhower_Organizer.iss`, `Lumen.iss`, plus `*_MSVC.iss` / `*_MinGW.iss`) enforce **app-local CRT deployment** (the 5 VC++ Runtime DLLs on MSVC, **with zero embedded `vc_redist.x64.exe`**), keeping installers compact (~`66.8 MB` MSVC vs ~`73.3 MB` MinGW) without an artificial ~18 MB size penalty.
- **Unified Test Suite (`tests/` — 11 CTest Executables):**
  - Strictly local build trees in `build/build_suite_testes_msvc` and `build/build_suite_testes_mingw` (**never distributed externally**), validating all subsystems: `test_core_utils`, `test_store_models`, `test_ui_host`, `test_plugin_pomodoro`, `test_plugin_feynman`, `test_plugin_mapamental`, `test_plugin_modelo`, `test_plugin_audiolivro`, `test_plugin_agenda`, `test_plugin_gantt`, and `test_frameworks_plugins`.

---

### <a id="microsoft-store-privacy-support"></a>🛡️ Microsoft Store Compliance, Privacy & Local-First Design

> [!IMPORTANT]
> **Microsoft Store Compliance (Policies 10.2 & 10.5):**
> - **Independent Privacy Policy:** **Microsoft does NOT supply a default privacy policy for EISENHOWER ORGANIZER or LÚMEN, and these apps are NOT covered by Microsoft's [Privacy Statement](https://www.microsoft.com/en-us/privacy/privacystatement).** All application behavior is governed solely by our [Privacy Policy](docs/privacy-policy.md#enus) and in-app documents under `source/assets/{eisenhower,lumen}/PRIVACY_POLICY/`.
> - **Independent Developer Support:** **Microsoft does NOT provide customer support for these apps.** Technical inquiries go directly to `Fernando Nillsson Cidade` at `linceu_lighthouse@outlook.com` or [GitHub Issues](https://github.com/fernandoncidade/Eisenhower_Organizer_Lumen_cpp/issues) on a voluntary best-effort basis with no SLA.
> - **Local-First with Optional User-Initiated Networking:** User tasks, active study data, SQLite databases (`listas.db`, `gantt_database.db`), mind maps, and logs stay strictly local under `%LOCALAPPDATA%\EisenhowerOrganizer` and `%LOCALAPPDATA%\Lumen`. Network access occurs solely on explicit user request (HTTPS GGUF weight download from Hugging Face, loopback `127.0.0.1:11434` Ollama queries, optional Edge TTS neural synthesis in Audiobook, or user-enabled local Gantt REST/Webhook endpoints).

---

### <a id="quick-start-building"></a>🚀 Building & Running Tests from Source

```powershell
# Clone repository
git clone https://github.com/fernandoncidade/Eisenhower_Organizer_Lumen_cpp.git
cd Eisenhower_Organizer_Lumen_cpp

# 1. Build & deploy clean hosts (MSVC and MinGW)
cmake --preset host-qt6-msvc-release && cmake --build --preset host-qt6-msvc-release --parallel
cmake --preset host-lumen-qt6-msvc-release && cmake --build --preset host-lumen-qt6-msvc-release --parallel
cmake --preset host-qt6-mingw-release && cmake --build --preset host-qt6-mingw-release --parallel
cmake --preset host-lumen-qt6-mingw-release && cmake --build --preset host-lumen-qt6-mingw-release --parallel

# 2. Build & run the full Unified Test Suite (11 CTest executables on MSVC and MinGW)
cmake --preset suite-testes-msvc-release && cmake --build --preset suite-testes-msvc-release --parallel
ctest --preset suite-testes-msvc-release --output-on-failure

cmake --preset suite-testes-mingw-release && cmake --build --preset suite-testes-mingw-release --parallel
ctest --preset suite-testes-mingw-release --output-on-failure
```

*(See [`CMakePresets.json`](CMakePresets.json), [`.vscode/tasks.json`](.vscode/tasks.json), and the Inno Setup scripts under [`library/`](library/Eisenhower_Organizer.iss) for standalone plugin presets, VS Code task labels, and installer commands.)*

---

### 🏛️ Auxiliary Scope & Legal Disclaimer ("AS IS")

Both source builds under **GNU AGPLv3** and official Store binaries are supplied as **auxiliary software tools** **"AS IS"**, without express or implied warranties (Brazilian Law No. 9.609/1998, Law No. 9.610/1998, Civil Code, Consumer Defense Code, and [GNU AGPLv3](LICENSE) Sections 15–16):
- **Local AI (`Qwen 2.5` / `Ollama`) & NLP:** Generated concept trees and summaries are probabilistic aids requiring human verification.
- **Assisted Reading (`Audiobook`):** Reading accessibility utility; not a certified medical or clinical device.
- **4D BIM Gantt Scheduling:** CPM, DAG, baseline, and 4D timeline calculations are auxiliary scheduling aids and **never replace** licensed engineering verification or legally required **ART (CREA)** / **RRT (CAU)** oversight. Exclusive reliance for critical production decisions rests solely on the user.

---

### 📚 Complete Documentation Index

- 🏛️ [System Architecture & C++23 Subsystems](docs/architecture.md#enus)
- 📖 [User Guide](docs/user-guide.md#enus) · [Full Operations Manual (`MANUAL.md`)](MANUAL.md)
- 📋 [Algorithmic Compliance, Accessibility & CTest Verification](docs/compliance.md#enus)
- ⚖️ [Dual Licensing, `source/assets/` Freedom & Store Checklist](docs/licensing.md#enus)
- 🔒 [Independent Privacy Policy (LGPD / Store 10.5)](docs/privacy-policy.md#enus)
- 🛡️ [Security Model, Atomic Persistence & Threat Boundaries](docs/security-model.md#enus)
- 🚀 [Release Notes v2026.11.22.0](docs/release-v2026.11.22.0.md#enus)
- 🔍 [Technical Observation & Snapshot (`OBSERVACAO.md`)](OBSERVACAO.md#enus)
- 💡 [Technical Backlog & Suggested Issues](docs/suggested-issues.md#enus)
- 🔬 [Technical Audit, ADRs, V&V Matrix & Roadmap](docs/technical-audit-and-roadmap.md#enus)
- 🤝 [Contributing Guide (`CONTRIBUTING.md`)](CONTRIBUTING.md#enus) · [Code of Conduct (`CODE_OF_CONDUCT.md`)](CODE_OF_CONDUCT.md#enus) · [Security Policy (`SECURITY.md`)](SECURITY.md#enus)

</details>
