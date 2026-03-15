<script>
  import { createEventDispatcher } from 'svelte';
  import AutomationSidebarDebugCard from './AutomationSidebarDebugCard.svelte';

  export let sections = [];
  export let texts = {
    section_nav_aria: 'Settings sections',
  };
  export let activeSectionId = '';
  export let runtimePlatform = 'windows';
  export let runtimeState = {};
  export let i18n = {};

  const dispatch = createEventDispatcher();

  function selectSection(id) {
    dispatch('select', { id });
  }

</script>

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

{#if activeSectionId === 'automation'}
  <AutomationSidebarDebugCard
    payloadState={runtimeState}
    platform={runtimePlatform}
    i18n={i18n}
  />
{/if}
