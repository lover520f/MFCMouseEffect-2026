import WorkspaceSidebar from '../WorkspaceSidebar.svelte';

const MODE_STORAGE_KEY = 'mfx_webui_view_mode';

const state = {
  initialized: false,
  hashBound: false,
  mode: 'focus',
  sections: [],
  activeId: '',
  i18n: null,
  component: null,
};

function el(id) {
  return document.getElementById(id);
}

function normalizeMode(mode) {
  return mode === 'all' ? 'all' : 'focus';
}

function currentHashId() {
  return (location.hash || '').replace('#', '').trim();
}

function readStoredMode() {
  try {
    return normalizeMode(window.localStorage.getItem(MODE_STORAGE_KEY) || 'focus');
  } catch (_e) {
    return 'focus';
  }
}

function persistMode(mode) {
  try {
    window.localStorage.setItem(MODE_STORAGE_KEY, normalizeMode(mode));
  } catch (_e) {
    // Ignore storage errors in restricted environments.
  }
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

function pickAvailableSectionId(candidate) {
  if (candidate && sectionById(candidate)) {
    return candidate;
  }
  const byHash = currentHashId();
  if (byHash && sectionById(byHash)) {
    return byHash;
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
    btn_view_focus: i18n.btn_view_focus || 'Focused View',
    btn_view_all: i18n.btn_view_all || 'All Sections',
    hint_view_focus: i18n.hint_view_focus || 'Focused view shows one section at a time to reduce noise.',
    workspace_current_label: i18n.workspace_current_label || 'Current Section',
    view_mode_aria: i18n.view_mode_aria || 'View mode',
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
  const grid = el('settings_grid');
  if (grid) {
    grid.classList.toggle('is-all-mode', state.mode === 'all');
  }

  const allMode = state.mode === 'all';
  for (const section of state.sections) {
    const visible = allMode || section.id === state.activeId;
    section.card.classList.toggle('is-active', visible);
    section.card.hidden = !visible;
  }
}

function updateSidebarView() {
  if (!state.component) {
    return;
  }
  state.component.$set({
    mode: state.mode,
    sections: sectionsViewModel(),
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

  ensureSectionTexts();
  applyCardsVisibility();
  updateSidebarView();

  if (opts.updateHash !== false) {
    updateHashIfNeeded(state.activeId);
  }

  if (opts.scroll && state.mode === 'all') {
    const section = sectionById(state.activeId);
    if (section) {
      section.card.scrollIntoView({ behavior: 'smooth', block: 'start' });
    }
  }
}

function setActive(sectionId, options) {
  const opts = options || {};
  state.activeId = pickAvailableSectionId(sectionId);
  render({
    updateHash: opts.updateHash !== false,
    scroll: !!opts.scroll,
  });
}

function setMode(mode, options) {
  const opts = options || {};
  state.mode = normalizeMode(mode);
  persistMode(state.mode);

  render({
    updateHash: opts.updateHash !== false,
    scroll: !!opts.scroll,
  });
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
      mode: state.mode,
      sections: [],
      summary: { title: '', description: '' },
      texts: workspaceTexts(),
    },
  });

  component.$on('select', (event) => {
    const id = event?.detail?.id;
    setActive(id, { updateHash: true, scroll: state.mode === 'all' });
  });

  component.$on('mode', (event) => {
    const mode = event?.detail?.mode;
    setMode(mode, { updateHash: true, scroll: mode === 'all' });
  });

  state.component = component;
}

function init() {
  collectSections();
  ensureSidebarComponent();
  bindHashChange();

  state.mode = readStoredMode();
  state.activeId = pickAvailableSectionId(currentHashId());

  render({ updateHash: true, scroll: false });
  state.initialized = true;
}

function refresh() {
  if (!state.initialized) {
    init();
    return;
  }

  collectSections();
  ensureSidebarComponent();
  bindHashChange();

  state.activeId = pickAvailableSectionId(state.activeId || currentHashId());
  render({ updateHash: false, scroll: false });
}

function syncI18n(i18n) {
  state.i18n = i18n || null;
  ensureSectionTexts();
  updateSidebarView();
}

window.MfxSectionWorkspace = {
  init,
  refresh,
  setMode,
  syncI18n,
};
