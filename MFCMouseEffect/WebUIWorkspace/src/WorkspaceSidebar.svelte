<script>
  import { createEventDispatcher } from 'svelte';

  export let sections = [];
  export let texts = {
    hint_view_focus: 'Focused view shows one section at a time to reduce noise.',
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
