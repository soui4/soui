/* HiDPI（SOUI SetScale 方案）验收：用 CDP 的 Emulation.setDeviceMetricsOverride
 * 模拟 devicePixelRatio，重载 demo 页，报告 fb/画布/档位状态并截图。
 *
 * 用法：node tools/diag/shoot_dpi.cjs <dsf> [等待毫秒] [输出名]
 *   dsf 例：1 / 1.5 / 2 （1.5 → SOUI 档位 150，fb 1200x900）
 * 输出：build/dpi/<dsf>.png + 一行状态 JSON（canvas 物理尺寸、fb、dpr）。
 */
'use strict';
const fs = require('fs');
const http = require('http');
const path = require('path');

const PORT = 9222;
const DSF = parseFloat(process.argv[2] || '1.5');
const WAIT = parseInt(process.argv[3] || '20000', 10);
const NAME = process.argv[4] || String(DSF).replace('.', '_');
const PAGE = process.env.DEMO_URL || 'http://127.0.0.1:8125/soui-wasm-demo.html';
const OUTDIR = path.join(__dirname, '..', '..', 'build', 'dpi');
fs.mkdirSync(OUTDIR, { recursive: true });
const OUT = path.join(OUTDIR, NAME + '.png');

function getJson(url) {
  return new Promise((res, rej) => {
    http.get(url, (r) => {
      let d = '';
      r.on('data', (c) => (d += c));
      r.on('end', () => { try { res(JSON.parse(d)); } catch (e) { rej(e); } });
    }).on('error', rej);
  });
}

(async () => {
  const list = await getJson(`http://127.0.0.1:${PORT}/json/list`);
  const target = list.find((t) => t.type === 'page');
  if (!target) { console.error('没有 page target'); process.exit(2); }

  const ws = new WebSocket(target.webSocketDebuggerUrl);
  let id = 0;
  const pending = new Map();
  function send(method, params = {}, ms = 25000) {
    const _id = ++id;
    const p = new Promise((r) => pending.set(_id, r));
    ws.send(JSON.stringify({ id: _id, method, params }));
    return Promise.race([p, new Promise((r) => setTimeout(() => r({ __timeout: method }), ms))]);
  }
  ws.addEventListener('message', (ev) => {
    const m = JSON.parse(ev.data);
    if (m.id && pending.has(m.id)) { pending.get(m.id)(m); pending.delete(m.id); }
  });
  await new Promise((res, rej) => {
    ws.addEventListener('open', res, { once: true });
    ws.addEventListener('error', rej, { once: true });
  });

  await send('Runtime.enable');
  await send('Page.enable');
  await send('Network.enable');
  await send('Network.setCacheDisabled', { cacheDisabled: true });
  // 模拟 HiDPI：viewport 900x700 逻辑，devicePixelRatio=DSF
  await send('Emulation.setDeviceMetricsOverride', {
    width: 900, height: 700, deviceScaleFactor: DSF, mobile: false,
  });

  const url = PAGE + '?t=' + Date.now();
  console.log('NAVIGATE ' + url);
  await send('Page.navigate', { url }, 30000);

  const t0 = Date.now();
  let st = null;
  while (Date.now() - t0 < WAIT) {
    await new Promise((r) => setTimeout(r, 1500));
    const r = await send('Runtime.evaluate', {
      expression:
        "(function(){var c=document.getElementById('screen'),ld=document.getElementById('loading'),l=document.getElementById('log');" +
        "var fb={w:0,h:0};try{fb={w:Module._SwinxFbWidth(),h:Module._SwinxFbHeight()};}catch(e){}" +
        "return JSON.stringify({cw:c?c.width:0,ch:c?c.height:0,cssW:c?c.getBoundingClientRect().width:0," +
        "dpr:window.devicePixelRatio,fb:fb,loadingDone:ld?ld.classList.contains('done'):null," +
        "failed:ld?ld.classList.contains('failed'):null,log:l?l.textContent:''});})()",
      returnByValue: true,
    }, 12000);
    if (r && r.__timeout) continue;
    const v = r && r.result && r.result.result ? r.result.result.value : null;
    if (v) {
      st = JSON.parse(v);
      console.log('STATUS ' + v);
      if (st.loadingDone && st.cw > 0) break;
      if (st.failed) break;
    }
  }

  await new Promise((r) => setTimeout(r, 3000));
  const du = await send('Runtime.evaluate', {
    expression:
      "(function(){var c=document.getElementById('screen');if(!c)return '';try{return c.toDataURL('image/png');}catch(e){return 'ERR:'+e.message;}})()",
    returnByValue: true,
  }, 60000);
  const val = du && du.result && du.result.result ? du.result.result.value : undefined;
  if (typeof val === 'string' && val.startsWith('data:image/png;base64,')) {
    const buf = Buffer.from(val.split(',')[1], 'base64');
    fs.writeFileSync(OUT, buf);
    console.log('WROTE ' + OUT + ' bytes=' + buf.length);
  } else {
    console.log('NO_CANVAS_DATA ' + JSON.stringify(du).slice(0, 400));
  }
  ws.close();
  process.exit(0);
})().catch((e) => { console.error('FATAL ' + (e && e.message)); process.exit(1); });
