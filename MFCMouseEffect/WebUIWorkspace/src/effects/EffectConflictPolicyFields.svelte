<script>
  import { createEventDispatcher } from "svelte";

  export let policy = {};
  export let options = {};

  const dispatch = createEventDispatcher();

  const defaults = {
    hold_move_policy: "hold_only",
  };

  const rows = [
    {
      key: "hold_move_policy",
      labelKey: "label_effect_conflict_hold_move_policy",
      labelDefault: "Hold-Move Coordination",
    },
  ];

  function normalizePolicy(input) {
    const value = input || {};
    return {
      hold_move_policy: value.hold_move_policy || value.hold_move || defaults.hold_move_policy,
    };
  }

  function normalizeOptions(input) {
    const value = input || {};
    return {
      hold_move_policy: Array.isArray(value.hold_move_policy)
        ? value.hold_move_policy
        : Array.isArray(value.hold_move)
          ? value.hold_move
          : [],
    };
  }

  function toSnapshot(form) {
    const value = form || normalizePolicy({});
    return {
      hold_move_policy: value.hold_move_policy || defaults.hold_move_policy,
    };
  }

  let form = normalizePolicy(policy);
  let lastPolicyRef = policy;
  let normalizedOptions = normalizeOptions(options);
  let lastOptionsRef = options;

  $: if (policy !== lastPolicyRef) {
    lastPolicyRef = policy;
    form = normalizePolicy(policy);
  }
  $: if (options !== lastOptionsRef) {
    lastOptionsRef = options;
    normalizedOptions = normalizeOptions(options);
  }
  $: dispatch("change", toSnapshot(form));
</script>

<div class="grid">
  {#each rows as row}
    <label for={`effect_conflict_${row.key}`} data-i18n={row.labelKey}
      >{row.labelDefault}</label
    >
    <select id={`effect_conflict_${row.key}`} bind:value={form[row.key]}>
      {#each normalizedOptions[row.key] as option}
        <option value={option.value}>{option.label}</option>
      {/each}
    </select>
  {/each}
  <div class="hint span2" data-i18n="hint_effect_conflict_policy">
    Choose whether hold effect should run alone during cursor movement, or blend with trail.
  </div>
</div>
