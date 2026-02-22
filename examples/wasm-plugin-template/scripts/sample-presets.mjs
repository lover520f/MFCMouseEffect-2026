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
    key: "text-spiral",
    source: "assembly/samples/text-spiral.ts",
    id: "demo.click.text-spiral.v1",
    name: "Demo Click Text Spiral",
    version: "0.1.0",
  },
  {
    key: "text-wave-chain",
    source: "assembly/samples/text-wave-chain.ts",
    id: "demo.click.text-wave-chain.v1",
    name: "Demo Click Text Wave Chain",
    version: "0.1.0",
  },
  {
    key: "image-pulse",
    source: "assembly/samples/image-pulse.ts",
    id: "demo.click.image-pulse.v1",
    name: "Demo Click Image Pulse",
    version: "0.1.0",
    imageAssets: [
      "assets/smile.png",
      "assets/coin.jpg",
      "assets/cat.gif",
      "assets/star.bmp",
    ],
  },
  {
    key: "image-burst",
    source: "assembly/samples/image-burst.ts",
    id: "demo.click.image-burst.v1",
    name: "Demo Click Image Burst",
    version: "0.1.0",
    imageAssets: [
      "assets/emoji-1.jpeg",
      "assets/emoji-2.png",
      "assets/emoji-3.gif",
    ],
  },
  {
    key: "image-lift",
    source: "assembly/samples/image-lift.ts",
    id: "demo.click.image-lift.v1",
    name: "Demo Click Image Lift",
    version: "0.1.0",
    imageAssets: [
      "assets/lift-a.tif",
      "assets/lift-b.tiff",
    ],
  },
  {
    key: "mixed-text-image",
    source: "assembly/samples/mixed-text-image.ts",
    id: "demo.click.mixed-text-image.v1",
    name: "Demo Click Mixed Text Image",
    version: "0.1.0",
    imageAssets: [
      "assets/mix-a.png",
      "assets/mix-b.jpg",
    ],
  },
  {
    key: "mixed-emoji-celebrate",
    source: "assembly/samples/mixed-emoji-celebrate.ts",
    id: "demo.click.mixed-emoji-celebrate.v1",
    name: "Demo Click Mixed Emoji Celebrate",
    version: "0.1.0",
    imageAssets: [
      "assets/party.gif",
      "assets/confetti.png",
      "assets/crown.png",
    ],
  },
  {
    key: "button-adaptive",
    source: "assembly/samples/button-adaptive.ts",
    id: "demo.click.button-adaptive.v1",
    name: "Demo Click Button Adaptive",
    version: "0.1.0",
    imageAssets: [
      "assets/btn-left.png",
      "assets/btn-right.gif",
      "assets/btn-middle.tiff",
    ],
  },
  {
    key: "scroll-particle-burst",
    source: "assembly/samples/scroll-particle-burst.ts",
    id: "demo.event.scroll-particle-burst.v1",
    name: "Demo Event Scroll Particle Burst",
    version: "0.1.0",
    imageAssets: [
      "assets/smile.png",
      "assets/confetti.png",
      "assets/party.gif",
      "assets/emoji-2.png",
    ],
  },
];

export function findSamplePreset(key) {
  const value = `${key || ""}`.trim().toLowerCase();
  return SAMPLE_PRESETS.find((sample) => sample.key.toLowerCase() === value) || null;
}

export function sampleKeysText() {
  return SAMPLE_PRESETS.map((sample) => sample.key).join(", ");
}
