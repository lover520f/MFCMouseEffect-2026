<script>
  import { CARD_SECTIONS } from './sections.js';
</script>

<div class="settings-grid" id="settings_grid">
  {#each CARD_SECTIONS as section}
    <div class={`card ${section.cardClass}`} id={section.id}>
      <h3 data-i18n={section.titleKey}>{section.titleDefault}</h3>
      <p class="card-subtitle" data-i18n={section.descKey}>{section.descDefault}</p>

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
