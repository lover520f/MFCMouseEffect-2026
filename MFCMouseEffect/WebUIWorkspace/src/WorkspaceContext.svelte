<script>
  export let summary = { title: '', description: '' };
  export let texts = {
    workspace_current_label: 'Current Section',
  };

  $: hasSummary = Boolean((summary?.title || '').trim()) || Boolean((summary?.description || '').trim());
</script>

{#if hasSummary}
  <section class="workspace-context" aria-live="polite">
    <div class="workspace-context-label">{texts.workspace_current_label}</div>
    <h2 class="workspace-context-title">{summary.title}</h2>
    <p class="workspace-context-desc">{summary.description}</p>
  </section>
{/if}
