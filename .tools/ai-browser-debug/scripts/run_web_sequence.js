const { chromium } = require("@playwright/test");
const http = require("http");
const fs = require("fs");
const path = require("path");

const rootDir = path.resolve(__dirname, "..", "..", "..");
const webDir = path.join(rootDir, "build", "web");
const host = "127.0.0.1";
const port = 8080;
const mode = process.argv[2] || "headless";
const scenarioArg = process.argv[3] || ".tools/ai-browser-debug/sequences/default.json";
const headless = mode !== "headed";
const slowMo = headless ? 0 : 250;
const scenarioPath = path.resolve(rootDir, scenarioArg);

if (!fs.existsSync(path.join(webDir, "bobtricks.html"))) {
  console.error("No existe build/web/bobtricks.html. Ejecuta scripts/build/build_web.sh primero.");
  process.exit(1);
}

if (!fs.existsSync(scenarioPath)) {
  console.error(`No existe la secuencia: ${scenarioPath}`);
  process.exit(1);
}

const scenario = JSON.parse(fs.readFileSync(scenarioPath, "utf8"));

const contentTypes = {
  ".html": "text/html; charset=utf-8",
  ".js": "text/javascript; charset=utf-8",
  ".wasm": "application/wasm",
  ".css": "text/css; charset=utf-8",
  ".json": "application/json; charset=utf-8",
  ".png": "image/png",
  ".jpg": "image/jpeg",
  ".svg": "image/svg+xml"
};

function safePathFromUrl(urlPath) {
  const cleanPath = decodeURIComponent((urlPath || "/").split("?")[0]);
  const requested = cleanPath === "/" ? "/bobtricks.html" : cleanPath;
  const resolved = path.resolve(webDir, `.${requested}`);
  if (!resolved.startsWith(webDir)) {
    return null;
  }
  return resolved;
}

function assertStep(step, field) {
  if (!(field in step)) {
    throw new Error(`El paso '${step.action}' requiere '${field}'`);
  }
}

async function resolveTarget(page, step) {
  if (step.selector) {
    const locator = page.locator(step.selector);
    await locator.waitFor();
    return locator;
  }

  const canvas = page.locator("canvas");
  await canvas.waitFor();
  return canvas;
}

async function canvasPointToPage(page, point) {
  const canvas = page.locator("canvas");
  await canvas.waitFor();
  const box = await canvas.boundingBox();
  if (!box) {
    throw new Error("No se pudo obtener el bounding box del canvas");
  }
  return {
    x: box.x + point.x,
    y: box.y + point.y
  };
}

async function runStep(page, step, summary, screenshotDir) {
  switch (step.action) {
    case "wait":
      assertStep(step, "ms");
      await page.waitForTimeout(step.ms);
      summary.push(`wait ${step.ms}ms`);
      break;
    case "click": {
      const target = await resolveTarget(page, step);
      if (step.x !== undefined && step.y !== undefined) {
        await target.click({ position: { x: step.x, y: step.y } });
        summary.push(`click ${step.selector || "canvas"} @ ${step.x},${step.y}`);
      } else {
        await target.click();
        summary.push(`click ${step.selector || "canvas"}`);
      }
      break;
    }
    case "fill": {
      assertStep(step, "selector");
      assertStep(step, "value");
      await page.locator(step.selector).fill(String(step.value));
      summary.push(`fill ${step.selector}=${step.value}`);
      break;
    }
    case "press":
      assertStep(step, "key");
      await page.keyboard.press(step.key);
      summary.push(`press ${step.key}`);
      break;
    case "move_mouse":
      assertStep(step, "x");
      assertStep(step, "y");
      if (step.relativeToCanvas !== false) {
        const point = await canvasPointToPage(page, { x: step.x, y: step.y });
        await page.mouse.move(point.x, point.y, { steps: step.steps || 1 });
      } else {
        await page.mouse.move(step.x, step.y, { steps: step.steps || 1 });
      }
      summary.push(`move mouse ${step.x},${step.y}`);
      break;
    case "drag": {
      assertStep(step, "from");
      assertStep(step, "to");
      const from = step.relativeToCanvas === false
        ? step.from
        : await canvasPointToPage(page, step.from);
      const to = step.relativeToCanvas === false
        ? step.to
        : await canvasPointToPage(page, step.to);
      await page.mouse.move(from.x, from.y);
      await page.mouse.down();
      await page.mouse.move(to.x, to.y, { steps: step.steps || 10 });
      await page.mouse.up();
      summary.push(
        `drag ${step.from.x},${step.from.y} -> ${step.to.x},${step.to.y}`
      );
      break;
    }
    case "wait_for_selector":
      assertStep(step, "selector");
      await page.locator(step.selector).waitFor({ timeout: step.timeout || 30_000 });
      summary.push(`wait_for_selector ${step.selector}`);
      break;
    case "wait_for_title":
      assertStep(step, "value");
      await page.waitForFunction(
        (expected) => document.title === expected,
        step.value,
        { timeout: step.timeout || 30_000 }
      );
      summary.push(`wait_for_title ${step.value}`);
      break;
    case "screenshot": {
      const fileName = step.path || `step-${String(summary.length + 1).padStart(2, "0")}.png`;
      const outputPath = path.resolve(screenshotDir, fileName);
      await page.screenshot({ path: outputPath });
      summary.push(`screenshot ${path.relative(rootDir, outputPath)}`);
      break;
    }
    case "stop":
      summary.push("stop");
      return false;
    default:
      throw new Error(`Accion no soportada: ${step.action}`);
  }

  return true;
}

const server = http.createServer((req, res) => {
  const filePath = safePathFromUrl(req.url);
  if (!filePath) {
    res.writeHead(403);
    res.end("Forbidden");
    return;
  }

  fs.readFile(filePath, (error, data) => {
    if (error) {
      res.writeHead(404);
      res.end("Not found");
      return;
    }

    res.writeHead(200, {
      "Content-Type": contentTypes[path.extname(filePath)] || "application/octet-stream",
      "Cache-Control": "no-store"
    });
    res.end(data);
  });
});

async function main() {
  await new Promise((resolve, reject) => {
    server.once("error", reject);
    server.listen(port, host, resolve);
  });

  const browser = await chromium.launch({
    headless,
    slowMo
  });
  const page = await browser.newPage({
    viewport: scenario.viewport || { width: 1440, height: 900 }
  });
  const consoleMessages = [];
  const summary = [];
  const screenshotDir = path.join(rootDir, "build", "web", "captures");

  fs.mkdirSync(screenshotDir, { recursive: true });

  page.on("console", (msg) => {
    consoleMessages.push(`${msg.type()}: ${msg.text()}`);
  });

  await page.goto(`http://${host}:${port}/${scenario.entry || "bobtricks.html"}`);
  await page.waitForSelector(scenario.readySelector || "canvas");
  const pointerLockCheckbox = page.locator('input[type="checkbox"]').nth(1);
  if (await pointerLockCheckbox.count()) {
    try {
      await pointerLockCheckbox.uncheck();
      summary.push("unchecked lock/hide mouse pointer");
    } catch {
    }
  }

  for (const step of scenario.steps || []) {
    const keepRunning = await runStep(page, step, summary, screenshotDir);
    if (!keepRunning) {
      break;
    }
  }

  if (scenario.expectTitle) {
    const title = await page.title();
    if (title !== scenario.expectTitle) {
      throw new Error(`Titulo final inesperado: '${title}', esperado '${scenario.expectTitle}'`);
    }
  }

  const result = {
    mode,
    scenario: path.relative(rootDir, scenarioPath),
    title: await page.title(),
    summary,
    consoleMessages
  };

  console.log(JSON.stringify(result, null, 2));

  await browser.close();
  await new Promise((resolve) => server.close(resolve));
}

main().catch(async (error) => {
  console.error(error);
  await new Promise((resolve) => server.close(resolve));
  process.exit(1);
});
