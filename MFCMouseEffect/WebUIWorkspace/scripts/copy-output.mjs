import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const __dirname = path.dirname(fileURLToPath(import.meta.url));
const workspaceDir = path.resolve(__dirname, '..');
const projectDir = path.resolve(workspaceDir, '..');
const repoRoot = path.resolve(projectDir, '..');
const webUiDir = path.join(projectDir, 'WebUI');

const generatedFiles = [
  'dialog.svelte.js',
  'settings-shell.svelte.js',
  'section-workspace.svelte.js',
  'general-settings.svelte.js',
  'effects-settings.svelte.js',
  'text-settings.svelte.js',
  'trail-settings.svelte.js',
  'input-indicator-settings.svelte.js',
  'automation-ui.svelte.js',
];

const staticWebUiFiles = [
  'index.html',
  'app.js',
  'styles.css',
  'web-api.js',
  'settings-form.js',
  'i18n.js',
  'i18n-runtime.js',
  'automation-templates.js',
];

function copyOrThrow(source, target) {
  if (!fs.existsSync(source)) {
    throw new Error(`Build output not found: ${source}`);
  }
  fs.copyFileSync(source, target);
}

function wrapBundleScope(content) {
  const marker = '/* mfx-scope-wrapped */';
  if (content.includes(marker)) return content;
  return [
    '/* mfx-scope-wrapped */',
    '(() => {',
    content,
    '})();',
    '',
  ].join('\n');
}

function copyGeneratedBundleOrThrow(source, target) {
  if (!fs.existsSync(source)) {
    throw new Error(`Build output not found: ${source}`);
  }
  const original = fs.readFileSync(source, 'utf8');
  const wrapped = wrapBundleScope(original);
  fs.writeFileSync(target, wrapped, 'utf8');
}

for (const fileName of generatedFiles) {
  const source = path.join(workspaceDir, 'dist', fileName);
  const target = path.join(webUiDir, fileName);
  copyGeneratedBundleOrThrow(source, target);
}

const runtimeWebUiDirs = [
  path.join(repoRoot, 'x64', 'Debug', 'webui'),
  path.join(repoRoot, 'x64', 'Release', 'webui'),
];

for (const runtimeDir of runtimeWebUiDirs) {
  if (!fs.existsSync(runtimeDir)) {
    continue;
  }

  for (const fileName of generatedFiles) {
    const source = path.join(webUiDir, fileName);
    const target = path.join(runtimeDir, fileName);
    copyOrThrow(source, target);
  }

  for (const fileName of staticWebUiFiles) {
    const source = path.join(webUiDir, fileName);
    const target = path.join(runtimeDir, fileName);
    copyOrThrow(source, target);
  }
}
