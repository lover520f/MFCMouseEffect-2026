import { resolve } from "node:path";
import {
  compileAssemblyScript,
  parseArgs,
  resolveTemplateRoot,
  writeManifest,
} from "./build-lib.mjs";
import { findSamplePreset, sampleKeysText } from "./sample-presets.mjs";

const rootDir = resolveTemplateRoot(import.meta.url);
const args = parseArgs(process.argv.slice(2));
const sampleKey = `${args.sample || ""}`.trim();
const preset = findSamplePreset(sampleKey);
if (!preset) {
  console.error(`Unknown sample "${sampleKey}". Available: ${sampleKeysText()}`);
  process.exit(1);
}

const outputDir = resolve(rootDir, "dist", "samples", preset.key);
const wasmPath = resolve(outputDir, "effect.wasm");
const watPath = resolve(outputDir, "effect.wat");
const manifestPath = resolve(outputDir, "plugin.json");

try {
  compileAssemblyScript({
    rootDir,
    entryRelativePath: preset.source,
    wasmOutputPath: wasmPath,
    watOutputPath: watPath,
  });
  writeManifest(manifestPath, {
    id: preset.id,
    name: preset.name,
    version: preset.version,
    api_version: 1,
    entry: "effect.wasm",
  });
  console.log(`Sample build complete: ${preset.key}`);
  console.log(`  wasm: ${wasmPath}`);
  console.log(`  manifest: ${manifestPath}`);
} catch (error) {
  console.error(error?.message || error);
  process.exit(1);
}
