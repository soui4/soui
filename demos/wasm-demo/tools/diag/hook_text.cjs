/* Hook Canvas2D fillText/measureText，记录调用参数，用来判断"某个控件没画字"
 * 是 SOUI 根本没派发绘制，还是派发了但坐标/裁剪不对。
 *
 * 用法：node tools/diag/hook_text.cjs [标签 canvas x] [要查的子串]
 *   例：node tools/diag/hook_text.cjs 229 "SOUI 4"
 *
 * 做法：Page.addScriptToEvaluateOnNewDocument 在模块加载前装钩子，
 * 记录进 window.__ft（环形缓冲，只留最近 N 条）。点击标签后停录，读回来。
 */
'use strict';
const http = require('http');
const path = require('path');

const PORT = 9222;
const PAGE = process.env.DEMO_URL || 'http://127.0.0.1:8123/soui-wasm-demo.html';
const TABX = parseFloat(process.argv[2] || '229');
const NEEDLE = process.argv[3] || 'SOUI';
const CAP = 4000;

function getJson(url) {
  return new Promise((res, rej) => {
    http
      .get(url, (r) => {
        let d = '';
        r.on('data', (c) => (d += c));
        r.on('end', () => res(JSON.parse(d)));
      })
      .on('error', rej);
  });
}

const HOOK = `
(function(){
  window.__ft = [];
  window.__rec = false;
  var P = CanvasRenderingContext2D.prototype;
  var of = P.fillText, om = P.measureText;
  function push(kind, t, x, y){
    if (!window.__rec) return;
    var a = window.__ft;
    a.push({k:kind, t:String(t), x:x, y:y,
            f:this.font, c:this.fillStyle,
            m:(this.getTransform? this.getTransform().e+','+this.getTransform().f : '')});
    if (a.length > ${CAP}) a.shift();
  }
  P.fillText = function(t, x, y){ push.call(this,'fill',t,x,y); return of.apply(this, arguments); };
  P.measureText = function(t){ push.call(this,'meas',t,0,0); return om.apply(this, arguments); };
})();
`;

(async () => {
  const list = await getJson(`http://127.0.0.1:${PORT}/json/list`);
  const target = list.find((t) => t.type === 'page');
  const ws = new WebSocket(target.webSocketDebuggerUrl);
  let id = 0;
  const pending = new Map();
  function send(method, params = {}, ms = 30000) {
    const _id = ++id;
    const p = new Promise((r) => pending.set(_id, r));
    ws.send(JSON.stringify({ id: _id, method, params }));
    return Promise.race([p, new Promise((r) => setTimeout(() => r({ __timeout: method }), ms))]);
  }
  ws.onmessage = (ev) => {
    const m = JSON.parse(ev.data);
    if (m.id && pending.has(m.id)) {
      pending.get(m.id)(m);
      pending.delete(m.id);
    }
  };
  await new Promise((res, rej) => { ws.onopen = res; ws.onerror = rej; });
  await send('Runtime.enable');
  await send('Page.enable');
  await send('Network.enable');
  await send('Network.setCacheDisabled', { cacheDisabled: true });
  await send('Page.addScriptToEvaluateOnNewDocument', { source: HOOK });

  await send('Page.navigate', { url: PAGE + '?t=' + Date.now() }, 30000);

  /* 等加载完 */
  const t0 = Date.now();
  while (Date.now() - t0 < 25000) {
    await new Promise((r) => setTimeout(r, 1500));
    const r = await send('Runtime.evaluate', {
      expression: "!!(document.getElementById('loading')||{}).classList && document.getElementById('loading').classList.contains('done')",
      returnByValue: true,
    }, 12000);
    if (r && !r.__timeout && r.result && r.result.result && r.result.result.value) break;
  }

  /* 拿 canvas 位置换算点击点 */
  const box = await send('Runtime.evaluate', {
    expression: "(function(){var c=document.getElementById('screen');var r=c.getBoundingClientRect();return JSON.stringify({x:r.left,y:r.top,w:r.width,h:r.height,cw:c.width,ch:c.height});})()",
    returnByValue: true,
  });
  const b = JSON.parse(box.result.result.value);
  const sx = b.x + (TABX / b.cw) * b.w;
  const sy = b.y + (64 / b.ch) * b.h;

  /* 开始录 -> 点击 -> 停录 */
  await send('Runtime.evaluate', { expression: 'window.__ft=[];window.__rec=true;' });
  for (const type of ['mousePressed', 'mouseReleased']) {
    await send('Input.dispatchMouseEvent', {
      type, x: sx, y: sy, button: 'left', clickCount: 1,
      buttons: type === 'mousePressed' ? 1 : 0,
    });
  }
  await new Promise((r) => setTimeout(r, 2000));
  await send('Runtime.evaluate', { expression: 'window.__rec=false;' });

  const needle = JSON.stringify(NEEDLE);
  const r = await send('Runtime.evaluate', {
    expression: `(function(){
      var n=${needle}, a=window.__ft||[];
      var hit=a.filter(function(o){return o.k==='fill' && o.t.indexOf(n)>=0;});
      var meas=a.filter(function(o){return o.k==='meas' && o.t.indexOf(n)>=0;});
      return JSON.stringify({
        total:a.length,
        fillHits:hit.slice(0,12),
        measHits:meas.slice(0,6),
        fillAll:hit.length, measAll:meas.length
      });
    })()`,
    returnByValue: true,
  }, 20000);
  console.log('RESULT ' + (r && r.result && r.result.result ? r.result.result.value : JSON.stringify(r).slice(0, 300)));

  ws.close();
  process.exit(0);
})();
