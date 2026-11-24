Optional Qwen Model Plugin
===========================

Included model:

- Qwen2.5-3B-Instruct Q4_K_M
- File: qwen2.5-3b-instruct-q4_k_m.gguf
- Source: Qwen/Qwen2.5-3B-Instruct-GGUF
- Revision: 7dabda4d13d513e3e842b20f0d435c732f172cbe
- SHA-256: 626b4a6678b86442240e33df819e00132d3ba7dddfE1cdc4fbb18e0a9615c62d
- Model license: Apache License 2.0 (see LICENSE_QWEN2.5.txt)

This model is textual, multilingual, and was chosen for extracting concepts,
relations, summaries, and hierarchies in JSON. There is no associated multimodal
projector; PDF, DOCX, EPUB, and other formats are converted to text by the
Mind Map plugin before inference.

When the document provides structural markers, processing preserves up to
six levels (work, parts/chapters, and successive subdivisions). The deeper levels
are summarized first; the resulting syntheses feed the parent nodes and
the full summary. Each output is validated for minimum length, duplication, and
alignment with the section. Invalid responses receive a local summary anchored in
the title and in the children, without discarding the entire tree.

It is possible to replace or add another language model in GGUF format.
Models with "instruct" or "chat" in the name are prioritized;
Q4_K_M and Q5_K_M quantizations are preferred when there is more than one option.

It is also possible to configure it without copying the model to the plugin:

- MAPAMENTAL_LLAMA_MODEL: path to a GGUF file or model directory.
- MODELO_QWEN_MODEL: preferred variable name to indicate the plugin GGUF.
- MAPAMENTAL_LLAMA_BACKEND: auto (default), vulkan, or cpu.
- MAPAMENTAL_LLAMA_CONTEXT: context between 4096 and 32768 tokens.
- MAPAMENTAL_LLAMA_PREDICT: output between 512 and 4096 tokens.
- MAPAMENTAL_LLAMA_SEED: reproducible generation seed (default: 7).
- MAPAMENTAL_LLAMA_HIERARCHICAL_BATCH: number of sections per batch, between 4 and
  24 (default: 14). Larger batches tend to be faster; smaller batches may
  improve accuracy in very dense works.
- MAPAMENTAL_LLAMA_TIMEOUT_MS: limit between 30000 and 900000 milliseconds.
- MAPAMENTAL_LLAMA_DISABLE=1: disables local inference and uses only the
  heuristic engine.

Without the Model plugin, without a valid GGUF, or without a functional runtime,
Mind Map ignores this capability and automatically continues with its enhanced
heuristic analysis.
