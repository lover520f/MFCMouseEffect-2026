(() => {
  function create(options) {
    const opts = options || {};
    const catalog = opts.catalog || {};
    const getElement = typeof opts.getElement === 'function'
      ? opts.getElement
      : (id) => document.getElementById(id);
    const syncConsumers = typeof opts.syncConsumers === 'function'
      ? opts.syncConsumers
      : () => {};
    let activeLang = '';
    let mutationObserver = null;

    function pickLang() {
      const languageSelect = getElement('ui_language');
      const selected = languageSelect ? languageSelect.value : '';
      if (selected) return selected;
      const browserLang = (navigator.language || '').toLowerCase();
      if (browserLang.startsWith('zh')) return 'zh-CN';
      return 'en-US';
    }

    function resolveText(lang) {
      const key = lang || activeLang || pickLang();
      return catalog[key] || catalog['en-US'] || {};
    }

    function currentText() {
      return resolveText();
    }

    function applySingleNode(node, text) {
      if (!node || node.nodeType !== 1) {
        return;
      }
      const i18nKey = node.getAttribute('data-i18n');
      if (i18nKey && text[i18nKey] && node.textContent !== text[i18nKey]) {
        node.textContent = text[i18nKey];
      }

      const titleKey = node.getAttribute('data-i18n-title');
      if (titleKey && text[titleKey] && node.getAttribute('title') !== text[titleKey]) {
        node.setAttribute('title', text[titleKey]);
      }

      const placeholderKey = node.getAttribute('data-i18n-placeholder');
      if (placeholderKey && text[placeholderKey] && node.getAttribute('placeholder') !== text[placeholderKey]) {
        node.setAttribute('placeholder', text[placeholderKey]);
      }
    }

    function applyNodeTree(root, text) {
      if (!root || root.nodeType !== 1) {
        return;
      }
      applySingleNode(root, text);
      root.querySelectorAll('[data-i18n], [data-i18n-title], [data-i18n-placeholder]')
        .forEach((node) => applySingleNode(node, text));
    }

    function applyDocumentNodes(text) {
      const root = document.documentElement || document.body;
      if (!root) {
        return;
      }
      applyNodeTree(root, text);
    }

    function applyTrailStyleLabels(text) {
      const styleMap = {
        default: text.style_default || 'default',
        snappy: text.style_snappy || 'snappy',
        long: text.style_long || 'long',
        cinematic: text.style_cinematic || 'cinematic',
        custom: text.style_custom || 'custom',
      };
      const trailStyleSelect = getElement('trail_style');
      if (!trailStyleSelect) {
        return;
      }
      Array.from(trailStyleSelect.options).forEach((option) => {
        const value = option.value;
        if (styleMap[value] && option.textContent !== styleMap[value]) {
          option.textContent = styleMap[value];
        }
      });
    }

    function syncDynamicNodes(mutations) {
      const text = currentText();
      let needsTrailStyleSync = false;
      for (const mutation of mutations) {
        if (mutation.type === 'attributes') {
          applySingleNode(mutation.target, text);
          continue;
        }
        if (mutation.type !== 'childList') {
          continue;
        }
        mutation.addedNodes.forEach((node) => {
          if (!node || node.nodeType !== 1) {
            return;
          }
          applyNodeTree(node, text);
          if (node.id === 'trail_style'
            || (typeof node.querySelector === 'function' && node.querySelector('#trail_style'))) {
            needsTrailStyleSync = true;
          }
        });
      }
      if (needsTrailStyleSync) {
        applyTrailStyleLabels(text);
      }
    }

    function ensureMutationObserver() {
      if (mutationObserver || typeof MutationObserver !== 'function') {
        return;
      }
      const root = document.documentElement || document.body;
      if (!root) {
        return;
      }
      mutationObserver = new MutationObserver(syncDynamicNodes);
      mutationObserver.observe(root, {
        childList: true,
        subtree: true,
        attributes: true,
        attributeFilter: ['data-i18n', 'data-i18n-title', 'data-i18n-placeholder'],
      });
    }

    function apply(lang) {
      activeLang = catalog[lang] ? lang : 'en-US';
      const text = resolveText(activeLang);
      document.title = text.title || 'MFCMouseEffect Settings';

      applyDocumentNodes(text);
      applyTrailStyleLabels(text);
      ensureMutationObserver();

      try {
        syncConsumers(text);
      } catch (_error) {
        // Ignore consumer sync failures to avoid breaking settings reload flow.
      }
      return text;
    }

    return {
      pickLang,
      currentText,
      apply,
    };
  }

  window.MfxI18nRuntime = {
    create,
  };
})();
