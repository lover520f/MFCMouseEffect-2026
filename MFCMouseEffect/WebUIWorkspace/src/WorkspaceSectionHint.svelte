<script>
  export let description = '';

  $: text = `${description || ''}`.trim();
</script>

{#if text}
  <p class="workspace-section-hint">{text}</p>
{/if}
