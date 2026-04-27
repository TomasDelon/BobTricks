(() => {
const THEME_KEY = "bt-presentation-theme";
const NOTES_KEY = "bt-presentation-notes-open";
const NOTES_WIDTH_KEY = "bt-presentation-notes-width";

function hexToRgb(hex) {
  const normalized = hex.replace("#", "");
  if (normalized.length !== 6) return null;
  const value = Number.parseInt(normalized, 16);
  return {
    r: (value >> 16) & 255,
    g: (value >> 8) & 255,
    b: value & 255
  };
}

function escapeHtml(value) {
  return value
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll(">", "&gt;");
}

function renderInline(text) {
  // Extract inline math $...$ before HTML-escaping so < > & inside are not mangled
  const maths = [];
  text = text.replace(/\$([^$\n]+)\$/g, (_, inner) => {
    maths.push(`$${inner}$`);
    return `\x00M${maths.length - 1}\x00`;
  });
  let out = escapeHtml(text)
    .replace(/`([^`]+)`/g, "<code>$1</code>")
    .replace(/\*\*([^*]+)\*\*/g, "<strong>$1</strong>")
    .replace(/\*([^*]+)\*/g, "<em>$1</em>")
    .replace(/\[([^\]]+)\]\(([^)]+)\)/g, '<a href="$2" target="_blank" rel="noreferrer">$1</a>');
  // Restore inline math (unescaped — KaTeX will process it)
  return out.replace(/\x00M(\d+)\x00/g, (_, i) => maths[Number(i)]);
}

function renderMarkdown(markdown) {
  const lines = markdown.replace(/\r\n/g, "\n").split("\n");
  const html = [];
  let inList = false;
  let inCode = false;
  let inMathBlock = false;
  let codeLines = [];
  let mathLines = [];

  const closeList = () => {
    if (inList) {
      html.push("</ul>");
      inList = false;
    }
  };

  const closeCode = () => {
    if (inCode) {
      html.push(`<pre><code>${escapeHtml(codeLines.join("\n"))}</code></pre>`);
      inCode = false;
      codeLines = [];
    }
  };

  const closeMathBlock = () => {
    if (inMathBlock) {
      html.push(`$$\n${mathLines.join("\n")}\n$$`);
      inMathBlock = false;
      mathLines = [];
    }
  };

  for (const line of lines) {
    if (line.trim() === "$$") {
      closeList();
      closeCode();
      if (inMathBlock) {
        closeMathBlock();
      } else {
        inMathBlock = true;
      }
      continue;
    }

    if (inMathBlock) {
      mathLines.push(line);
      continue;
    }

    if (line.startsWith("```")) {
      if (inCode) {
        closeCode();
      } else {
        closeList();
        inCode = true;
      }
      continue;
    }

    if (inCode) {
      codeLines.push(line);
      continue;
    }

    if (!line.trim()) {
      closeList();
      html.push("");
      continue;
    }

    const heading = line.match(/^(#{1,3})\s+(.*)$/);
    if (heading) {
      closeList();
      const level = heading[1].length;
      html.push(`<h${level}>${renderInline(heading[2])}</h${level}>`);
      continue;
    }

    const bullet = line.match(/^[-*]\s+(.*)$/);
    if (bullet) {
      if (!inList) {
        html.push("<ul>");
        inList = true;
      }
      html.push(`<li>${renderInline(bullet[1])}</li>`);
      continue;
    }

    closeList();
    html.push(`<p>${renderInline(line)}</p>`);
  }

  closeList();
  closeCode();
  closeMathBlock();
  return html.join("\n");
}

function renderLatex(element) {
  if (!window.renderMathInElement || !element) return;
  window.renderMathInElement(element, {
    delimiters: [
      { left: "$$", right: "$$", display: true },
      { left: "$", right: "$", display: false }
    ],
    throwOnError: false,
    strict: "ignore",
    ignoredTags: ["script", "noscript", "style", "textarea", "pre", "code"]
  });
}

function highlightCode(source) {
  const pattern = /(\/\/.*$|\/\*[\s\S]*?\*\/|"(?:\\.|[^"])*"|'(?:\\.|[^'])*'|\b(?:const|constexpr|return|if|else|switch|case|break|continue|for|while|do|class|struct|enum|namespace|public|private|protected|template|typename|using|auto|static|inline|virtual|override|noexcept|new|delete|try|catch|throw)\b|\b(?:void|bool|char|short|int|long|float|double|size_t|Vec2|InputFrame|ScenarioDef|TelemetryRow|std)\b|\b\d+(?:\.\d+)?\b|(?:[A-Za-z_]\w*)(?=\s*\())/gm;

  return escapeHtml(source).replace(pattern, (match) => {
    if (match.startsWith("//") || match.startsWith("/*")) return `<span class="cm">${match}</span>`;
    if (match.startsWith('"') || match.startsWith("'")) return `<span class="st">${match}</span>`;
    if (/^\d/.test(match)) return `<span class="nu">${match}</span>`;
    if (/^(const|constexpr|return|if|else|switch|case|break|continue|for|while|do|class|struct|enum|namespace|public|private|protected|template|typename|using|auto|static|inline|virtual|override|noexcept|new|delete|try|catch|throw)$/.test(match)) return `<span class="kw">${match}</span>`;
    if (/^(void|bool|char|short|int|long|float|double|size_t|Vec2|InputFrame|ScenarioDef|TelemetryRow|std)$/.test(match)) return `<span class="ty">${match}</span>`;
    return `<span class="fn">${match}</span>`;
  });
}

function applyTheme(theme) {
  document.documentElement.dataset.theme = theme;
  localStorage.setItem(THEME_KEY, theme);
  document.dispatchEvent(new CustomEvent("bt:theme-change", { detail: theme }));
}

function toggleTheme() {
  const next = document.documentElement.dataset.theme === "light" ? "dark" : "light";
  applyTheme(next);
}

function initTheme() {
  const theme = localStorage.getItem(THEME_KEY) || "dark";
  applyTheme(theme);
}

class BtCodeWindow extends HTMLElement {
  connectedCallback() {
    if (this.dataset.ready) return;
    this.dataset.ready = "true";
    this.classList.add("bt-code-window");
    const source = this.textContent.replace(/^\n+|\n+\s*$/g, "");
    this.innerHTML = `
      <div class="bt-code-window__dots" aria-hidden="true">
        <span></span><span></span><span></span>
      </div>
      <pre class="bt-code-window__pre"><code>${highlightCode(source)}</code></pre>
    `;
  }
}

class BtMermaidViewer extends HTMLElement {
  connectedCallback() {
    if (this.__btReady) return;
    this.__btReady = true;
    this.dataset.ready = "true";
    this.classList.add("bt-mermaid-viewer");
    this.src = this.getAttribute("src") || "";
    this.zoom = 1;
    this.dragState = null;
    this.innerHTML = `
      <button type="button" class="bt-mermaid-viewer__trigger" aria-label="Open diagram in fullscreen view">
        <div class="bt-mermaid-viewer__surface"></div>
      </button>
    `;

    this.triggerEl = this.querySelector(".bt-mermaid-viewer__trigger");
    this.previewSurfaceEl = this.querySelector(".bt-mermaid-viewer__surface");
    this.overlayEl = this.createOverlay();
    this.hostSlideRoot = this.closest("[data-mermaid-slide-root]") || this.closest(".bt-science-slide") || this.parentElement;
    this.hostSlideRoot.appendChild(this.overlayEl);
    this.overlayViewportEl = this.overlayEl.querySelector(".bt-mermaid-overlay__viewport");
    this.overlayCanvasEl = this.overlayEl.querySelector(".bt-mermaid-overlay__canvas");
    this.statusEl = this.overlayEl.querySelector(".bt-mermaid-overlay__status");
    this.debug = this.getAttribute("debug") === "true";

    this.boundOpenFromPreview = (event) => {
      if (this.debug) {
        this.dataset.lastEvent = `${event.type}:${event.target?.className || event.target?.tagName || "unknown"}`;
      }
      event.preventDefault();
      event.stopPropagation();
      this.openDialog();
    };
    this.addEventListener("click", this.boundOpenFromPreview, true);
    this.triggerEl.addEventListener("click", this.boundOpenFromPreview);
    this.previewSurfaceEl.addEventListener("click", this.boundOpenFromPreview);
    this.triggerEl.addEventListener("pointerup", this.boundOpenFromPreview);
    this.overlayEl.addEventListener("click", (event) => {
      if (event.target === this.overlayEl) this.closeDialog();
    });
    this.overlayEl.querySelector("[data-action='zoom-in']").addEventListener("click", () => this.adjustZoom(0.2));
    this.overlayEl.querySelector("[data-action='zoom-out']").addEventListener("click", () => this.adjustZoom(-0.2));
    this.overlayEl.querySelector("[data-action='reset']").addEventListener("click", () => this.setZoom(1));
    this.overlayEl.querySelector("[data-action='close']").addEventListener("click", () => this.closeDialog());
    this.overlayViewportEl.addEventListener("wheel", (event) => {
      if (!this.overlayEl.classList.contains("is-open")) return;
      event.preventDefault();
      this.adjustZoom(event.deltaY < 0 ? 0.12 : -0.12);
    }, { passive: false });
    this.overlayViewportEl.addEventListener("pointerdown", (event) => this.onPointerDown(event));
    window.addEventListener("pointermove", this.boundPointerMove = (event) => this.onPointerMove(event));
    window.addEventListener("pointerup", this.boundPointerUp = () => this.onPointerUp());
    this.boundKeyDown = (event) => {
      if (!this.overlayEl.classList.contains("is-open")) return;
      if (event.key === "Escape") {
        this.closeDialog();
        return;
      }
      if (event.key === "+" || event.key === "=") {
        event.preventDefault();
        this.adjustZoom(0.2);
      }
      if (event.key === "-") {
        event.preventDefault();
        this.adjustZoom(-0.2);
      }
      if (event.key === "0") {
        event.preventDefault();
        this.setZoom(1);
      }
    };
    window.addEventListener("keydown", this.boundKeyDown);
    this.renderStaticPreview();
  }

  disconnectedCallback() {
    this.removeEventListener("click", this.boundOpenFromPreview, true);
    window.removeEventListener("keydown", this.boundKeyDown);
    window.removeEventListener("pointermove", this.boundPointerMove);
    window.removeEventListener("pointerup", this.boundPointerUp);
    if (this.overlayEl?.classList.contains("is-open")) {
      document.body.style.overflow = "";
    }
    this.overlayEl?.remove();
  }

  createOverlay() {
    const overlay = document.createElement("div");
    overlay.className = "bt-mermaid-overlay";
    overlay.innerHTML = `
      <div class="bt-mermaid-overlay__controls">
        <button type="button" data-action="zoom-out" aria-label="Zoom out">-</button>
        <div class="bt-mermaid-overlay__status">100%</div>
        <button type="button" data-action="zoom-in" aria-label="Zoom in">+</button>
        <button type="button" data-action="reset" aria-label="Reset zoom">100%</button>
        <button type="button" data-action="close" aria-label="Close fullscreen view">Close</button>
      </div>
      <div class="bt-mermaid-overlay__viewport">
        <div class="bt-mermaid-overlay__canvas"></div>
      </div>
    `;
    return overlay;
  }

  renderStaticPreview() {
    if (!this.src) {
      const fallback = `<pre class="bt-code-window__pre"><code>Missing svg src</code></pre>`;
      this.previewSurfaceEl.innerHTML = fallback;
      this.overlayCanvasEl.innerHTML = fallback;
      return;
    }
    const previewAlt = this.getAttribute("alt") || "Architecture overview";
    this.previewSurfaceEl.innerHTML = `<img src="${escapeHtml(this.src)}" alt="${escapeHtml(previewAlt)}">`;
    this.overlayCanvasEl.innerHTML = `<img src="${escapeHtml(this.src)}" alt="${escapeHtml(previewAlt)}">`;
  }

  openDialog() {
    if (this.debug) {
      this.dataset.modalState = "opening";
      this.dataset.overlayParent = this.overlayEl.parentElement?.className || this.overlayEl.parentElement?.tagName || "none";
    }
    this.overlayEl.classList.add("is-open");
    this.setZoom(1);
    this.overlayViewportEl.scrollTop = 0;
    this.overlayViewportEl.scrollLeft = 0;
    if (this.debug) {
      this.dataset.modalState = this.overlayEl.classList.contains("is-open") ? "open" : "failed";
    }
  }

  closeDialog() {
    this.overlayEl.classList.remove("is-open");
    this.onPointerUp();
    if (this.debug) {
      this.dataset.modalState = "closed";
    }
  }

  adjustZoom(delta) {
    this.setZoom(this.zoom + delta);
  }

  setZoom(value) {
    this.zoom = Math.min(3, Math.max(0.6, Number(value.toFixed(2))));
    this.overlayCanvasEl.style.setProperty("--bt-mermaid-scale", String(this.zoom));
    if (this.statusEl) this.statusEl.textContent = `${Math.round(this.zoom * 100)}%`;
  }

  onPointerDown(event) {
    if (!this.overlayEl.classList.contains("is-open")) return;
    if (event.button !== 0) return;
    this.dragState = {
      startX: event.clientX,
      startY: event.clientY,
      scrollLeft: this.overlayViewportEl.scrollLeft,
      scrollTop: this.overlayViewportEl.scrollTop
    };
    this.overlayViewportEl.classList.add("is-dragging");
  }

  onPointerMove(event) {
    if (!this.dragState) return;
    const dx = event.clientX - this.dragState.startX;
    const dy = event.clientY - this.dragState.startY;
    this.overlayViewportEl.scrollLeft = this.dragState.scrollLeft - dx;
    this.overlayViewportEl.scrollTop = this.dragState.scrollTop - dy;
  }

  onPointerUp() {
    this.dragState = null;
    this.overlayViewportEl?.classList.remove("is-dragging");
  }
}

class BtSlideFrame extends HTMLElement {
  connectedCallback() {
    if (this.dataset.ready) return;
    this.dataset.ready = "true";
    this.classList.add("bt-slide-frame");
    const ratio = this.getAttribute("ratio");
    if (ratio) {
      this.style.setProperty("--slide-aspect", ratio.replace(":", " / "));
    }
    this.innerHTML = `
      <div class="bt-slide-frame__inner">
        <div class="bt-slide-frame__surface">${this.innerHTML}</div>
      </div>
    `;
  }
}

class BtNotesPanel extends HTMLElement {
  connectedCallback() {
    if (this.dataset.ready) return;
    this.dataset.ready = "true";
    this.classList.add("bt-notes-panel");
    this._notes = {};    // slide index (number) → current text in memory
    this._defaults = []; // slide index → default text from HTML attributes
    this._currentSlideIndex = 0;

    this.innerHTML = `
      <div class="bt-notes-panel__head">
        <div class="bt-eyebrow">Speaker Notes</div>
        <button type="button" class="bt-btn bt-notes-panel__export-btn" style="font-size:12px;padding:5px 11px;flex-shrink:0;">Exporter JSON</button>
      </div>
      <div class="bt-notes-panel__body">
        <label class="bt-notes-panel__editor">
          <span class="sr-only">Markdown source</span>
          <textarea></textarea>
        </label>
        <div class="bt-notes-panel__preview"></div>
      </div>
    `;
    this.textarea = this.querySelector("textarea");
    this.preview = this.querySelector(".bt-notes-panel__preview");

    this.textarea.addEventListener("input", () => {
      this._notes[this._currentSlideIndex] = this.textarea.value;
      this.renderPreview();
      this.resizeEditor();
    });
    this.querySelector(".bt-notes-panel__export-btn").addEventListener("click", () => {
      this._flushCurrent();
      this._downloadNotes();
    });
    this.preview.addEventListener("click", () => this.setEditing(true));
    this.textarea.addEventListener("keydown", (event) => {
      if (event.key === "Escape") {
        event.preventDefault();
        this.setEditing(false);
      }
    });
    this.onDocumentPointerDown = (event) => {
      if (!this.isEditing) return;
      if (!this.contains(event.target)) this.setEditing(false);
    };
    document.addEventListener("pointerdown", this.onDocumentPointerDown);
    this.setEditing(false);
  }

  disconnectedCallback() {
    document.removeEventListener("pointerdown", this.onDocumentPointerDown);
  }

  init(defaults) {
    this._defaults = defaults || [];
    const src = window.BT_NOTES || {};
    for (const [k, v] of Object.entries(src)) {
      this._notes[Number(k)] = v;
    }
    this.loadSlide(0, this._defaults[0] || "");
  }

  loadSlide(index, defaultText) {
    this._flushCurrent();
    this._currentSlideIndex = index;
    this.textarea.value = this._notes[index] ?? defaultText ?? this._defaults[index] ?? "";
    this.renderPreview();
    this.resizeEditor();
  }

  _flushCurrent() {
    if (this.textarea) {
      this._notes[this._currentSlideIndex] = this.textarea.value;
    }
  }

  _downloadNotes() {
    const output = {};
    for (let i = 0; i < this._defaults.length; i++) {
      output[i] = this._notes[i] ?? this._defaults[i] ?? "";
    }
    const js = "window.BT_NOTES = " + JSON.stringify(output, null, 2) + ";\n";
    const blob = new Blob([js], { type: "text/javascript" });
    const url = URL.createObjectURL(blob);
    const a = document.createElement("a");
    a.href = url;
    a.download = "notes.js";
    a.click();
    URL.revokeObjectURL(url);
  }

  renderPreview() {
    this.preview.innerHTML = renderMarkdown(this.textarea.value);
    renderLatex(this.preview);
  }

  resizeEditor() {
    this.textarea.style.height = "auto";
    this.textarea.style.height = `${this.textarea.scrollHeight}px`;
  }

  setEditing(isEditing) {
    this.isEditing = isEditing;
    this.classList.toggle("is-editing", isEditing);
    if (isEditing) {
      requestAnimationFrame(() => {
        this.resizeEditor();
        this.textarea.focus();
        const end = this.textarea.value.length;
        this.textarea.setSelectionRange(end, end);
      });
    }
  }

  get value() {
    return this.textarea.value;
  }
}

// ─── DECLARATIVE SLIDE COMPONENT EXPANDER ────────────────────────────────────
// Transforms <bt-slide>, <bt-code>, <bt-video>, etc. into the full HTML
// structure expected by the CSS. Called synchronously in BtPresentationShell
// before the deck is built, so hljs and KaTeX see the final DOM.

function _dots() {
  return '<div class="bt-code-window__dots" aria-hidden="true"><span></span><span></span><span></span></div>';
}
function _vdots() {
  return '<div class="bt-video-window__dots" aria-hidden="true"><span></span><span></span><span></span></div>';
}

function _dedent(text) {
  const lines = text.split('\n');
  const nonEmpty = lines.filter(l => l.trim().length > 0);
  if (!nonEmpty.length) return '';
  const minIndent = Math.min(...nonEmpty.map(l => (l.match(/^ */) || [''])[0].length));
  return lines.map(l => l.slice(minIndent)).join('\n').replace(/^\n+|\n+\s*$/g, '');
}

const _expanders = {
  'bt-video'(el) {
    const src = el.getAttribute('src') || '';
    const label = el.getAttribute('label') || '';
    const div = document.createElement('div');
    div.className = 'bt-video-window';
    div.innerHTML = `${_vdots()}<video autoplay loop muted playsinline${label ? ` aria-label="${escapeHtml(label)}"` : ''}><source src="${escapeHtml(src)}" type="video/mp4"></video>`;
    el.replaceWith(div);
  },

  'bt-video-stack'(el) {
    el.querySelectorAll('bt-video').forEach(v => _expanders['bt-video'](v));
    const div = document.createElement('div');
    div.className = 'bt-video-stack--vertical';
    div.innerHTML = el.innerHTML;
    el.replaceWith(div);
  },

  'bt-code'(el) {
    const file = el.getAttribute('file') || '';
    const variant = el.getAttribute('variant') || '';
    const source = _dedent(el.textContent);
    const div = document.createElement('div');
    div.className = 'bt-code-window bt-snippet-code' + (variant === 'fill' ? ' bt-fit-code' : '');
    div.style.cssText = 'background:#17161d;color:#f3f5f7;border-color:rgba(255,255,255,0.12);';
    if (variant === 'fill') div.style.cssText += 'width:100%;height:100%;min-height:0;';
    div.innerHTML = `${_dots()}${file ? `<p class="bt-window-label">${escapeHtml(file)}</p>` : ''}<pre class="bt-code-window__pre"><code class="language-cpp">${escapeHtml(source)}</code></pre>`;
    el.replaceWith(div);
  },

  'bt-math'(el) {
    const label = el.getAttribute('label') || '';
    const raw = el.innerHTML;
    const eqs = [];
    const re = /\$\$([\s\S]+?)\$\$/g;
    let m;
    while ((m = re.exec(raw)) !== null) {
      eqs.push(`<div class="bt-math-window__eq">$$${m[1]}$$</div>`);
    }
    const div = document.createElement('div');
    div.className = 'bt-code-window bt-math-window bt-math-under-code';
    div.innerHTML = `${_dots()}${label ? `<p class="bt-window-label">${escapeHtml(label)}</p>` : ''}<div class="bt-math-window__body">${eqs.join('')}</div>`;
    el.replaceWith(div);
  },

  'bt-terminal'(el) {
    const label = el.getAttribute('label') || '';
    const div = document.createElement('div');
    div.className = 'bt-code-window bt-terminal-window';
    div.innerHTML = `${_dots()}${label ? `<p class="bt-window-label">${escapeHtml(label)}</p>` : ''}<pre class="bt-code-window__pre">${el.innerHTML.trim()}</pre>`;
    el.replaceWith(div);
  },

  'bt-bullets'(el) {
    const ul = document.createElement('ul');
    ul.className = 'bt-science-slide__bullets';
    ul.innerHTML = el.innerHTML;
    el.replaceWith(ul);
  },

  'bt-table'(el) {
    const label = el.getAttribute('label') || '';
    const div = document.createElement('div');
    div.className = 'bt-code-window';
    div.innerHTML = `${_dots()}${label ? `<p class="bt-window-label">${escapeHtml(label)}</p>` : ''}<table class="bt-trigger-table">${el.innerHTML}</table>`;
    el.replaceWith(div);
  },

  'bt-timeline-grid'(el) {
    const ol = document.createElement('ol');
    ol.className = 'bt-demo-timeline';
    const cols = el.getAttribute('cols') || '2';
    ol.style.gridTemplateColumns = `repeat(${cols}, minmax(0, 1fr))`;
    ol.innerHTML = el.innerHTML;
    el.replaceWith(ol);
  },

  'bt-conclusion-grid'(el) {
    const div = document.createElement('div');
    div.className = 'bt-conclusion-grid';
    div.innerHTML = el.innerHTML;
    el.replaceWith(div);
  }
};

function _expandLeaves(root) {
  // bt-video-stack must run before bt-video so it can handle its own children
  root.querySelectorAll('bt-video-stack').forEach(el => _expanders['bt-video-stack'](el));
  const leaves = ['bt-video', 'bt-code', 'bt-math', 'bt-terminal', 'bt-bullets', 'bt-table', 'bt-timeline-grid', 'bt-conclusion-grid'];
  for (const tag of leaves) {
    root.querySelectorAll(tag).forEach(el => _expanders[tag](el));
  }
}

function expandPresentationComponents(root) {
  root.querySelectorAll('bt-slide').forEach(el => {
    _expandLeaves(el);

    const title = el.getAttribute('title') || '';
    const layout = el.getAttribute('layout') || 'two-col';
    const fit = el.getAttribute('fit') !== 'false';
    const paddingTop = el.getAttribute('padding-top') || '2.4%';

    const leftEl = el.querySelector(':scope > bt-left');
    const rightEl = el.querySelector(':scope > bt-right');

    const leftHTML = leftEl
      ? leftEl.innerHTML
      : Array.from(el.children).filter(c => c.tagName.toLowerCase() !== 'bt-right').map(c => c.outerHTML).join('');
    const rightHTML = rightEl ? rightEl.innerHTML : '';
    const hasRight = !!rightEl && layout !== 'full';
    const rightStretch = rightEl && rightEl.hasAttribute('stretch');

    const gridStyle = !hasRight ? 'grid-template-columns:1fr;' : '';
    const rightColStyle = rightStretch
      ? 'grid-column:3;align-self:stretch;justify-self:stretch;width:100%;height:100%;min-height:0;display:flex;align-items:stretch;'
      : 'grid-column:3;align-self:start;';

    const slideNotes = el.getAttribute('notes') || '';

    const section = document.createElement('section');
    section.className = 'bt-science-slide' + (fit ? ' bt-fit-slide' : '');
    if (slideNotes) section.dataset.slideNotes = slideNotes;
    section.innerHTML = `
      <div class="bt-science-slide__content" style="padding-top:${paddingTop};">
        <div class="bt-science-slide__body"${gridStyle ? ` style="${gridStyle}"` : ''}>
          <div class="bt-science-slide__text bt-science-slide__body-text">
            <h1 class="bt-science-slide__title">${escapeHtml(title)}</h1>
            <div class="bt-left-stack__body">${leftHTML}</div>
          </div>
          ${hasRight ? `<div class="bt-right-stack" style="${rightColStyle}"><div class="bt-right-stack__body">${rightHTML}</div></div>` : ''}
        </div>
      </div>`;
    el.replaceWith(section);
  });
}

// ─────────────────────────────────────────────────────────────────────────────

class BtPresentationShell extends HTMLElement {
  connectedCallback() {
    if (this.dataset.ready) return;
    this.dataset.ready = "true";
    this.classList.add("bt-presentation-shell");
    expandPresentationComponents(this);
    const slideNodes = Array.from(this.children).filter((node) => node.nodeType === Node.ELEMENT_NODE);
    const slideNotesList = slideNodes.map(node =>
      node.dataset.slideNotes || node.dataset.notes || ''
    );
    const defaultSlide = `
      <section class="bt-science-slide">
        <div class="bt-science-slide__text">
          <h1 class="bt-science-slide__title">Structured Slide
System</h1>
          <p class="bt-science-slide__subtitle">Use the mouse wheel to move across slides with a proper transition.</p>
          <p class="bt-science-slide__meta">Theme and speaker notes remain available while the frame stays stable.</p>
        </div>
        <div class="bt-science-slide__visual" aria-hidden="true"></div>
        <div class="bt-science-slide__terrain"></div>
      </section>
    `;
    const slideMarkup = (slideNodes.length ? slideNodes : [{ outerHTML: defaultSlide }])
      .map((node, index) => `
        <div class="bt-presentation-shell__slide${index === 0 ? " is-active" : ""}" data-slide-index="${index}">
          ${node.outerHTML}
        </div>
      `)
      .join("");

    this.innerHTML = `
      <div class="bt-presentation-shell__canvas">
        <div class="bt-presentation-shell__stage">
          <bt-slide-frame ratio="16:9">
            <div class="bt-presentation-shell__deck">${slideMarkup}</div>
            <div class="bt-presentation-shell__counter">1 / ${slideNodes.length || 1}</div>
          </bt-slide-frame>
        </div>
        <aside class="bt-presentation-shell__notes">
          <div class="bt-notes-resize-handle" aria-hidden="true"></div>
          <bt-notes-panel></bt-notes-panel>
        </aside>
      </div>
    `;

    this.notes = this.querySelector(".bt-presentation-shell__notes");
    this.stage = this.querySelector(".bt-presentation-shell__stage");
    this.slideFrame = this.querySelector("bt-slide-frame");
    this.deck = this.querySelector(".bt-presentation-shell__deck");
    this.counter = this.querySelector(".bt-presentation-shell__counter");
    this.slides = Array.from(this.querySelectorAll(".bt-presentation-shell__slide"));
    this.slideNotesList = slideNotesList;
    this.notesPanel = this.querySelector("bt-notes-panel");
    this.currentSlideIndex = 0;
    this.isSlideAnimating = false;
    this.isNotesOpen = localStorage.getItem(NOTES_KEY) === "true";
    this.resizeObserver = new ResizeObserver(() => this.updateSlideScale());
    this.resizeObserver.observe(this.stage);
    this.resizeObserver.observe(this);
    this.onWheel = (event) => {
      if (document.querySelector(".bt-mermaid-overlay.is-open")) {
        event.preventDefault();
        return;
      }
      if (Math.abs(event.deltaY) < 10) return;
      event.preventDefault();
      this.navigateSlides(event.deltaY > 0 ? 1 : -1);
    };
    this.onKeyDown = (event) => {
      if (event.altKey && (event.key === "m" || event.key === "M")) {
        event.preventDefault();
        toggleTheme();
      }
      if (event.altKey && (event.key === "n" || event.key === "N")) {
        event.preventDefault();
        this.toggleNotes();
      }
      if (event.key === "ArrowDown" || event.key === "ArrowUp") {
        const active = document.activeElement;
        const inTextarea = active && active.tagName === "TEXTAREA";
        const inEditable = active && active.isContentEditable;
        if (inTextarea || inEditable) return;
        event.preventDefault();
        this.navigateSlides(event.key === "ArrowDown" ? 1 : -1);
      }
    };
    this.stage.addEventListener("wheel", this.onWheel, { passive: false });
    window.addEventListener("keydown", this.onKeyDown);
    this.setupNotesResize();
    this.classList.toggle("is-notes-open", this.isNotesOpen);
    this.updateCounter();
    requestAnimationFrame(() => {
      this.updateSlideScale();
      this.notesPanel?.init(this.slideNotesList);
    });
  }

  disconnectedCallback() {
    this.stage?.removeEventListener("wheel", this.onWheel);
    window.removeEventListener("keydown", this.onKeyDown);
    this.resizeObserver?.disconnect();
    window.removeEventListener("pointermove", this._onResizeMove);
    window.removeEventListener("pointerup", this._onResizeUp);
  }

  setupNotesResize() {
    const handle = this.querySelector(".bt-notes-resize-handle");
    if (!handle) return;

    const saved = localStorage.getItem(NOTES_WIDTH_KEY);
    if (saved) this.style.setProperty("--notes-width", saved + "px");

    this._onResizeMove = (e) => {
      if (!this._resizeDragging) return;
      const notesEl = this.notes;
      const right = window.innerWidth;
      const w = Math.min(Math.max(right - e.clientX, 260), window.innerWidth * 0.72);
      this.style.setProperty("--notes-width", w + "px");
      localStorage.setItem(NOTES_WIDTH_KEY, Math.round(w));
      this.updateSlideScale();
    };
    this._onResizeUp = () => {
      if (!this._resizeDragging) return;
      this._resizeDragging = false;
      handle.classList.remove("is-dragging");
      document.body.style.cursor = "";
      document.body.style.userSelect = "";
    };

    handle.addEventListener("pointerdown", (e) => {
      e.preventDefault();
      this._resizeDragging = true;
      handle.classList.add("is-dragging");
      document.body.style.cursor = "col-resize";
      document.body.style.userSelect = "none";
    });

    window.addEventListener("pointermove", this._onResizeMove);
    window.addEventListener("pointerup", this._onResizeUp);
  }

  toggleNotes(force) {
    this.isNotesOpen = typeof force === "boolean" ? force : !this.isNotesOpen;
    this.classList.toggle("is-notes-open", this.isNotesOpen);
    localStorage.setItem(NOTES_KEY, String(this.isNotesOpen));
    this.updateSlideScale();
  }

  navigateSlides(direction) {
    if (this.isSlideAnimating) return;
    const nextIndex = this.currentSlideIndex + direction;
    if (nextIndex < 0 || nextIndex >= this.slides.length) return;

    const current = this.slides[this.currentSlideIndex];
    const next = this.slides[nextIndex];
    const enterClass = direction > 0 ? "is-enter-from-bottom" : "is-enter-from-top";
    const exitClass = direction > 0 ? "is-exit-to-top" : "is-exit-to-bottom";

    this.isSlideAnimating = true;
    this.notesPanel?.loadSlide(nextIndex, this.slideNotesList[nextIndex] || '');
    next.classList.add(enterClass);
    void next.offsetHeight;
    next.classList.add("is-active");
    current.classList.add(exitClass);
    next.classList.remove(enterClass);

    window.setTimeout(() => {
      current.classList.remove("is-active", "is-exit-to-top", "is-exit-to-bottom");
      next.classList.remove("is-enter-from-top", "is-enter-from-bottom");
      this.currentSlideIndex = nextIndex;
      this.updateCounter();
      this.isSlideAnimating = false;
    }, 210);
  }

  updateCounter() {
    if (!this.counter) return;
    this.counter.textContent = `${this.currentSlideIndex + 1} / ${this.slides.length}`;
  }

  updateSlideScale() {
    if (!this.stage) return;

    const shellRect = this.getBoundingClientRect();
    const stageRect = this.stage.getBoundingClientRect();
    const shellWidth = Math.max(shellRect.width, 1);
    const shellHeight = Math.max(shellRect.height, 1);

    if (!this.isNotesOpen) {
      this.style.setProperty("--bt-shell-width", `${shellWidth}px`);
      this.style.setProperty("--bt-shell-height", `${shellHeight}px`);
      this.style.setProperty("--bt-slide-scale", "1");
      return;
    }

    const margin = 28;
    const availableWidth = Math.max(stageRect.width - margin * 2, 1);
    const availableHeight = Math.max(stageRect.height - margin * 2, 1);
    const scale = Math.min(availableWidth / shellWidth, availableHeight / shellHeight);

    this.style.setProperty("--bt-shell-width", `${shellWidth}px`);
    this.style.setProperty("--bt-shell-height", `${shellHeight}px`);
    this.style.setProperty("--bt-slide-scale", `${scale}`);
  }
}

if (!customElements.get("bt-code-window")) customElements.define("bt-code-window", BtCodeWindow);
if (!customElements.get("bt-mermaid-viewer")) customElements.define("bt-mermaid-viewer", BtMermaidViewer);
if (!customElements.get("bt-slide-frame")) customElements.define("bt-slide-frame", BtSlideFrame);
if (!customElements.get("bt-notes-panel")) customElements.define("bt-notes-panel", BtNotesPanel);
if (!customElements.get("bt-presentation-shell")) customElements.define("bt-presentation-shell", BtPresentationShell);

function bindControl(input) {
  const target = input.dataset.target;
  const property = input.dataset.cssVar;
  if (!target || !property) return;

  const targetEl = document.querySelector(target);
  if (!targetEl) return;

  const update = () => {
    const unit = input.dataset.unit || "";
    targetEl.style.setProperty(property, `${input.value}${unit}`);
    if (property === "--accent") {
      const rgb = hexToRgb(input.value);
      if (rgb) {
        targetEl.style.setProperty("--accent-soft", `rgba(${rgb.r}, ${rgb.g}, ${rgb.b}, 0.14)`);
        targetEl.style.setProperty("--accent-border", `rgba(${rgb.r}, ${rgb.g}, ${rgb.b}, 0.34)`);
      }
    }
  };

  input.addEventListener("input", update);
  update();
}

function initComponentLab() {
  initTheme();

  document.querySelectorAll("[data-css-var]").forEach(bindControl);

  document.querySelectorAll("[data-theme-toggle]").forEach((button) => {
    button.addEventListener("click", () => toggleTheme());
  });

  const shell = document.querySelector("bt-presentation-shell");
  document.querySelectorAll("[data-toggle-notes]").forEach((button) => {
    button.addEventListener("click", () => shell?.toggleNotes());
  });
}

window.BTPresentation = {
  applyTheme,
  toggleTheme,
  initTheme,
  initComponentLab
};
})();
