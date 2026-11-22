---
license: apache-2.0
---
# ACSLBench

ACSLBench is a benchmark dataset for natural-language-to-ACSL specification generation and C program verification. Each task contains a natural language requirement, a corresponding ACSL function contract, and an ACSL-annotated C program that can be used for formal verification experiments with Frama-C.

The dataset is designed for research on large language models, program specification generation, formal methods, and safety-critical C software verification.

## Dataset Contents

This repository contains 598 tasks.

| File / Directory | Description |
| --- | --- |
| `acslbench.json` | Original JSON dataset. Each item contains `nl` and `acsl`. |
| `acslbench.csv` | CSV version for Hugging Face loading. It contains `task_id`, `nl`, `acsl`, `c_file`, and `acsl_c`. |
| `ACSL_C/` | ACSL-annotated C source files, from `task_1.c` to `task_598.c`. |
| `ACSL_C.tar` | Archived copy of the ACSL-annotated C source directory. |

Each C file starts with a comment containing the corresponding natural language requirement, followed by the ACSL-annotated C program.

## Fields

The CSV file has the following columns:

| Field | Type | Description |
| --- | --- | --- |
| `task_id` | integer | Task index, from 1 to 598. |
| `nl` | string | Natural language description of the expected C function behavior. |
| `acsl` | string | ACSL function contract. |
| `c_file` | string | Name of the corresponding C source file in `ACSL_C/`. |
| `acsl_c` | string | Full ACSL-annotated C source code. |

## Example

```python
from datasets import load_dataset

dataset = load_dataset("yzrsdad/ACSLBench", data_files="acslbench.csv")
print(dataset["train"][0])
```

A typical item contains a natural language requirement such as asking for an ACSL-annotated C function, the expected ACSL contract, and the full C program with ACSL annotations.

## Suggested Uses

ACSLBench can be used for:

- natural language to ACSL specification generation;
- C function contract generation;
- loop invariant and ACSL annotation synthesis;
- retrieval-augmented generation for formal specifications;
- evaluating LLM-generated ACSL with Frama-C;
- studying semantic consistency between natural language requirements and formal specifications.

## Verification Tooling

The ACSL annotations are intended for use with Frama-C and its WP plugin. Depending on the Frama-C version and local prover configuration, verification outcomes may vary. Users are encouraged to report the Frama-C version, prover backend, timeout, and command-line options when publishing experimental results.