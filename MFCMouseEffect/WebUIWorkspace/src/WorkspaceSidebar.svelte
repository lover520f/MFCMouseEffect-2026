<script>
  import { createEventDispatcher } from 'svelte';

  export let mode = 'focus';
  export let sections = [];
  export let summary = { title: '', description: '' };
  export let texts = {
    btn_view_focus: 'Focused View',
    btn_view_all: 'All Sections',
    hint_view_focus: 'Focused view shows one section at a time to reduce noise.',
    workspace_current_label: 'Current Section',
    view_mode_aria: 'View mode',
    section_nav_aria: 'Settings sections',
  };

  const dispatch = createEventDispatcher();

  function selectSection(id) {
    dispatch('select', { id });
  }

  function selectMode(next) {
    dispatch('mode', { mode: next });
  }
</script>

<div class="workspace-mode" role="group" aria-label={texts.view_mode_aria}>
  <button
    type="button"
    class="btn-soft"
    class:is-active={mode === 'focus'}
    aria-pressed={mode === 'focus' ? 'true' : 'false'}
    on:click={() => selectMode('focus')}
  >
    {texts.btn_view_focus}
  </button>
  <button
    type="button"
    class="btn-soft"
    class:is-active={mode === 'all'}
    aria-pressed={mode === 'all' ? 'true' : 'false'}
    on:click={() => selectMode('all')}
  >
    {texts.btn_view_all}
  </button>
</div>

<p class="workspace-hint">{texts.hint_view_focus}</p>

<div class="section-nav" role="navigation" aria-label={texts.section_nav_aria}>
  {#each sections as section (section.id)}
    <a
      href={"#" + section.id}
      class:active={section.active}
      on:click|preventDefault={() => selectSection(section.id)}
    >
      {section.title}
    </a>
  {/each}
</div>

<section class="workspace-summary" aria-live="polite">
  <div class="workspace-summary-label">{texts.workspace_current_label}</div>
  <h2 class="workspace-summary-title">{summary.title}</h2>
  <p class="workspace-summary-desc">{summary.description}</p>
</section>
