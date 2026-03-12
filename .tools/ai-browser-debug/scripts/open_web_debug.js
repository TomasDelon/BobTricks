const { chromium } = require("@playwright/test");
const http = require("http");
const fs = require("fs");
const path = require("path");

const rootDir = path.resolve(__dirname, "..", "..", "..");
const webDir = path.join(rootDir, "build", "web");
const host = "127.0.0.1";
const port = 8080;

if (!fs.existsSync(path.join(webDir, "bobtricks.html"))) {
  console.error("No existe build/web/bobtricks.html. Ejecuta scripts/build/build_web.sh primero.");
  process.exit(1);
}

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
  const cleanPath = decodeURIComponent(urlPath.split("?")[0]);
  const requested = cleanPath === "/" ? "/bobtricks.html" : cleanPath;
  const resolved = path.resolve(webDir, `.${requested}`);
  if (!resolved.startsWith(webDir)) {
    return null;
  }
  return resolved;
}

const server = http.createServer((req, res) => {
  const filePath = safePathFromUrl(req.url || "/");
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

    const ext = path.extname(filePath);
    res.writeHead(200, {
      "Content-Type": contentTypes[ext] || "application/octet-stream",
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
    headless: false,
    devtools: true
  });

  const page = await browser.newPage({
    viewport: { width: 1440, height: 900 }
  });

  await page.goto(`http://${host}:${port}/bobtricks.html`);

  console.log(`App abierta en http://${host}:${port}/bobtricks.html`);
  console.log("La ventana queda abierta. Cierra Chromium o pulsa Ctrl+C para terminar.");

  const shutdown = async () => {
    await browser.close();
    await new Promise((resolve) => server.close(resolve));
    process.exit(0);
  };

  process.on("SIGINT", shutdown);
  process.on("SIGTERM", shutdown);
}

main().catch((error) => {
  console.error(error);
  server.close();
  process.exit(1);
});
