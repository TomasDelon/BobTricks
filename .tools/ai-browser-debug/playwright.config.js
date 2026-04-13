const { defineConfig } = require("@playwright/test");

module.exports = defineConfig({
  testDir: "./tests/web",
  timeout: 30_000,
  use: {
    baseURL: "http://127.0.0.1:8080"
  },
  webServer: {
    command: "python3 -m http.server 8080 --directory ../../build/web",
    url: "http://127.0.0.1:8080/bobtricks.html",
    reuseExistingServer: true,
    timeout: 30_000
  }
});
