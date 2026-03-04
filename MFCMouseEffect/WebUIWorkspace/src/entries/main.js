import WorkspaceSidebar from '../WorkspaceSidebar.svelte';

const state = {
  initialized: false,
  hashBound: false,
  sections: [],
  activeId: '',
  i18n: null,
  component: null,
};

function el(id) {
  return document.getElementById(id);
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
}

function setActive(sectionId, options) {
  const opts = options || {};
  state.activeId = pickAvailableSectionId(sectionId);
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
      summary: { title: '', description: '' },
      texts: workspaceTexts(),
    },
  });

  component.$on('select', (event) => {
    const id = event?.detail?.id;
    setActive(id, { updateHash: true });
  });

  state.component = component;
}

function init() {
  collectSections();
  ensureSidebarComponent();
  bindHashChange();

  state.activeId = pickAvailableSectionId(currentHashId());

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
  bindHashChange();

  state.activeId = pickAvailableSectionId(state.activeId || currentHashId());
  render({ updateHash: false });
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
