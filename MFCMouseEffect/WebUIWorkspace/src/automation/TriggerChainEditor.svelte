<script>
  import { createEventDispatcher } from 'svelte';
  import { normalizeTriggerChain, serializeTriggerChain } from './trigger-chain.js';

  export let value = [];
  export let options = [];
  export let disabled = false;
  export let maxNodes = 6;
  export let texts = {
    addNode: 'Add chain node',
    removeNode: 'Remove node',
    chainJoiner: 'Then',
  };

  const dispatch = createEventDispatcher();
  let chain = [];

  $: chain = currentChain();

  function currentChain() {
    return normalizeTriggerChain(value, options, '');
  }

  function emitChange(next) {
    const nextChain = normalizeTriggerChain(next, options, '');
    const serialized = serializeTriggerChain(nextChain, options, '');
    chain = nextChain;
    dispatch('chainchange', { value: serialized, chain: nextChain.slice() });
  }

  function updateNode(index, nextValue) {
    const nextChain = currentChain();
    nextChain[index] = nextValue;
    emitChange(nextChain);
  }

  function addNode() {
    const chain = currentChain();
    if (chain.length >= maxNodes) {
      return;
    }
    const fallback = chain.length > 0 ? chain[chain.length - 1] : (options[0]?.value || '');
    emitChange(chain.concat(fallback));
  }

  function removeNode(index) {
    const chain = currentChain();
    if (chain.length <= 1) {
      return;
    }
    emitChange(chain.filter((_item, i) => i !== index));
  }
</script>

{#if chain.length > 0}
  <div class="automation-chain-editor" class:is-disabled={disabled}>
    {#each chain as node, index (index)}
      <div class="automation-chain-node">
        <select
          class="automation-chain-select"
          value={node}
          disabled={disabled}
          on:change={(event) => updateNode(index, event.currentTarget.value)}
        >
          {#each options as option (option.value)}
            <option value={option.value}>{option.label}</option>
          {/each}
        </select>
        {#if !disabled && chain.length > 1}
          <button
            type="button"
            class="btn-soft automation-chain-remove"
            title={texts.removeNode}
            on:click={() => removeNode(index)}
          >
            x
          </button>
        {/if}
      </div>
      {#if index < chain.length - 1}
        <span
          class="automation-chain-joiner"
          title={texts.chainJoiner}
          aria-label={texts.chainJoiner}
        >
          &#8595;
        </span>
      {/if}
    {/each}

    {#if !disabled && chain.length < maxNodes}
      <button type="button" class="btn-soft automation-chain-add" on:click={addNode}>
        + {texts.addNode}
      </button>
    {/if}
  </div>
{/if}
