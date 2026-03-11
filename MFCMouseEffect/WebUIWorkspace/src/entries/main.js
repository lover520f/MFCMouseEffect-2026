import WorkspaceSidebar from '../WorkspaceSidebar.svelte';
import WorkspaceContext from '../WorkspaceContext.svelte';
import { readUiState, writeUiState } from './ui-state-storage.js';

const state = {
  initialized: false,
  hashBound: false,
  sections: [],
  activeId: '',
  i18n: null,
  component: null,
  contextComponent: null,
};

const WORKSPACE_STATE_STORAGE_NS = 'workspace.v1';

function el(id) {
  return document.getElementById(id);
}

function readWorkspaceStorage() {
  return readUiState(WORKSPACE_STATE_STORAGE_NS);
}

function writeWorkspaceStorage(nextState) {
  writeUiState(WORKSPACE_STATE_STORAGE_NS, nextState);
}

function currentHashId() {
  return (location.hash || '').replace('#', '').trim();
}

function readSectionTitle(card) {
  const heading = card?.querySelector('h3');
  return heading ? (heading.textContent || '').trim() : '';
}

function readSectionDescription(card) {
  const subtitle = card?.querySelector('.card-subtitle');
  return subtitle ? (subtitle.textContent || '').trim() : '';
}

function collectSections() {
  const cards = Array.from(document.querySelectorAll('#settings_grid > .card[id]'));
  const out = [];

  for (const card of cards) {
    const id = (card.id || '').trim();
    if (!id) {
      continue;
    }
    out.push({
      id,
      card,
      title: readSectionTitle(card),
      description: readSectionDescription(card),
    });
  }

  state.sections = out;
}

function sectionById(id) {
  for (const section of state.sections) {
    if (section.id === id) {
      return section;
    }
  }
  return null;
}

function firstSectionId() {
  return state.sections.length > 0 ? state.sections[0].id : '';
}

function resolveLegacySectionAlias(candidate) {
  const id = `${candidate || ''}`.trim().toLowerCase();
  if (id === 'wasm') {
    return {
      sectionId: 'active',
      effectsTabId: 'plugin',
    };
  }
  return {
    sectionId: `${candidate || ''}`.trim(),
    effectsTabId: '',
  };
}

function applyLegacySectionAliasEffects(alias) {
  if (!alias || alias.effectsTabId !== 'plugin') {
    return;
  }
  const effectsSection = window.MfxEffectsSection || null;
  if (effectsSection && typeof effectsSection.setActiveTab === 'function') {
    effectsSection.setActiveTab(alias.effectsTabId);
  }
}

function pickAvailableSectionId(candidate) {
  const directAlias = resolveLegacySectionAlias(candidate);
  if (directAlias.sectionId && sectionById(directAlias.sectionId)) {
    applyLegacySectionAliasEffects(directAlias);
    return directAlias.sectionId;
  }

  const persisted = readWorkspaceStorage();
  const persistedAlias = resolveLegacySectionAlias(persisted?.activeSectionId || '');
  if (persistedAlias.sectionId && sectionById(persistedAlias.sectionId)) {
    applyLegacySectionAliasEffects(persistedAlias);
    return persistedAlias.sectionId;
  }

  const hashAlias = resolveLegacySectionAlias(currentHashId());
  if (hashAlias.sectionId && sectionById(hashAlias.sectionId)) {
    applyLegacySectionAliasEffects(hashAlias);
    return hashAlias.sectionId;
  }
  return firstSectionId();
}

function ensureSectionTexts() {
  for (const section of state.sections) {
    section.title = readSectionTitle(section.card);
    section.description = readSectionDescription(section.card);
  }
}

function activeSectionSummary() {
  const section = sectionById(state.activeId);
  if (!section) {
    return { title: '', description: '' };
  }
  return {
    title: section.title || '',
    description: section.description || '',
  };
}

function sectionsViewModel() {
  return state.sections.map((section) => ({
    id: section.id,
    title: section.title || section.id,
    active: section.id === state.activeId,
  }));
}

function workspaceTexts() {
  const i18n = state.i18n || {};
  return {
    hint_view_focus: i18n.hint_view_focus || 'Focused view shows one section at a time to reduce noise.',
    workspace_current_label: i18n.workspace_current_label || 'Current Section',
    section_nav_aria: i18n.section_nav_aria || 'Settings sections',
  };
}

function updateHashIfNeeded(sectionId) {
  if (!sectionId) {
    return;
  }
  const nextHash = `#${sectionId}`;
  if (location.hash === nextHash) {
    return;
  }
  history.replaceState(null, '', nextHash);
}

function applyCardsVisibility() {
  for (const section of state.sections) {
    const visible = section.id === state.activeId;
    section.card.classList.toggle('is-active', visible);
    section.card.hidden = !visible;
  }
}

function updateSidebarView() {
  if (!state.component) {
    return;
  }
  state.component.$set({
    sections: sectionsViewModel(),
    texts: workspaceTexts(),
  });
}

function updateContextView() {
  if (!state.contextComponent) {
    return;
  }
  state.contextComponent.$set({
    summary: activeSectionSummary(),
    texts: workspaceTexts(),
  });
}

function render(options) {
  const opts = options || {};
  state.activeId = pickAvailableSectionId(state.activeId);
  if (!state.activeId) {
    return;
  }

  writeWorkspaceStorage({
    activeSectionId: state.activeId,
  });

  ensureSectionTexts();
  applyCardsVisibility();
  updateSidebarView();
  updateContextView();

  if (opts.updateHash !== false) {
    updateHashIfNeeded(state.activeId);
  }
}

function setActive(sectionId, options) {
  const opts = options || {};
  state.activeId = pickAvailableSectionId(sectionId);
  if (state.activeId) {
    writeWorkspaceStorage({
      activeSectionId: state.activeId,
    });
  }
  render({
    updateHash: opts.updateHash !== false,
  });
}

function setMode(_mode, options) {
  const opts = options || {};
  // Keep API compatibility, but workspace is now always focused mode.
  render({ updateHash: opts.updateHash !== false });
}

function bindHashChange() {
  if (state.hashBound) {
    return;
  }
  state.hashBound = true;

  window.addEventListener('hashchange', () => {
    setActive(currentHashId(), { updateHash: false, scroll: false });
  });
}

function ensureSidebarComponent() {
  if (state.component) {
    return;
  }
  const mountNode = el('workspace_sidebar_mount');
  if (!mountNode) {
    return;
  }

  const component = new WorkspaceSidebar({
    target: mountNode,
    props: {
      sections: [],
      texts: workspaceTexts(),
    },
  });

  component.$on('select', (event) => {
    const id = event?.detail?.id;
    setActive(id, { updateHash: true });
  });

  state.component = component;
}

function ensureContextComponent() {
  if (state.contextComponent) {
    return;
  }
  const mountNode = el('workspace_context_mount');
  if (!mountNode) {
    return;
  }

  state.contextComponent = new WorkspaceContext({
    target: mountNode,
    props: {
      summary: activeSectionSummary(),
      texts: workspaceTexts(),
    },
  });
}

function init() {
  collectSections();
  ensureSidebarComponent();
  ensureContextComponent();
  bindHashChange();

  state.activeId = pickAvailableSectionId(state.activeId);

  render({ updateHash: true });
  state.initialized = true;
}

function refresh() {
  if (!state.initialized) {
    init();
    return;
  }

  collectSections();
  ensureSidebarComponent();
  ensureContextComponent();
  bindHashChange();

  state.activeId = pickAvailableSectionId(state.activeId);
  render({ updateHash: false });
}

function syncI18n(i18n) {
  state.i18n = i18n || null;
  ensureSectionTexts();
  updateSidebarView();
  updateContextView();
}

window.MfxSectionWorkspace = {
  init,
  refresh,
  setMode,
  syncI18n,
};
