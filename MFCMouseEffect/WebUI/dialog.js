(() => {
  let maskEl = null;
  let onClose = null;

  function removeDialog() {
    if (!maskEl) return;
    document.removeEventListener('keydown', handleEsc);
    document.body.classList.remove('mfx-modal-open');
    maskEl.remove();
    maskEl = null;
    const close = onClose;
    onClose = null;
    if (close) close();
  }

  function handleEsc(e) {
    if (e.key === 'Escape') removeDialog();
  }

  function buildDialog(title, message, okText) {
    const mask = document.createElement('div');
    mask.className = 'mfx-modal-mask';
    mask.addEventListener('click', (e) => {
      if (e.target === mask) removeDialog();
    });

    const card = document.createElement('section');
    card.className = 'mfx-modal-card';
    card.setAttribute('role', 'dialog');
    card.setAttribute('aria-modal', 'true');
    card.setAttribute('aria-label', title || 'Notice');

    const h = document.createElement('h4');
    h.className = 'mfx-modal-title';
    h.textContent = title || 'Notice';

    const p = document.createElement('p');
    p.className = 'mfx-modal-text';
    p.textContent = message || '';

    const actions = document.createElement('div');
    actions.className = 'mfx-modal-actions';

    const okBtn = document.createElement('button');
    okBtn.className = 'primary';
    okBtn.type = 'button';
    okBtn.textContent = okText || 'OK';
    okBtn.addEventListener('click', removeDialog);

    actions.appendChild(okBtn);
    card.appendChild(h);
    card.appendChild(p);
    card.appendChild(actions);
    mask.appendChild(card);
    return { mask, okBtn };
  }

  function showNotice(opts) {
    const options = opts || {};
    removeDialog();
    const built = buildDialog(options.title, options.message, options.okText);
    maskEl = built.mask;
    onClose = options.onClose || null;
    document.body.appendChild(maskEl);
    document.body.classList.add('mfx-modal-open');
    document.addEventListener('keydown', handleEsc);
    built.okBtn.focus();
  }

  window.MfxDialog = {
    showNotice
  };
})();
