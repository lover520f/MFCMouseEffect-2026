<script>
  import { createEventDispatcher } from 'svelte';

  export let sections = [];
  export let texts = {
    section_nav_aria: 'Settings sections',
  };
  export let onSelect = null;

  const dispatch = createEventDispatcher();

  export function syncView(nextSections, nextTexts) {
    sections = Array.isArray(nextSections) ? nextSections : [];
    texts = (nextTexts && typeof nextTexts === 'object') ? nextTexts : {
      section_nav_aria: 'Settings sections',
    };
  }

  function selectSection(id) {
    if (typeof onSelect === 'function') {
      onSelect({ id });
    }
    dispatch('select', { id });
  }

</script>

<div class="section-nav section-nav--top" role="navigation" aria-label={texts.section_nav_aria}>
  {#each sections as section (section.id)}
    <a
      href={"#" + section.id}
      class:active={section.active}
      class="section-nav__link"
      on:click|preventDefault={() => selectSection(section.id)}
    >
      {section.title}
    </a>
  {/each}
</div>
