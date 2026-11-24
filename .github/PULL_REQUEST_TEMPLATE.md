# Modelo de Pull Request / Pull Request Template

<p align="center">
  <b>Selecione o idioma / Select language:</b><br>
  <a href="#ptbr">🇧🇷 Português (BR)</a> | <a href="#enus">🇺🇸 English (US)</a>
</p>

> [!NOTE]
> **Aviso ao Colaborador / Contributor Notice:**  
> 🇧🇷 *Este repositório é um projeto pessoal e educacional mantido pelo autor em ritmo próprio. Pull Requests são bem-vindos, mas a revisão e o merge ocorrem de forma voluntária e sem prazo garantido. Sob a licença **GNU AGPLv3**, você tem plena liberdade para manter e evoluir suas alterações em um fork próprio.*  
> 🇺🇸 *This repository is a personal and educational project maintained by the author at their own pace. Pull Requests are welcome, but review and merging occur voluntarily without any guaranteed timeline. Under **GNU AGPLv3**, you have complete freedom to maintain and distribute your improvements in your own fork.*

---

## <a id="ptbr"></a>🇧🇷 Português (BR)

<details>
<summary>Clique para expandir / recolher o template em português</summary><br>

## Descrição
Descreva sucintamente o objetivo deste PR, o produto ou módulos impactados e qual issue ele soluciona.

## Produto e Módulos Afetados
- [ ] **EISENHOWER ORGANIZER** (`BUILD_TARGET=EISENHOWER`)
- [ ] **LÚMEN** (`BUILD_TARGET=LUMEN`)
- [ ] Host / `UnifiedTaskStore` / UI Principal / Calendário / Recorrências
- [ ] Módulo **Pomodoro**
- [ ] Módulo **Feynman**
- [ ] Módulo **Mapa Mental** (NLP / Extratores EPUB/DOCX/PDF/XLSX)
- [ ] Módulo **Modelo** (IA Local Qwen 2.5 GGUF / `llama.cpp` / Ollama)
- [ ] Módulo **Audiolivro** (Texto / PDF / TTS / Régua de Foco)
- [ ] Módulo **Agenda** (Registro de Atividades / SQLite)
- [ ] Módulo **Gantt & BIM 4D** (CPM / DAG / Simulação 4D / MS Project XML / CSV / REST Security)
- [ ] Utilitários (`mocks/`) / Documentação / Traduções / Scripts CMake & Inno Setup

## Testes e Validação (Obrigatório)
- [ ] Compilado com **Pure MSVC x64** (`Visual Studio 18 2026`, `MSVC 14.52`, Qt `6.12.0`)
- [ ] Compilado com **MinGW-w64 x64** (`GCC 15.1.0`, Qt `6.12.0`, Ninja)
- [ ] Suíte completa aprovada (11/11) em **`suite-testes-msvc-release`** (`build/build_suite_testes_msvc`)
- [ ] Suíte completa aprovada (11/11) em **`suite-testes-mingw-release`** (`build/build_suite_testes_mingw`)
- [ ] Verificada tradução dinâmica `pt_BR` / `en_US` em tempo de execução (`QEvent::LanguageChange`)
- [ ] Distribuição externa limpa preservada (sem `vc_redist.x64.exe`, sem `docs/*.md`, sem testes nas pastas de deploy)

## Issues Relacionadas
Corrige #(número da issue)

</details>

---

## <a id="enus"></a>🇺🇸 English (US)

<details>
<summary>Click to expand / collapse template in English</summary><br>

## Description
Briefly describe the goal of this PR, impacted products or modules, and which issue it resolves.

## Affected Product & Modules
- [ ] **EISENHOWER ORGANIZER** (`BUILD_TARGET=EISENHOWER`)
- [ ] **LÚMEN** (`BUILD_TARGET=LUMEN`)
- [ ] Host / `UnifiedTaskStore` / Main UI / Calendar / Recurrences
- [ ] **Pomodoro** Module
- [ ] **Feynman** Module
- [ ] **Mapa Mental** Module (NLP / EPUB/DOCX/PDF/XLSX Extractors)
- [ ] **Modelo** Module (Local AI Qwen 2.5 GGUF / `llama.cpp` / Ollama)
- [ ] **Audiolivro** Module (Text / PDF / TTS / Focus Ruler)
- [ ] **Agenda** Module (Activity Registration / SQLite)
- [ ] **Gantt & BIM 4D** Module (CPM / DAG / 4D Simulation / MS Project XML / CSV / REST Security)
- [ ] Tools (`mocks/`) / Documentation / Translations / CMake & Inno Setup Scripts

## Build & Verification (Required)
- [ ] Built cleanly with **Pure MSVC x64** (`Visual Studio 18 2026`, `MSVC 14.52`, Qt `6.12.0`)
- [ ] Built cleanly with **MinGW-w64 x64** (`GCC 15.1.0`, Qt `6.12.0`, Ninja)
- [ ] Full unified test suite passed (11/11) in **`suite-testes-msvc-release`** (`build/build_suite_testes_msvc`)
- [ ] Full unified test suite passed (11/11) in **`suite-testes-mingw-release`** (`build/build_suite_testes_mingw`)
- [ ] Live runtime translation `pt_BR` / `en_US` verified (`QEvent::LanguageChange`)
- [ ] Clean external distribution preserved (no `vc_redist.x64.exe`, no `docs/*.md`, no test binaries in deploy dirs)

## Related Issues
Fixes #(issue number)

</details>
