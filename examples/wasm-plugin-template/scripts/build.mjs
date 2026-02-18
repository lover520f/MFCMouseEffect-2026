import { spawnSync } from "node:child_process";
import { copyFileSync, mkdirSync } from "node:fs";
import { dirname, resolve } from "node:path";
import { fileURLToPath } from "node:url";

const scriptDir = dirname(fileURLToPath(import.meta.url));
const rootDir = resolve(scriptDir, "..");
const distDir = resolve(rootDir, "dist");

mkdirSync(distDir, { recursive: true });

const ascPath = resolve(rootDir, "node_modules", "assemblyscript", "bin", "asc");
const args = ["assembly/index.ts", "--config", "asconfig.json", "--target", "release"];
const build = spawnSync(process.execPath, [ascPath, ...args], {
  cwd: rootDir,
  stdio: "inherit",
});
if (build.status !== 0) {
  process.exit(build.status ?? 1);
}

copyFileSync(resolve(rootDir, "plugin.json"), resolve(distDir, "plugin.json"));
console.log("Template build complete:", resolve(distDir, "effect.wasm"));
