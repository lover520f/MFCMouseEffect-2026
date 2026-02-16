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

    function pickLang() {
      const languageSelect = getElement('ui_language');
      const selected = languageSelect ? languageSelect.value : '';
      if (selected) return selected;
      const browserLang = (navigator.language || '').toLowerCase();
      if (browserLang.startsWith('zh')) return 'zh-CN';
      return 'en-US';
    }

    function currentText() {
      return catalog[pickLang()] || catalog['en-US'] || {};
    }

    function apply(lang) {
      const text = catalog[lang] || catalog['en-US'] || {};
      document.title = text.title || 'MFCMouseEffect Settings';

      document.querySelectorAll('[data-i18n]').forEach((node) => {
        const key = node.getAttribute('data-i18n');
        if (key && text[key]) node.textContent = text[key];
      });
      document.querySelectorAll('[data-i18n-title]').forEach((node) => {
        const key = node.getAttribute('data-i18n-title');
        if (key && text[key]) node.setAttribute('title', text[key]);
      });
      document.querySelectorAll('[data-i18n-placeholder]').forEach((node) => {
        const key = node.getAttribute('data-i18n-placeholder');
        if (key && text[key]) node.setAttribute('placeholder', text[key]);
      });

      const styleMap = {
        default: text.style_default || 'default',
        snappy: text.style_snappy || 'snappy',
        long: text.style_long || 'long',
        cinematic: text.style_cinematic || 'cinematic',
        custom: text.style_custom || 'custom',
      };
      const trailStyleSelect = getElement('trail_style');
      if (trailStyleSelect) {
        Array.from(trailStyleSelect.options).forEach((option) => {
          const value = option.value;
          if (styleMap[value]) option.textContent = styleMap[value];
        });
      }

      syncConsumers(text);
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
