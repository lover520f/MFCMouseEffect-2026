import path from 'node:path';
import { defineConfig } from 'vite';
import { svelte } from '@sveltejs/vite-plugin-svelte';

const ENTRY_ROOT = 'src/entries';

const TARGETS = {
  indicator: {
    entry: 'input-indicator-main.js',
    name: 'MfxInputIndicatorSettingsBundle',
    fileName: 'input-indicator-settings.svelte.js',
  },
  trail: {
    entry: 'trail-main.js',
    name: 'MfxTrailSettingsBundle',
    fileName: 'trail-settings.svelte.js',
  },
  text: {
    entry: 'text-main.js',
    name: 'MfxTextSettingsBundle',
    fileName: 'text-settings.svelte.js',
  },
  effects: {
    entry: 'effects-main.js',
    name: 'MfxEffectsSettingsBundle',
    fileName: 'effects-settings.svelte.js',
  },
  general: {
    entry: 'general-main.js',
    name: 'MfxGeneralSettingsBundle',
    fileName: 'general-settings.svelte.js',
  },
  automation: {
    entry: 'automation-main.js',
    name: 'MfxAutomationUiBundle',
    fileName: 'automation-ui.svelte.js',
  },
  dialog: {
    entry: 'dialog-main.js',
    name: 'MfxDialogBundle',
    fileName: 'dialog.svelte.js',
  },
  shell: {
    entry: 'shell-main.js',
    name: 'MfxSettingsShellBundle',
    fileName: 'settings-shell.svelte.js',
  },
  workspace: {
    entry: 'main.js',
    name: 'MfxSectionWorkspaceBundle',
    fileName: 'section-workspace.svelte.js',
  },
};

function pickBuildTarget(mode) {
  const target = TARGETS[mode] || TARGETS.workspace;
  return {
    entry: path.resolve(__dirname, ENTRY_ROOT, target.entry),
    name: target.name,
    fileName: target.fileName,
  };
}

export default defineConfig(({ mode }) => {
  const target = pickBuildTarget(mode);
  return {
    plugins: [svelte({
      compilerOptions: {
        compatibility: {
          componentApi: 4,
        },
      },
    })],
    build: {
      emptyOutDir: false,
      outDir: path.resolve(__dirname, 'dist'),
      sourcemap: false,
      minify: false,
      lib: {
        entry: target.entry,
        name: target.name,
        formats: ['iife'],
        fileName: () => target.fileName,
      },
      rollupOptions: {
        output: {
          extend: true,
        },
      },
    },
  };
});
