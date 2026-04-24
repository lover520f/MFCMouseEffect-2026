<script>
  export let description = '';

  export function syncDescription(nextDescription) {
    description = `${nextDescription || ''}`;
  }

  $: text = `${description || ''}`.trim();
</script>

{#if text}
  <p class="workspace-section-hint">{text}</p>
{/if}
