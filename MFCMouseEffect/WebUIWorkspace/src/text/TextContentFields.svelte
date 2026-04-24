<script>
  import { createEventDispatcher } from 'svelte';

  export let text = {};
  export let onChangeState = null;

  const dispatch = createEventDispatcher();

  function toNumber(value, fallback) {
    const parsed = Number(value);
    if (Number.isFinite(parsed)) return parsed;
    return fallback;
  }

  function normalizeText(input) {
    const value = input || {};
    return {
      text_content: value.text_content || '',
      text_font_size: toNumber(value.text_font_size, 0),
    };
  }

  function toSnapshot(form) {
    const value = form || normalizeText({});
    return {
      text_content: value.text_content || '',
      text_font_size: toNumber(value.text_font_size, 0),
    };
  }

  let form = normalizeText(text);
  let lastTextRef = text;

  $: if (text !== lastTextRef) {
    lastTextRef = text;
    form = normalizeText(text);
  }

  $: {
    const snapshot = toSnapshot(form);
    if (typeof onChangeState === 'function') {
      onChangeState(snapshot);
    }
    dispatch('change', snapshot);
  }
</script>

<div class="grid">
  <label for="text_font_size" data-i18n="label_text_font_size">Text font size (pt)</label>
  <input id="text_font_size" type="number" min="6" max="96" step="0.5" bind:value={form.text_font_size} />

  <label for="text_content" data-i18n="label_texts">Comma separated</label>
  <textarea
    id="text_content"
    data-i18n-placeholder="placeholder_texts"
    placeholder="happy,healthy"
    bind:value={form.text_content}
  ></textarea>

  <div class="hint span2" data-i18n="hint_texts">Use English comma "," to separate words.</div>
</div>
