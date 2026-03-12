#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
cd "$ROOT_DIR"

if ! command -v emcmake >/dev/null 2>&1; then
    echo "emcmake was not found. Activate Emscripten before building the web target."
    exit 1
fi

export EM_CACHE="${EM_CACHE:-$ROOT_DIR/build/emscripten-cache}"
mkdir -p "$EM_CACHE"

EM_CONFIG_FILE="$ROOT_DIR/build/emscripten-config.py"
cat > "$EM_CONFIG_FILE" <<EOF
EMSCRIPTEN_ROOT = '/usr/share/emscripten'
LLVM_ROOT = '/usr/bin'
BINARYEN_ROOT = '/usr'
NODE_JS = '/usr/bin/node'
JAVA = 'java'
FROZEN_CACHE = False
CACHE = r'$EM_CACHE'
PORTS = r'$EM_CACHE/ports'
CLOSURE_COMPILER = 'closure-compiler'
LLVM_ADD_VERSION = '15'
CLANG_ADD_VERSION = '15'
EOF
export EM_CONFIG="$EM_CONFIG_FILE"

emcmake cmake -S . -B build/web
cmake --build build/web -j

python3 - <<'PY'
from pathlib import Path

html_path = Path("build/web/bobtricks.html")
html = html_path.read_text(encoding="utf-8")

old_canvas = '<canvas class="emscripten" id="canvas" oncontextmenu="event.preventDefault()" tabindex=-1></canvas>'
new_canvas = '<canvas class="emscripten" id="canvas" oncontextmenu="event.preventDefault()" tabindex="0"></canvas>'
if old_canvas in html:
    html = html.replace(old_canvas, new_canvas, 1)

html = html.replace(' || key === "&"', "")
html = html.replace(' || key === "é"', "")
html = html.replace(' || key === """', "")

focus_snippet = """          canvas.addEventListener("click", function() { canvas.focus(); });
          canvas.addEventListener("mousedown", function() { canvas.focus(); });
          window.addEventListener("keydown", function(event) {
            const code = event.code || "";
            const key = event.key || "";
            const keyCode = event.keyCode || event.which || 0;
            if (code === "Digit1" || key === "1" || keyCode === 49) {
              if (Module._bobtricks_request_mode) Module._bobtricks_request_mode(0);
              event.preventDefault();
              return;
            }
            if (code === "Digit2" || key === "2" || keyCode === 50) {
              if (Module._bobtricks_request_mode) Module._bobtricks_request_mode(1);
              event.preventDefault();
              return;
            }
            if (code === "Digit3" || key === "3" || keyCode === 51) {
              if (Module._bobtricks_request_mode) Module._bobtricks_request_mode(2);
              event.preventDefault();
              return;
            }
            if (code === "Space" || key === " ") {
              if (Module._bobtricks_toggle_pause) Module._bobtricks_toggle_pause();
              event.preventDefault();
              return;
            }
            if (code === "Minus" || key === "-") {
              if (Module._bobtricks_slow_down) Module._bobtricks_slow_down();
              event.preventDefault();
              return;
            }
            if (code === "Equal" || key === "=" || key === "+") {
              if (Module._bobtricks_speed_up) Module._bobtricks_speed_up();
              event.preventDefault();
              return;
            }
            if (code === "KeyR" || key === "r" || key === "R") {
              if (Module._bobtricks_request_reset) Module._bobtricks_request_reset();
              event.preventDefault();
              return;
            }
            if (code === "Escape" || key === "Escape") {
              if (Module._bobtricks_request_quit) Module._bobtricks_request_quit();
              event.preventDefault();
              return;
            }
            if (code === "F11" || key === "F11") {
              if (Module._bobtricks_toggle_fullscreen) Module._bobtricks_toggle_fullscreen();
              event.preventDefault();
            }
          }, true);
          window.addEventListener("load", function() {
            window.setTimeout(function() { canvas.focus(); }, 0);
          });
          window.setTimeout(function() { canvas.focus(); }, 0);
"""

anchor = """          canvas.addEventListener("webglcontextlost", function(e) { alert('WebGL context lost. You will need to reload the page.'); e.preventDefault(); }, false);

          return canvas;
"""

replacement = """          canvas.addEventListener("webglcontextlost", function(e) { alert('WebGL context lost. You will need to reload the page.'); e.preventDefault(); }, false);
""" + focus_snippet + """
          return canvas;
"""

if focus_snippet not in html and anchor in html:
    html = html.replace(anchor, replacement, 1)

html_path.write_text(html, encoding="utf-8")
PY

echo "Web build completed in build/web/"
