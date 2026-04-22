import WorkspaceSidebar from '../WorkspaceSidebar.svelte';
import WorkspaceSectionHint from '../WorkspaceSectionHint.svelte';
import WorkspaceAutomationAssist from '../WorkspaceAutomationAssist.svelte';
import { syncMountedComponent } from './component-instance.js';
import { readUiState, writeUiState } from './ui-state-storage.js';

const state = {
  initialized: false,
  hashBound: false,
  sections: [],
  activeId: '',
  i18n: null,
  runtimePlatform: 'windows',
  runtimeState: {},
  component: null,
  hintComponent: null,
  auxComponent: null,
  auxRefreshTimer: 0,
  recoverTimer: 0,
  recoverAttempts: 0,
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
  const titleKey = `${card?.dataset?.sectionTitleKey || ''}`.trim();
  const titleDefault = `${card?.dataset?.sectionTitleDefault || ''}`.trim();
  if (titleKey) {
    const fromI18n = `${state.i18n?.[titleKey] || ''}`.trim();
    if (fromI18n) {
      return fromI18n;
    }
  }
  if (titleDefault) {
    return titleDefault;
  }
  const heading = card?.querySelector('h3');
  return heading ? (heading.textContent || '').trim() : '';
}

function readSectionDescription(card) {
  const descKey = `${card?.dataset?.sectionDescKey || ''}`.trim();
  const descDefault = `${card?.dataset?.sectionDescDefault || ''}`.trim();
  if (descKey) {
    const fromI18n = `${state.i18n?.[descKey] || ''}`.trim();
    if (fromI18n) {
      return fromI18n;
    }
  }
  if (descDefault) {
    return descDefault;
  }
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

function clearRecoverTimer() {
  if (!state.recoverTimer) {
    return;
  }
  window.clearTimeout(state.recoverTimer);
  state.recoverTimer = 0;
}

function clearAuxRefreshTimer() {
  if (!state.auxRefreshTimer) {
    return;
  }
  window.clearTimeout(state.auxRefreshTimer);
  state.auxRefreshTimer = 0;
}

function revealCardsFallback() {
  const cards = Array.from(document.querySelectorAll('#settings_grid > .card[id]'));
  for (const card of cards) {
    card.hidden = false;
    card.classList.remove('is-active');
  }
}

function scheduleRecoverRefresh() {
  if (state.recoverTimer) {
    return;
  }
  if (state.recoverAttempts >= 40) {
    return;
  }
  state.recoverTimer = window.setTimeout(() => {
    state.recoverTimer = 0;
    state.recoverAttempts += 1;
    collectSections();
    if (state.sections.length <= 0) {
      scheduleRecoverRefresh();
      return;
    }
    state.recoverAttempts = 0;
    refresh();
  }, 50);
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
      sectionId: 'plugins',
      effectsTabId: '',
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

function sectionsViewModel() {
  return state.sections.map((section) => ({
    id: section.id,
    title: section.title || section.id,
    active: section.id === state.activeId,
  }));
}

function activeSectionDescription() {
  const section = sectionById(state.activeId);
  return `${section?.description || ''}`.trim();
}

function workspaceTexts() {
  const i18n = state.i18n || {};
  return {
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
  const mountNode = el('workspace_sidebar_mount');
  state.component = syncMountedComponent(state.component, mountNode, createSidebarComponent, {
    sections: sectionsViewModel(),
    texts: workspaceTexts(),
  });
}

function updateHintView() {
  if (!state.hintComponent) {
    return;
  }
  const mountNode = el('workspace_hint_mount');
  state.hintComponent = syncMountedComponent(state.hintComponent, mountNode, createHintComponent, {
    description: activeSectionDescription(),
  });
}

function updateAuxView() {
  if (!state.auxComponent) {
    return;
  }
  const nextI18n = state.i18n || {};
  const mountNode = el('workspace_aux_mount');
  state.auxComponent = syncMountedComponent(state.auxComponent, mountNode, createAuxComponent, {
    activeSectionId: state.activeId,
    runtimePlatform: state.runtimePlatform,
    runtimeState: state.runtimeState,
    i18n: nextI18n,
  });
}

function scheduleAuxViewRefresh() {
  clearAuxRefreshTimer();
  const delayMs = window.__MFX_DEV_RUNTIME__ ? 900 : 0;
  if (delayMs <= 0) {
    updateAuxView();
    return;
  }
  state.auxRefreshTimer = window.setTimeout(() => {
    state.auxRefreshTimer = 0;
    updateAuxView();
  }, delayMs);
}

function createSidebarComponent(mountNode, props) {
  return new WorkspaceSidebar({
    target: mountNode,
    props: {
      ...props,
      onSelect: ({ id }) => {
        setActive(id, { updateHash: true });
      },
    },
  });
}

function createAuxComponent(mountNode, props) {
  return new WorkspaceAutomationAssist({
    target: mountNode,
    props,
  });
}

function createHintComponent(mountNode, props) {
  return new WorkspaceSectionHint({
    target: mountNode,
    props,
  });
}

function render(options) {
  const opts = options || {};
  state.activeId = pickAvailableSectionId(state.activeId);
  if (!state.activeId) {
    revealCardsFallback();
    updateSidebarView();
    updateHintView();
    updateAuxView();
    scheduleRecoverRefresh();
    return;
  }

  clearRecoverTimer();
  state.recoverAttempts = 0;

  writeWorkspaceStorage({
    activeSectionId: state.activeId,
  });

  ensureSectionTexts();
  applyCardsVisibility();
  updateSidebarView();
  updateHintView();
  updateAuxView();

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

  const component = createSidebarComponent(mountNode, {
    sections: [],
    texts: workspaceTexts(),
  });

  state.component = component;
}

function ensureAuxComponent() {
  if (state.auxComponent) {
    return;
  }
  const mountNode = el('workspace_aux_mount');
  if (!mountNode) {
    return;
  }

  state.auxComponent = createAuxComponent(mountNode, {
    activeSectionId: state.activeId,
    runtimePlatform: state.runtimePlatform,
    runtimeState: state.runtimeState,
    i18n: state.i18n || {},
  });
}

function ensureHintComponent() {
  if (state.hintComponent) {
    return;
  }
  const mountNode = el('workspace_hint_mount');
  if (!mountNode) {
    return;
  }

  state.hintComponent = createHintComponent(mountNode, {
    description: activeSectionDescription(),
  });
}

function init() {
  collectSections();
  ensureSidebarComponent();
  ensureHintComponent();
  ensureAuxComponent();
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
  ensureHintComponent();
  ensureAuxComponent();
  bindHashChange();

  state.activeId = pickAvailableSectionId(state.activeId);
  render({ updateHash: false });
}

function syncI18n(i18n) {
  state.i18n = i18n || null;
  ensureSectionTexts();
  updateSidebarView();
  updateHintView();
  scheduleAuxViewRefresh();
}

function normalizePlatform(value) {
  const text = `${value || ''}`.trim().toLowerCase();
  if (text === 'macos' || text === 'windows' || text === 'linux') {
    return text;
  }
  return 'windows';
}

function syncRuntimeState(runtimeState) {
  const source = (runtimeState && typeof runtimeState === 'object') ? runtimeState : {};
  state.runtimePlatform = normalizePlatform(source.platform);
  state.runtimeState = {
    input_automation_gesture_route_status: source.input_automation_gesture_route_status || null,
  };
  scheduleAuxViewRefresh();
}

function getActiveSectionId() {
  return `${state.activeId || ''}`.trim();
}

function isAutomationSectionActive() {
  return getActiveSectionId() === 'automation';
}

window.MfxSectionWorkspace = {
  init,
  refresh,
  setMode,
  syncI18n,
  syncRuntimeState,
  getActiveSectionId,
  isAutomationSectionActive,
};
