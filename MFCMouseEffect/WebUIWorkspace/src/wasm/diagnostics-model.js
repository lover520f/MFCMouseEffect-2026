function normalizeInteger(value) {
  const parsed = Number(value);
  if (!Number.isFinite(parsed)) {
    return 0;
  }
  return Math.max(0, Math.round(parsed));
}

function normalizeBoolean(value) {
  return !!value;
}

function normalizeText(value) {
  return `${value || ''}`.trim();
}

export function normalizeWasmDiagnostics(input) {
  const value = input || {};
  return {
    last_call_duration_us: normalizeInteger(value.last_call_duration_us),
    last_output_bytes: normalizeInteger(value.last_output_bytes),
    last_command_count: normalizeInteger(value.last_command_count),
    last_call_exceeded_budget: normalizeBoolean(value.last_call_exceeded_budget),
    last_call_rejected_by_budget: normalizeBoolean(value.last_call_rejected_by_budget),
    last_output_truncated_by_budget: normalizeBoolean(value.last_output_truncated_by_budget),
    last_command_truncated_by_budget: normalizeBoolean(value.last_command_truncated_by_budget),
    last_budget_reason: normalizeText(value.last_budget_reason),
    last_parse_error: normalizeText(value.last_parse_error),
  };
}

export function hasWasmDiagnosticWarning(diagnostics) {
  const value = diagnostics || {};
  if (value.last_call_exceeded_budget) return true;
  if (value.last_call_rejected_by_budget) return true;
  if (value.last_output_truncated_by_budget) return true;
  if (value.last_command_truncated_by_budget) return true;
  const parseError = normalizeText(value.last_parse_error).toLowerCase();
  return parseError.length > 0 && parseError !== 'none';
}

export function buildWasmCallMetricsText(diagnostics, translate) {
  const value = diagnostics || {};
  const t = typeof translate === 'function' ? translate : (_key, fallback) => fallback;
  return `${t('label_wasm_metric_duration', 'Duration')}=${value.last_call_duration_us}us, `
    + `${t('label_wasm_metric_output_bytes', 'Output')}=${value.last_output_bytes}B, `
    + `${t('label_wasm_metric_command_count', 'Commands')}=${value.last_command_count}`;
}

export function buildWasmBudgetFlagsText(diagnostics, translate) {
  const value = diagnostics || {};
  const t = typeof translate === 'function' ? translate : (_key, fallback) => fallback;
  const yes = t('wasm_text_yes', 'Yes');
  const no = t('wasm_text_no', 'No');
  const boolText = (flag) => (flag ? yes : no);
  return `${t('label_wasm_budget_flag_exceeded', 'Exceeded')}=${boolText(value.last_call_exceeded_budget)}, `
    + `${t('label_wasm_budget_flag_rejected', 'Rejected')}=${boolText(value.last_call_rejected_by_budget)}, `
    + `${t('label_wasm_budget_flag_output_truncated', 'OutputTrunc')}=${boolText(value.last_output_truncated_by_budget)}, `
    + `${t('label_wasm_budget_flag_command_truncated', 'CommandTrunc')}=${boolText(value.last_command_truncated_by_budget)}`;
}
