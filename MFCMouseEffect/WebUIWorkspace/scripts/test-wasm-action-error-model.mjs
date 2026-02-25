import {
  normalizeActionErrorCode,
  resolveWasmActionErrorMessage,
} from '../src/wasm/action-error-model.js';

function assert(condition, message) {
  if (!condition) {
    throw new Error(message);
  }
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

function testResolveKnownErrorMessageWithTranslate() {
  const translated = resolveWasmActionErrorMessage('manifest_path_not_found', (key, fallback) => {
    if (key === 'wasm_error_manifest_path_not_found') {
      return 'translated: not found';
    }
    return fallback;
  });
  assert(translated === 'translated: not found', 'known error code should use translation callback');
}

function testResolveUnknownErrorMessage() {
  const unknown = resolveWasmActionErrorMessage('unknown_error_code');
  assert(unknown === '', 'unknown error code should return empty message');
}

function main() {
  testNormalizeActionErrorCode();
  testResolveKnownErrorMessageWithFallback();
  testResolveKnownErrorMessageWithTranslate();
  testResolveUnknownErrorMessage();
  console.log('[result] wasm action error model tests passed');
}

main();
