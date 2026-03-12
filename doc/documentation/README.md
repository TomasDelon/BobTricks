# API Documentation

This directory contains the Doxygen configuration used to generate the API reference for the
project.

## Requirements

- `doxygen`

## Generate The Documentation

Run this command from the repository root:

```bash
doxygen doc/documentation/Doxyfile
```

The generated output is written to:

```text
doc/documentation/output/
```

## Current Scope

The current Doxygen input focuses on:

- `README.md`
- `include/`
- `src/`

This is enough for the current project stage and can be extended later as the codebase grows.
