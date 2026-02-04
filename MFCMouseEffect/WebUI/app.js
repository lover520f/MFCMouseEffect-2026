(() => {
  const token = new URL(location.href).searchParams.get('token') || '';
  const toastEl = document.getElementById('toast');
  const toast = (msg) => { toastEl.textContent = msg || ''; };
  const el = (id) => document.getElementById(id);

  async function apiGet(path){
    const r = await fetch(path, {headers: {'X-MFCMouseEffect-Token': token}});
    if(!r.ok) throw new Error(await r.text());
    return await r.json();
  }
  async function apiPost(path, obj){
    const r = await fetch(path, {
      method:'POST',
      headers:{'Content-Type':'application/json','X-MFCMouseEffect-Token': token},
      body: JSON.stringify(obj || {})
    });
    if(!r.ok) throw new Error(await r.text());
    return await r.json();
  }

  function fillSelect(sel, items, current){
    sel.innerHTML = '';
    for(const it of items || []){
      const o = document.createElement('option');
      o.value = it.value;
      o.textContent = it.label;
      sel.appendChild(o);
    }
    if(current) sel.value = current;
  }

  function num(id, v){ el(id).value = (v ?? '').toString(); }
  function getNum(id){ return Number(el(id).value || 0); }

  function scrollToHash(){
    const h = (location.hash || '').replace('#','');
    if(!h) return;
    const node = document.getElementById(h);
    if(node) node.scrollIntoView({behavior:'smooth', block:'start'});
  }

  async function reload(){
    toast('Loading...');
    const schema = await apiGet('/api/schema');
    const st = await apiGet('/api/state');

    fillSelect(el('ui_language'), schema.ui_languages, st.ui_language);
    fillSelect(el('theme'), schema.themes, st.theme);
    fillSelect(el('click'), schema.effects?.click, st.active?.click);
    fillSelect(el('trail'), schema.effects?.trail, st.active?.trail);
    fillSelect(el('scroll'), schema.effects?.scroll, st.active?.scroll);
    fillSelect(el('hold'), schema.effects?.hold, st.active?.hold);
    fillSelect(el('hover'), schema.effects?.hover, st.active?.hover);

    el('text_content').value = st.text_content || '';

    el('trail_style').value = st.trail_style || 'default';
    const p = st.trail_profiles || {};
    num('p_streamer_duration', p.streamer?.duration_ms); num('p_streamer_max', p.streamer?.max_points);
    num('p_electric_duration', p.electric?.duration_ms); num('p_electric_max', p.electric?.max_points);
    num('p_meteor_duration', p.meteor?.duration_ms);     num('p_meteor_max', p.meteor?.max_points);
    num('p_tubes_duration', p.tubes?.duration_ms);       num('p_tubes_max', p.tubes?.max_points);
    num('p_line_duration', p.line?.duration_ms);         num('p_line_max', p.line?.max_points);

    const k = st.trail_params || {};
    num('k_streamer_glow', k.streamer?.glow_width_scale);
    num('k_streamer_core', k.streamer?.core_width_scale);
    num('k_streamer_head', k.streamer?.head_power);
    num('k_electric_amp', k.electric?.amplitude_scale);
    num('k_electric_fork', k.electric?.fork_chance);
    num('k_meteor_rate', k.meteor?.spark_rate_scale);
    num('k_meteor_speed', k.meteor?.spark_speed_scale);

    toast('Ready.');
  }

  function buildState(){
    return {
      ui_language: el('ui_language').value,
      theme: el('theme').value,
      active: {
        click: el('click').value,
        trail: el('trail').value,
        scroll: el('scroll').value,
        hold: el('hold').value,
        hover: el('hover').value,
      },
      text_content: el('text_content').value || '',
      trail_style: el('trail_style').value,
      trail_profiles: {
        line:     {duration_ms: getNum('p_line_duration'),     max_points: getNum('p_line_max')},
        streamer: {duration_ms: getNum('p_streamer_duration'), max_points: getNum('p_streamer_max')},
        electric: {duration_ms: getNum('p_electric_duration'), max_points: getNum('p_electric_max')},
        meteor:   {duration_ms: getNum('p_meteor_duration'),   max_points: getNum('p_meteor_max')},
        tubes:    {duration_ms: getNum('p_tubes_duration'),    max_points: getNum('p_tubes_max')},
      },
      trail_params: {
        streamer: {
          glow_width_scale: getNum('k_streamer_glow'),
          core_width_scale: getNum('k_streamer_core'),
          head_power: getNum('k_streamer_head')
        },
        electric: {
          amplitude_scale: getNum('k_electric_amp'),
          fork_chance: getNum('k_electric_fork')
        },
        meteor: {
          spark_rate_scale: getNum('k_meteor_rate'),
          spark_speed_scale: getNum('k_meteor_speed')
        },
      }
    };
  }

  el('btnReload').addEventListener('click', async () => {
    try{
      toast('Reloading config...');
      await apiPost('/api/reload', {});
      await reload();
      scrollToHash();
    }catch(e){
      toast('Reload failed: ' + e.message);
    }
  });

  el('btnSave').addEventListener('click', async () => {
    try{
      toast('Saving...');
      const st = buildState();
      const res = await apiPost('/api/state', st);
      toast(res.ok ? 'Saved.' : ('Failed: ' + (res.error || '')));
    }catch(e){
      toast('Save failed: ' + e.message);
    }
  });

  reload().then(scrollToHash).catch(e => toast('Load failed: ' + e.message));
})();

