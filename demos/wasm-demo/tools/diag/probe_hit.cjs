/* 探针：确认 Input.dispatchMouseEvent 能不能落到 canvas#screen 上。
 * 打印 body/wrap/canvas 的矩形，挂 pointerdown 计数器，派发一次点击，回报命中情况。
 *
 * 用法：node tools/diag/probe_hit.cjs <cx> <cy>
 *   cx/cy 是 canvas 内坐标
 */
'use strict';
const http = require('http');

const PORT = 9222;
const CX = parseFloat(process.argv[2] || '28');
const CY = parseFloat(process.argv[3] || '70');

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

(async () => {
  const list = await getJson(`http://127.0.0.1:${PORT}/json/list`);
  const target = list.find((t) => t.type === 'page');
  const ws = new WebSocket(target.webSocketDebuggerUrl);
  let id = 0;
  const pending = new Map();
  function send(method, params = {}, ms = 20000) {
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

  const r = await send('Runtime.evaluate', {
    expression: `(function(){
      var c=document.getElementById('screen');
      var wrap=document.getElementById('screen-wrap');
      var lg=document.getElementById('log');
      function rc(e){var r=e.getBoundingClientRect();return {x:r.left,y:r.top,w:r.width,h:r.height};}
      window.__hit={down:0,up:0,move:0,tgt:''};
      if(!window.__hooked){
        window.__hooked=1;
        c.addEventListener('pointerdown',function(e){window.__hit.down++;window.__hit.tgt='canvas';window.__hit.cx=e.clientX;window.__hit.cy=e.clientY;},true);
        window.addEventListener('pointerup',function(e){window.__hit.up++;},true);
      }
      return JSON.stringify({
        dpr: window.devicePixelRatio,
        vw: innerWidth, vh: innerHeight,
        body: rc(document.body), wrap: rc(wrap), canvas: rc(c), log: rc(lg),
        scrollY: scrollY, docH: document.documentElement.scrollHeight
      });
    })()`,
    returnByValue: true,
  });
  const info = JSON.parse(r.result.result.value);
  console.log('LAYOUT ' + JSON.stringify(info));

  const sx = info.canvas.x + (CX / 800) * info.canvas.w;
  const sy = info.canvas.y + (CY / 600) * info.canvas.h;
  console.log('CLICK_AT ' + sx.toFixed(1) + ',' + sy.toFixed(1));

  for (const type of ['mousePressed', 'mouseReleased']) {
    await send('Input.dispatchMouseEvent', {
      type,
      x: sx,
      y: sy,
      button: 'left',
      clickCount: 1,
      buttons: type === 'mousePressed' ? 1 : 0,
    });
  }
  await new Promise((res) => setTimeout(res, 400));
  const h = await send('Runtime.evaluate', { expression: 'JSON.stringify(window.__hit)', returnByValue: true });
  console.log('HIT ' + h.result.result.value);
  ws.close();
  process.exit(0);
})();
