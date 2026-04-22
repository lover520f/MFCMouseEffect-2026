import path from 'node:path';
import fs from 'node:fs';
import { defineConfig } from 'vite';
import { svelte } from '@sveltejs/vite-plugin-svelte';

const ENTRY_ROOT = 'src/entries';
const DEV_RUNTIME_ROUTE = '/__mfx/dev-runtime';
const DEFAULT_DEV_PROBE_FILE = '/tmp/mfx-core-websettings.probe';

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
  'mouse-companion': {
    entry: 'mouse-companion-main.js',
    name: 'MfxMouseCompanionSettingsBundle',
    fileName: 'mouse-companion-settings.svelte.js',
  },
  automation: {
    entry: 'automation-main.js',
    name: 'MfxAutomationUiBundle',
    fileName: 'automation-ui.svelte.js',
  },
  wasm: {
    entry: 'wasm-main.js',
    name: 'MfxWasmSectionBundle',
    fileName: 'wasm-settings.svelte.js',
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

function trimText(value) {
  return `${value || ''}`.trim();
}

function safeOrigin(rawUrl) {
  try {
    return trimText(new URL(rawUrl).origin);
  } catch (_error) {
    return '';
  }
}

function safeTokenFromUrl(rawUrl) {
  try {
    return trimText(new URL(rawUrl).searchParams.get('token') || '');
  } catch (_error) {
    return '';
  }
}

function readProbeRuntime(probeFile) {
  const runtime = {
    probeFile,
    url: '',
    baseUrl: '',
    token: '',
  };
  if (!probeFile || !fs.existsSync(probeFile)) {
    return runtime;
  }

  const lines = fs.readFileSync(probeFile, 'utf8').split(/\r?\n/);
  for (const line of lines) {
    const index = line.indexOf('=');
    if (index <= 0) {
      continue;
    }
    const key = trimText(line.slice(0, index));
    const value = trimText(line.slice(index + 1));
    if (!key || !value) {
      continue;
    }
    if (key === 'url') {
      runtime.url = value;
      continue;
    }
    if (key === 'token') {
      runtime.token = value;
      continue;
    }
    if (key === 'base_url' || key === 'baseUrl') {
      runtime.baseUrl = value;
    }
  }

  if (!runtime.baseUrl && runtime.url) {
    runtime.baseUrl = safeOrigin(runtime.url);
  }
  if (!runtime.token && runtime.url) {
    runtime.token = safeTokenFromUrl(runtime.url);
  }
  return runtime;
}

function resolveDevRuntime() {
  const probeFile = trimText(process.env.MFX_WEBUI_DEV_PROBE_FILE || DEFAULT_DEV_PROBE_FILE);
  const probeRuntime = readProbeRuntime(probeFile);
  const baseUrl = trimText(process.env.MFX_WEBUI_DEV_BASE_URL || probeRuntime.baseUrl);
  const token = trimText(process.env.MFX_WEBUI_DEV_TOKEN || probeRuntime.token);
  const settingsUrl = trimText(probeRuntime.url || (baseUrl ? `${baseUrl}/` : ''));

  return {
    available: !!baseUrl,
    baseUrl,
    token,
    settingsUrl,
    probeFile,
  };
}

function createDevRuntimePlugin() {
  return {
    name: 'mfx-dev-runtime',
    configureServer(server) {
      server.middlewares.use(async (req, res, next) => {
        const requestUrl = trimText(req.originalUrl || req.url || '');
        if (!requestUrl) {
          next();
          return;
        }

        if (requestUrl === DEV_RUNTIME_ROUTE) {
          const runtime = resolveDevRuntime();
          res.statusCode = 200;
          res.setHeader('Content-Type', 'application/json; charset=utf-8');
          res.end(JSON.stringify(runtime));
          return;
        }

        if (!requestUrl.startsWith('/api/')) {
          next();
          return;
        }

        const runtime = resolveDevRuntime();
        if (!runtime.available) {
          res.statusCode = 502;
          res.setHeader('Content-Type', 'application/json; charset=utf-8');
          res.end(JSON.stringify({
            ok: false,
            error: 'mfx-dev-runtime unavailable',
            probeFile: runtime.probeFile,
          }));
          return;
        }

        try {
          const target = new URL(requestUrl, runtime.baseUrl);
          const headers = new Headers();
          for (const [key, value] of Object.entries(req.headers)) {
            if (value == null) {
              continue;
            }
            if (key === 'host' || key === 'connection' || key === 'content-length') {
              continue;
            }
            if (Array.isArray(value)) {
              for (const item of value) {
                headers.append(key, item);
              }
              continue;
            }
            headers.set(key, value);
          }

          const response = await fetch(target, {
            method: req.method || 'GET',
            headers,
            body: req.method === 'GET' || req.method === 'HEAD' ? undefined : req,
            duplex: req.method === 'GET' || req.method === 'HEAD' ? undefined : 'half',
          });

          res.statusCode = response.status;
          for (const [key, value] of response.headers.entries()) {
            if (key === 'content-length' || key === 'transfer-encoding' || key === 'connection') {
              continue;
            }
            res.setHeader(key, value);
          }
          const body = Buffer.from(await response.arrayBuffer());
          res.end(body);
        } catch (error) {
          res.statusCode = 502;
          res.setHeader('Content-Type', 'application/json; charset=utf-8');
          res.end(JSON.stringify({
            ok: false,
            error: error instanceof Error ? error.message : String(error),
            baseUrl: runtime.baseUrl,
          }));
        }
      });
    },
  };
}

export default defineConfig(({ mode }) => {
  const target = pickBuildTarget(mode);
  return {
    server: {
      fs: {
        allow: [
          path.resolve(__dirname),
          path.resolve(__dirname, '../WebUI'),
        ],
      },
    },
    esbuild: {
      legalComments: 'none',
    },
    plugins: [
      svelte({
        compilerOptions: {
          compatibility: {
            componentApi: 4,
          },
        },
      }),
      createDevRuntimePlugin(),
    ],
    build: {
      emptyOutDir: false,
      outDir: path.resolve(__dirname, 'dist'),
      sourcemap: false,
      minify: 'esbuild',
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
