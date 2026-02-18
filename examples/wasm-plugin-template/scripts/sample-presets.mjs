export const SAMPLE_PRESETS = [
  {
    key: "text-rise",
    source: "assembly/samples/text-rise.ts",
    id: "demo.click.text-rise.v1",
    name: "Demo Click Text Rise",
    version: "0.1.0",
  },
  {
    key: "text-burst",
    source: "assembly/samples/text-burst.ts",
    id: "demo.click.text-burst.v1",
    name: "Demo Click Text Burst",
    version: "0.1.0",
  },
  {
    key: "image-pulse",
    source: "assembly/samples/image-pulse.ts",
    id: "demo.click.image-pulse.v1",
    name: "Demo Click Image Pulse",
    version: "0.1.0",
  },
  {
    key: "mixed-text-image",
    source: "assembly/samples/mixed-text-image.ts",
    id: "demo.click.mixed-text-image.v1",
    name: "Demo Click Mixed Text Image",
    version: "0.1.0",
  },
];

export function findSamplePreset(key) {
  const value = `${key || ""}`.trim().toLowerCase();
  return SAMPLE_PRESETS.find((sample) => sample.key.toLowerCase() === value) || null;
}

export function sampleKeysText() {
  return SAMPLE_PRESETS.map((sample) => sample.key).join(", ");
}
