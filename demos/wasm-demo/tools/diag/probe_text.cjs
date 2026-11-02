/* 记录 fillText / fillRect 的「文本 + 当前变换 + globalAlpha」。
 *
 * 用于区分三类失败：
 *   ① 根本没调用 —— 窗口没画（裁剪、可见性、层级被跳过）
 *   ② 调用了但变换异常 —— 矩阵把内容推到画面外（e/f 巨大、a/d 为 0）
 *   ③ 调用了但 alpha≈0 —— 透明度动画或 AlphaBlend 路径坏了
 *
 * 用法：
 *   node tools/diag/probe_text.cjs <url> <tabIndex> <filter> [clickX,clickY] [delayMs]
 * 例：
 *   node tools/diag/probe_text.cjs http://127.0.0.1:8125/soui-wasm-demo.html 11 alpha 60,199 250
 */
'use strict';
const fs = require('fs');
const http = require('http');
const path = require('path');

const PORT = 9222;
const PAGE = process.argv[2];
const TAB = Number(process.argv[3] || -1);
const FILTER = process.argv[4] || '';
const CLICK = process.argv[5] || '';
const DELAY = Number(process.argv[6] || 250);

const HOOK = `
(function(){
  window.__rec = [];
  window.__cid = 0;
  window.__idmap = new WeakMap();
  window.__byid = {};
  function cid(c){ if(!c) return -1; var v = window.__idmap.get(c); if(v===undefined){ v = ++window.__cid; window.__idmap.set(c, v); window.__byid[v] = c;} return v; }
  var P = CanvasRenderingContext2D.prototype;
  function snap(ctx, kind, what){
    try{
      var t = ctx.getTransform();
      if (window.__rec.length < 20000) window.__rec.push([
        kind, what,
        +t.a.toFixed(3), +t.b.toFixed(3), +t.c.toFixed(3), +t.d.toFixed(3),
        +t.e.toFixed(1), +t.f.toFixed(1),
        +ctx.globalAlpha.toFixed(3),
        String(ctx.fillStyle).slice(0, 24),
        ctx.canvas && ctx.canvas.width + 'x' + ctx.canvas.height,
        cid(ctx.canvas)
      ]);
    }catch(e){}
  }
  var ft = P.fillText;
  P.fillText = function(t,x,y){ snap(this,'text',String(t)+'@'+Math.round(x)+','+Math.round(y)); return ft.apply(this,arguments); };
  var fr = P.fillRect;
  P.fillRect = function(x,y,w,h){ snap(this,'rect',Math.round(x)+','+Math.round(y)+' '+Math.round(w)+'x'+Math.round(h)); return fr.apply(this,arguments); };
  var pa = P.putImageData;
  P.putImageData = function(img){ snap(this,'putImg',img.width+'x'+img.height); return pa.apply(this,arguments); };
  /* drawImage 也要记：离屏画布（layered window / 位图 DC）只有被 drawImage 合到
   * 主画布上才会出现在屏幕上，缺这一步的症状就是"画了但看不见"。 */
  var di = P.drawImage;
  P.drawImage = function(src){
    var s = (src && src.width) ? (src.width+'x'+src.height+'#'+cid(src)) : '?';
    snap(this,'drawImage','<-'+s);
    return di.apply(this,arguments);
  };
})();
`;

function getJson(url) {
  return new Promise((res, rej) => {
    http
      .get(url, (r) => {
        let d = '';
        r.on('data', (c) => (d += c));
        r.on('end', () => {
          try {
            res(JSON.parse(d));
          } catch (e) {
            rej(e);
          }
        });
      })
      .on('error', rej);
  });
}

(async () => {
  const list = await getJson(`http://127.0.0.1:${PORT}/json/list`);
  const target = list.find((t) => t.type === 'page');
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
    if (m.id && pending.has(m.id)) {
      pending.get(m.id)(m);
      pending.delete(m.id);
    }
  };
  await new Promise((res, rej) => {
    ws.onopen = res;
    ws.onerror = rej;
  });

  await send('Runtime.enable');
  await send('Page.enable');
  await send('Network.enable');
  await send('Network.setCacheDisabled', { cacheDisabled: true });
  await send('Emulation.setDeviceMetricsOverride', { width: 1000, height: 1400, deviceScaleFactor: 1, mobile: false });
  await send('Page.addScriptToEvaluateOnNewDocument', { source: HOOK });
  await send('Page.navigate', { url: PAGE + '?t=' + Date.now() }, 40000);

  const t0 = Date.now();
  let ready = false;
  while (Date.now() - t0 < 120000) {
    await new Promise((r) => setTimeout(r, 1200));
    const r = await send(
      'Runtime.evaluate',
      {
        expression:
          "(function(){var c=document.getElementById('screen'),ld=document.getElementById('loading');return JSON.stringify({cw:c?c.width:0,done:ld?ld.classList.contains('done'):null});})()",
        returnByValue: true,
      },
      15000
    );
    const v = r && r.result && r.result.result ? r.result.result.value : null;
    if (!v) continue;
    const st = JSON.parse(v);
    if (st.done && st.cw > 0) {
      ready = true;
      break;
    }
  }
  if (!ready) {
    console.log('NOT_READY');
    process.exit(3);
  }
  await new Promise((r) => setTimeout(r, 2000));

  const box = await send('Runtime.evaluate', {
    expression:
      "(function(){var c=document.getElementById('screen');var r=c.getBoundingClientRect();return JSON.stringify({x:r.left,y:r.top,w:r.width,h:r.height,cw:c.width,ch:c.height});})()",
    returnByValue: true,
  });
  const b = JSON.parse(box.result.result.value);
  async function clickAt(cx, cy) {
    const sx = b.x + (cx / b.cw) * b.w;
    const sy = b.y + (cy / b.ch) * b.h;
    for (const type of ['mousePressed', 'mouseReleased']) {
      await send('Input.dispatchMouseEvent', { type, x: sx, y: sy, button: 'left', clickCount: 1, buttons: type === 'mousePressed' ? 1 : 0 });
    }
  }

  if (TAB >= 0) {
    const tabs = JSON.parse(fs.readFileSync(path.join(__dirname, '..', '..', 'build', 'cmp', 'canvas2d', 'tabs.json'), 'utf8'));
    await clickAt(tabs[TAB].cx, Number(process.env.TAB_Y || 72));
    await new Promise((r) => setTimeout(r, 1500));
  }

  /* 默认在切标签后清空记录（只看后续交互）；NOCLEAR=1 保留切标签那一刻的
   * 全部绘制 —— 静态页面"哪些行没画出来"必须抓这次绘制。 */
  if (!process.env.NOCLEAR)
    await send('Runtime.evaluate', { expression: 'window.__rec.length = 0' });
  if (CLICK) {
    const [cx, cy] = CLICK.split(',').map(Number);
    await clickAt(cx, cy);
  }
  await new Promise((r) => setTimeout(r, DELAY));

  const r = await send(
    'Runtime.evaluate',
    { expression: 'JSON.stringify(window.__rec)', returnByValue: true },
    30000
  );
  const rec = JSON.parse(r.result.result.value);
  if (process.env.DUMP) {
    fs.writeFileSync(process.env.DUMP, JSON.stringify(rec));
    console.log('DUMP ' + process.env.DUMP + ' n=' + rec.length);
  }
  /* filter 匹配整条记录（文本、坐标、变换、样式、画布尺寸都算），这样既能找
   * "rotate@128,349" 这样的文本调用，也能用 "#0ea5e9" 找某个颜色的矩形填充。 */
  const hits = FILTER ? rec.filter((e) => JSON.stringify(e).indexOf(FILTER) >= 0) : rec;
  console.log('TOTAL ' + rec.length + '  MATCH ' + hits.length + '  (filter=' + FILTER + ')');
  for (const e of hits.slice(0, 40)) console.log(JSON.stringify(e));
  /* 汇总：按 canvas 尺寸分组，看是否出现了新的离屏画布（layered window 会新建 RT） */
  const byCanvas = {};
  rec.forEach((e) => (byCanvas[e[10]] = (byCanvas[e[10]] || 0) + 1));
  console.log('CANVASES ' + JSON.stringify(byCanvas));
  /* EXPR：延时结束后在页面里求值，用来直接读某块离屏画布的像素
   * （window.__byid[id] 是 hook 见过的 canvas，含离屏的）。 */
  if (process.env.EXPR) {
    const er = await send('Runtime.evaluate', { expression: process.env.EXPR, returnByValue: true }, 30000);
    const ev = er && er.result && er.result.result ? er.result.result.value : JSON.stringify(er).slice(0, 300);
    console.log('EXPR ' + (typeof ev === 'string' ? ev : JSON.stringify(ev)));
  }
  ws.close();
  process.exit(0);
})();
