import { resolve } from "node:path";
import {
  compileAssemblyScript,
  copyRelativeFiles,
  resolveTemplateRoot,
  writeManifest,
} from "./build-lib.mjs";
import { SAMPLE_PRESETS } from "./sample-presets.mjs";

const rootDir = resolveTemplateRoot(import.meta.url);

try {
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
      api_version: 1,
      entry: "effect.wasm",
      image_assets: Array.isArray(preset.imageAssets) ? preset.imageAssets : [],
    });
    copyRelativeFiles(rootDir, outputDir, preset.imageAssets);
    console.log(`Built sample: ${preset.key}`);
  }
  console.log("All sample bundles are ready under dist/samples.");
} catch (error) {
  console.error(error?.message || error);
  process.exit(1);
}
