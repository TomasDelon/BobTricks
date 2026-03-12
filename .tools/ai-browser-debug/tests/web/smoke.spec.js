const { test, expect } = require("@playwright/test");

test("web build loads", async ({ page }) => {
  await page.goto("/bobtricks.html");

  await expect(page).toHaveTitle(/BobTricks/);
  await expect(page.locator("canvas")).toHaveCount(1);
});
