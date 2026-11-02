/* 记录每次 ctx.fill 的「路径段数 / 填充色 / 变换 / globalAlpha」。
 *
 * 用途：区分"这条 fill 根本没发生"和"发生了但路径是空的/颜色不对/变换丢了"。
 * 路径段数为 0 说明画的时候路径已经空掉（FillPath 前的 BeginPath/EndPath 没记上），
 * 段数正常却没有像素，就往填充色、变换、裁剪去查。
 *
 * 用法：node tools/diag/probe_fill.cjs [tabIndex]
 *   先跑过 cmp_shot.cjs（它写出 build/cmp/canvas2d/tabs.json，里面是各标签中心 x），
 *   本脚本用那份坐标去点标签，再回读 window.__log。
 */
'use strict';
const http = require('http');
const fs = require('fs');
const path = require('path');
const PORT = 9222;
const TAB = parseInt(process.argv[2] || '12', 10);

const HOOK = `
(function(){
  var P = CanvasRenderingContext2D.prototype;
  var seg = 0, rec = [];
  window.__log = rec;
  var ob = P.beginPath;
  P.beginPath = function(){ seg = 0; return ob.apply(this, arguments); };
  ['moveTo','lineTo','bezierCurveTo','quadraticCurveTo','closePath','ellipse','arc','rect'].forEach(function(n){
    var o = P[n];
    P[n] = function(){ seg++; return o.apply(this, arguments); };
  });
  var of = P.fill;
  P.fill = function(){
    if (rec.length < 3000) {
      var t = this.getTransform();
      rec.push([seg, (typeof this.fillStyle === 'string' ? this.fillStyle : '[grad]'),
                t.a.toFixed(3), t.d.toFixed(3), t.e.toFixed(1), t.f.toFixed(1), this.globalAlpha]);
    }
    return of.apply(this, arguments);
  };
})();
`;

function getJson(url) {
  return new Promise((res, rej) => {
    http.get(url, (r) => { let d = ''; r.on('data', (c) => (d += c)); r.on('end', () => { try { res(JSON.parse(d)); } catch (e) { rej(e); } }); }).on('error', rej);
  });
}

(async () => {
  const list = await getJson(`http://127.0.0.1:${PORT}/json/list`);
  const target = list.find((t) => t.type === 'page' && t.url.indexOf('soui-wasm-demo') >= 0) || list[0];
  const ws = new WebSocket(target.webSocketDebuggerUrl);
  let id = 0;
  const pending = new Map();
  function send(method, params = {}, ms = 25000) {
    const _id = ++id;
    const p = new Promise((r) => pending.set(_id, r));
    ws.send(JSON.stringify({ id: _id, method, params }));
    return Promise.race([p, new Promise((r) => setTimeout(() => r({ __timeout: method }), ms))]);
  }
  ws.onmessage = (ev) => {
    const m = JSON.parse(ev.data);
    if (m.id && pending.has(m.id)) { pending.get(m.id)(m); pending.delete(m.id); }
  };
  await new Promise((res, rej) => { ws.onopen = res; ws.onerror = rej; });
  await send('Runtime.enable');
  await send('Page.enable');
  await send('Network.enable');
  await send('Network.setCacheDisabled', { cacheDisabled: true });
  await send('Emulation.setDeviceMetricsOverride', { width: 1000, height: 1400, deviceScaleFactor: 1, mobile: false });
  await send('Page.addScriptToEvaluateOnNewDocument', { source: HOOK });
  await send('Page.navigate', { url: 'http://127.0.0.1:8125/soui-wasm-demo.html?t=' + Date.now() }, 40000);
  for (let i = 0; i < 60; i++) {
    await new Promise((r) => setTimeout(r, 1000));
    const r = await send('Runtime.evaluate', {
      expression: "(function(){var c=document.getElementById('screen'),ld=document.getElementById('loading');return JSON.stringify({cw:c?c.width:0,done:ld?ld.classList.contains('done'):null});})()",
      returnByValue: true,
    });
    const v = r && r.result && r.result.result ? r.result.result.value : null;
    if (v) { const st = JSON.parse(v); if (st.done && st.cw > 0) break; }
  }
  await new Promise((r) => setTimeout(r, 2500));
  const box = JSON.parse((await send('Runtime.evaluate', {
    expression: "(function(){var c=document.getElementById('screen');var r=c.getBoundingClientRect();return JSON.stringify({x:r.left,y:r.top,w:r.width,h:r.height,cw:c.width,ch:c.height});})()",
    returnByValue: true,
  })).result.result.value);
  const tabs = JSON.parse(fs.readFileSync(path.join(__dirname, '..', '..', 'build', 'cmp', 'canvas2d', 'tabs.json'), 'utf8'));
  const t = tabs[TAB];
  const sx = box.x + (t.cx / box.cw) * box.w;
  const sy = box.y + ((76.5 - 4) / box.ch) * box.h;
  for (const type of ['mousePressed', 'mouseReleased']) {
    await send('Input.dispatchMouseEvent', { type, x: sx, y: sy, button: 'left', clickCount: 1, buttons: type === 'mousePressed' ? 1 : 0 });
  }
  await new Promise((r) => setTimeout(r, 2500));
  const res = await send('Runtime.evaluate', {
    expression: `(function(){
      var r = window.__log || [];
      var svg = r.filter(function(e){ return e[2] !== '1.000'; });
      return JSON.stringify({total:r.length, nonUnit:svg.length, tail:svg.slice(-50)});
    })()`,
    returnByValue: true,
  });
  console.log(res.result.result.value);
  ws.close();
  process.exit(0);
})();
