import { spawnSync } from "node:child_process";
import { copyFileSync, existsSync, mkdirSync, readFileSync, writeFileSync } from "node:fs";
import { dirname, relative, resolve } from "node:path";
import { fileURLToPath } from "node:url";

export function resolveTemplateRoot(importMetaUrl) {
  const scriptDir = dirname(fileURLToPath(importMetaUrl));
  return resolve(scriptDir, "..");
}

export function parseArgs(argv) {
  const out = {};
  for (let i = 0; i < argv.length; i += 1) {
    const token = `${argv[i] || ""}`;
    if (!token.startsWith("--")) {
      continue;
    }
    const key = token.slice(2);
    const next = `${argv[i + 1] || ""}`;
    if (!next || next.startsWith("--")) {
      out[key] = "true";
      continue;
    }
    out[key] = next;
    i += 1;
  }
  return out;
}

function toAscPath(rootDir) {
  const candidates = [
    resolve(rootDir, "node_modules", "assemblyscript", "bin", "asc.js"),
    resolve(rootDir, "node_modules", "assemblyscript", "bin", "asc"),
  ];
  for (const candidate of candidates) {
    if (existsSync(candidate)) {
      return candidate;
    }
  }
  return "";
}

function toPosixRelative(rootDir, targetPath) {
  return relative(rootDir, targetPath).replace(/\\/g, "/");
}

export function ensureDistDir(dirPath) {
  mkdirSync(dirPath, { recursive: true });
}

export function compileAssemblyScript({
  rootDir,
  entryRelativePath,
  wasmOutputPath,
  watOutputPath,
}) {
  const ascPath = toAscPath(rootDir);
  if (!ascPath || !existsSync(ascPath)) {
    throw new Error("AssemblyScript compiler not found. Run npm/pnpm install first.");
  }

  ensureDistDir(dirname(wasmOutputPath));
  if (watOutputPath) {
    ensureDistDir(dirname(watOutputPath));
  }

  const args = [
    toPosixRelative(rootDir, resolve(rootDir, entryRelativePath)),
    "--config",
    "asconfig.json",
    "--target",
    "release",
    "--outFile",
    toPosixRelative(rootDir, wasmOutputPath),
  ];
  if (watOutputPath) {
    args.push("--textFile", toPosixRelative(rootDir, watOutputPath));
  }

  const build = spawnSync(process.execPath, [ascPath, ...args], {
    cwd: rootDir,
    stdio: "inherit",
  });
  if (build.status !== 0) {
    throw new Error(`AssemblyScript build failed (${entryRelativePath}).`);
  }
}

export function loadTemplateManifest(rootDir) {
  const manifestPath = resolve(rootDir, "plugin.json");
  const body = readFileSync(manifestPath, "utf8");
  return JSON.parse(body);
}

export function writeManifest(manifestPath, manifest) {
  ensureDistDir(dirname(manifestPath));
  writeFileSync(manifestPath, `${JSON.stringify(manifest, null, 2)}\n`, "utf8");
}

export function copyRelativeFiles(rootDir, outputDir, relativeFiles) {
  if (!Array.isArray(relativeFiles) || relativeFiles.length <= 0) {
    return;
  }
  for (const file of relativeFiles) {
    const relPath = `${file || ""}`.trim();
    if (!relPath) {
      continue;
    }
    const sourcePath = resolve(rootDir, relPath);
    if (!existsSync(sourcePath)) {
      throw new Error(`Missing asset file: ${relPath}`);
    }
    const targetPath = resolve(outputDir, relPath);
    ensureDistDir(dirname(targetPath));
    copyFileSync(sourcePath, targetPath);
  }
}
