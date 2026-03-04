<script>
  import { createEventDispatcher } from 'svelte';

  export let sections = [];
  export let summary = { title: '', description: '' };
  export let texts = {
    hint_view_focus: 'Focused view shows one section at a time to reduce noise.',
    workspace_current_label: 'Current Section',
    section_nav_aria: 'Settings sections',
  };

  const dispatch = createEventDispatcher();

  function selectSection(id) {
    dispatch('select', { id });
  }

</script>

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
