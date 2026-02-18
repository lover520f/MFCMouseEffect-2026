import { resolve } from "node:path";
import {
  compileAssemblyScript,
  loadTemplateManifest,
  resolveTemplateRoot,
  writeManifest,
} from "./build-lib.mjs";

const rootDir = resolveTemplateRoot(import.meta.url);
const distDir = resolve(rootDir, "dist");

try {
  compileAssemblyScript({
    rootDir,
    entryRelativePath: "assembly/index.ts",
    wasmOutputPath: resolve(distDir, "effect.wasm"),
    watOutputPath: resolve(distDir, "effect.wat"),
  });
  writeManifest(resolve(distDir, "plugin.json"), loadTemplateManifest(rootDir));
  console.log("Template build complete:", resolve(distDir, "effect.wasm"));
} catch (error) {
  console.error(error?.message || error);
  process.exit(1);
}
