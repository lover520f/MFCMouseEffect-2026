import {
  listWasmActionErrorI18nKeys,
  normalizeActionErrorCode,
  resolveWasmActionErrorMessage,
} from '../src/wasm/action-error-model.js';
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

function assert(condition, message) {
  if (!condition) {
    throw new Error(message);
  }
}

function escapeRegex(value) {
  return `${value || ''}`.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
}

function extractObjectBody(source, marker) {
  const markerIndex = source.indexOf(marker);
  assert(markerIndex >= 0, `i18n marker not found: ${marker}`);

  const braceStart = source.indexOf('{', markerIndex);
  assert(braceStart >= 0, `i18n object start not found: ${marker}`);

  let depth = 0;
  for (let i = braceStart; i < source.length; i += 1) {
    const ch = source[i];
    if (ch === '{') {
      depth += 1;
      continue;
    }
    if (ch !== '}') {
      continue;
    }
    depth -= 1;
    if (depth === 0) {
      return source.slice(braceStart + 1, i);
    }
  }
  throw new Error(`i18n object end not found: ${marker}`);
}

function resolveWebUiI18nPath() {
  const scriptDir = path.dirname(fileURLToPath(import.meta.url));
  const workspaceDir = path.resolve(scriptDir, '..');
  const projectDir = path.resolve(workspaceDir, '..');
  return path.join(projectDir, 'WebUI', 'i18n.js');
}

function testNormalizeActionErrorCode() {
  assert(normalizeActionErrorCode(' MANIFEST_PATH_NOT_FOUND ') === 'manifest_path_not_found',
    'normalizeActionErrorCode should trim and lowercase input');
  assert(normalizeActionErrorCode('') === '', 'normalizeActionErrorCode should keep empty input');
  assert(normalizeActionErrorCode(null) === '', 'normalizeActionErrorCode should handle null');
}

function testResolveKnownErrorMessageWithFallback() {
  const message = resolveWasmActionErrorMessage('manifest_path_not_found');
  assert(message.includes('does not exist'), 'known error code should map to fallback message');
}

function testResolveRuntimeReloadErrorMessages() {
  const reloadMissingTarget = resolveWasmActionErrorMessage('reload_target_missing');
  assert(reloadMissingTarget.includes('No active WASM plugin target'),
    'reload_target_missing should map to runtime reload fallback message');

  const reloadModuleMissing = resolveWasmActionErrorMessage('module_load_failed');
  assert(reloadModuleMissing.includes('Failed to load WASM module'),
    'module_load_failed should map to runtime reload fallback message');

  const reloadManifestApi = resolveWasmActionErrorMessage('manifest_api_unsupported');
  assert(reloadManifestApi.includes('unsupported'),
    'manifest_api_unsupported should map to runtime reload fallback message');
}

function testResolveKnownErrorMessageWithTranslate() {
  const translated = resolveWasmActionErrorMessage('reload_target_missing', (key, fallback) => {
    if (key === 'wasm_error_reload_target_missing') {
      return 'translated: reload target missing';
    }
    return fallback;
  });
  assert(translated === 'translated: reload target missing',
    'known runtime error code should use translation callback');
}

function testResolveUnknownErrorMessage() {
  const unknown = resolveWasmActionErrorMessage('unknown_error_code');
  assert(unknown === '', 'unknown error code should return empty message');
}

function testI18nKeyParity() {
  const i18nPath = resolveWebUiI18nPath();
  const source = fs.readFileSync(i18nPath, 'utf8');

  const enSection = extractObjectBody(source, '"en-US": {');
  const zhSection = extractObjectBody(source, '"zh-CN": {');

  const i18nKeys = listWasmActionErrorI18nKeys();
  const uniqueKeyCount = new Set(i18nKeys).size;
  assert(uniqueKeyCount === i18nKeys.length, 'error model i18n keys must be unique');

  for (const key of i18nKeys) {
    const keyPattern = new RegExp(`\\b${escapeRegex(key)}\\s*:`);
    assert(keyPattern.test(enSection), `missing i18n key in en-US: ${key}`);
    assert(keyPattern.test(zhSection), `missing i18n key in zh-CN: ${key}`);
  }
}

function main() {
  testNormalizeActionErrorCode();
  testResolveKnownErrorMessageWithFallback();
  testResolveRuntimeReloadErrorMessages();
  testResolveKnownErrorMessageWithTranslate();
  testResolveUnknownErrorMessage();
  testI18nKeyParity();
  console.log('[result] wasm action error model tests passed');
}

main();
