import { resolve } from "node:path";
import {
  compileAssemblyScript,
  copyRelativeFiles,
  resetOutputDir,
  resolveTemplateRoot,
  writeManifest,
} from "./build-lib.mjs";
import { SAMPLE_PRESETS } from "./sample-presets.mjs";

const rootDir = resolveTemplateRoot(import.meta.url);
const samplesDistDir = resolve(rootDir, "dist", "samples");

try {
  resetOutputDir(samplesDistDir);
  for (const preset of SAMPLE_PRESETS) {
    const outputDir = resolve(rootDir, "dist", "samples", preset.key);
    const wasmPath = resolve(outputDir, "effect.wasm");
    const watPath = resolve(outputDir, "effect.wat");
    const manifestPath = resolve(outputDir, "plugin.json");

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
      api_version: 2,
      entry: "effect.wasm",
      image_assets: Array.isArray(preset.imageAssets) ? preset.imageAssets : [],
      surfaces: Array.isArray(preset.surfaces) ? preset.surfaces : ["effects"],
      input_kinds: Array.isArray(preset.inputKinds) ? preset.inputKinds : undefined,
      enable_frame_tick: typeof preset.enableFrameTick === "boolean" ? preset.enableFrameTick : undefined,
    });
    copyRelativeFiles(rootDir, outputDir, preset.imageAssets);
    console.log(`Built sample: ${preset.key}`);
  }
  console.log("All sample bundles are ready under dist/samples.");
} catch (error) {
  console.error(error?.message || error);
  process.exit(1);
}
