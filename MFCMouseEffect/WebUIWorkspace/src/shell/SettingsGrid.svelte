<script>
  import { CARD_SECTIONS } from './sections.js';
</script>

<div class="settings-grid" id="settings_grid">
  {#each CARD_SECTIONS as section}
    <div
      class={`card ${section.cardClass}`}
      id={section.id}
      data-section-title-key={section.titleKey}
      data-section-title-default={section.titleDefault}
      data-section-desc-key={section.descKey}
      data-section-desc-default={section.descDefault}
    >
      {#if section.wrapperClass}
        <div class={section.wrapperClass}>
          <div id={section.mountId} class={section.mountClass}></div>
        </div>
      {:else}
        <div id={section.mountId}></div>
      {/if}
    </div>
  {/each}
</div>
