# AI Browser Debug

This directory contains the optional browser-debug feature based on Playwright.

## Ownership

Everything in this directory belongs to the detachable AI/browser-debug tooling layer.

If this directory is removed:

- the Playwright scripts disappear
- the JSON interaction sequences disappear
- the local Node dependencies disappear
- the main project still builds natively

## Contents

- `package.json`: local Playwright dependencies
- `playwright.config.js`: web test configuration
- `scripts/`: wrappers and runners
- `sequences/`: editable interaction sequences
- `tests/`: browser smoke tests

## Usage

From the repository root:

```bash
./.tools/ai-browser-debug/scripts/build_web.sh
./.tools/ai-browser-debug/scripts/serve_web.sh
./.tools/ai-browser-debug/scripts/test_web.sh
./.tools/ai-browser-debug/scripts/open_web_debug.sh
./.tools/ai-browser-debug/scripts/run_web_sequence_headed.sh
```

Or directly:

```bash
./.tools/ai-browser-debug/scripts/run_web_sequence_headed.sh
```
