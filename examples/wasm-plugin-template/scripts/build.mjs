import { resolve } from "node:path";
import {
  compileAssemblyScript,
  copyRelativeFiles,
  loadTemplateManifest,
  resolveTemplateRoot,
  writeManifest,
} from "./build-lib.mjs";

const rootDir = resolveTemplateRoot(import.meta.url);
const distDir = resolve(rootDir, "dist");

try {
  const manifest = loadTemplateManifest(rootDir);
  compileAssemblyScript({
    rootDir,
    entryRelativePath: "assembly/index.ts",
    wasmOutputPath: resolve(distDir, "effect.wasm"),
    watOutputPath: resolve(distDir, "effect.wat"),
  });
  const distManifestPath = resolve(distDir, "plugin.json");
  writeManifest(distManifestPath, manifest);
  copyRelativeFiles(rootDir, distDir, manifest.image_assets);
  console.log("Template build complete:", resolve(distDir, "effect.wasm"));
} catch (error) {
  console.error(error?.message || error);
  process.exit(1);
}
