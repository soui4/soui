/* 探测 canvas 上下文的「状态栈」健康度：save / restore / clip 的收支。
 *
 * canvas 的 clip() 一旦建立就无法撤销（只能靠 restore 弹出）。如果后端里
 * ctx.save() 与 ctx.restore() 不配对，clip 会一帧一帧地累积叠加，与上一帧的
 * 裁剪区求交 —— 画到第 N 帧时可用区域已经被啃成空，症状正是"多刷新几次就
 * 什么都不显示 / 文字消失"。
 *
 * 用法：
 *   node tools/diag/probe_state.cjs <url> <tabIndex|-> [step ...]
 * step 与 interact.cjs 一致：click:X,Y  down/move/up:X,Y  wait:MS  shot:NAME  eval:JS
 * 额外有 stat:LABEL —— 打一条「当前计数快照」，用来对比两步之间的增量。
 *
 * 例（模态表单：打开表单 → 快照 → 鼠标移动 → 快照）：
 *   node tools/diag/probe_state.cjs http://127.0.0.1:8125/soui-wasm-demo.html 13 \
 *     click:37,164 wait:400 stat:before move:300,300 wait:400 stat:after move:420,330 wait:400 stat:end
 */
'use strict';
const fs = require('fs');
const http = require('http');
const path = require('path');

const PORT = Number(process.env.CDP_PORT || 9222);
const PAGE = process.argv[2];
if (!PAGE) {
  console.error('usage: probe_state.cjs <url> <tabIndex|-> [steps...]');
  process.exit(1);
}
const TAB = process.argv[3] === '-' ? -1 : Number(process.argv[3]);
const STEPS = process.argv.slice(4);
const OUTDIR = process.env.LIVE_DIR || path.join(__dirname, '..', '..', 'build', 'cmp', 'live');
if (STEPS.some((s) => s.startsWith('shot:'))) fs.mkdirSync(OUTDIR, { recursive: true });

const HOOK = `
(function(){
  var P = CanvasRenderingContext2D.prototype;
  window.__st = { seq:0, save:0, restore:0, clip:0, events:[] };
  window.__cid = 0; window.__idmap = new WeakMap(); window.__byid = {};
  function cid(c){ if(!c) return -1; var v = window.__idmap.get(c); if(v===undefined){ v = ++window.__cid; window.__idmap.set(c, v); window.__byid[v] = c; } return v; }
  function note(kind, what, ctx){
    var s = window.__st; s.seq++;
    if (kind === 'save') s.save++;
    if (kind === 'restore') s.restore++;
    if (kind === 'clip') s.clip++;
    /* 只留 clip / 大范围 fill / drawImage 这三类，避免事件表爆掉。
     * 每条带上「目标画布 id + 尺寸 + 当前变换 + globalAlpha」——只看参数无法判断
     * 这次绘制落到了主画布还是某个小 RT 上，也就分不清"画错位置"和"贴回去时错了"。 */
    if (kind === 'clip' || kind === 'drawImage' || kind === 'wipe') {
      var tf = '';
      try { var t = ctx.getTransform(); tf = [t.a,t.b,t.c,t.d,+t.e.toFixed(1),+t.f.toFixed(1)].join(','); } catch(e){}
      var cn = ctx && ctx.canvas;
      if (s.events.length < 6000)
        s.events.push([s.seq, kind, what, cn ? cid(cn) : -1, cn ? cn.width + 'x' + cn.height : '?', tf, ctx ? +ctx.globalAlpha.toFixed(3) : -1]);
    }
  }
  function wrap(name, kind){
    var f = P[name];
    P[name] = function(){
      note(kind, Array.prototype.slice.call(arguments).map(function(a){ return (typeof a === 'object' && a && a.width) ? ('<'+a.width+'x'+a.height+'#'+cid(a)+'>') : String(a); }).join(','), this);
      return f.apply(this, arguments);
    };
  }
  wrap('save','save');
  wrap('restore','restore');
  wrap('clip','clip');
  wrap('drawImage','drawImage');
  /* 主画布被 resize 会清空像素并重置全部状态（含 clip）——这是"状态泄漏"的天然解药，
   * 所以要把每次 resize 也记下来，否则会误判。 */
  var W = Object.getOwnPropertyDescriptor(HTMLCanvasElement.prototype, 'width');
  var H = Object.getOwnPropertyDescriptor(HTMLCanvasElement.prototype, 'height');
  function wipe(kind, v, el){
    var st = window.__st;
    st.seq++;
    if (st.events.length < 6000) {
      /* 只留前若干条调用栈：谁把画布尺寸改成 1x1，一眼就看出来 */
      var where = '';
      if (st.events.filter(function(e){ return e[1] === 'wipe'; }).length < 8) {
        var lines = String(new Error().stack || '').split('\\n');
        where = lines.slice(2, 5).map(function(s){ return s.trim().replace(/^at\s+/, '').slice(0, 90); }).join(' | ');
      }
      st.events.push([st.seq, 'wipe', kind + '=' + v + '#' + cid(el), cid(el), el.width + 'x' + el.height, where, 0]);
    }
  }
  Object.defineProperty(HTMLCanvasElement.prototype, 'width', {
    get: W.get, configurable: true,
    set: function(v){ wipe('w', v, this); return W.set.call(this, v); }
  });
  Object.defineProperty(HTMLCanvasElement.prototype, 'height', {
    get: H.get, configurable: true,
    set: function(v){ wipe('h', v, this); return H.set.call(this, v); }
  });
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
  if (!target) {
    console.error('no page target');
    process.exit(2);
  }
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
      return;
    }
    /* CONSOLE=1：把页面 console.log 原样打出来。wasm 侧 printf 走这里，
     * 是 SWINX_SC_TRACE / SWINX_GDI_TRACE 这类 C++ 诊断的唯一出口。 */
    if (process.env.CONSOLE && m.method === 'Runtime.consoleAPICalled') {
      const txt = (m.params.args || []).map((a) => (a.value !== undefined ? a.value : a.description || '')).join(' ');
      if (txt.indexOf(process.env.CONSOLE) >= 0 || process.env.CONSOLE === '1') console.log('[page] ' + txt);
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
  await send('Emulation.setDeviceMetricsOverride', { width: 1000, height: 1400, deviceScaleFactor: parseFloat(process.env.DSF || '1'), mobile: false });
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
      "(function(){var c=document.getElementById('screen');var r=c.getBoundingClientRect();var lw,lh;try{lw=Module._SwinxFbLogicalWidth();lh=Module._SwinxFbLogicalHeight();}catch(e){lw=c.width;lh=c.height;}return JSON.stringify({x:r.left,y:r.top,w:r.width,h:r.height,cw:c.width,ch:c.height,lw:lw,lh:lh});})()",
    returnByValue: true,
  });
  const b = JSON.parse(box.result.result.value);

  /* cx/cy 是逻辑坐标：CSS 偏移 = 逻辑 ×(CSS宽/逻辑宽)。之前用物理宽当除数，
   * dpr≠1 时所有点击/移动等比缩小错位（与 interact.cjs 同一处坑）。 */
  async function clickAt(cx, cy) {
    const sx = b.x + (cx / b.lw) * b.w;
    const sy = b.y + (cy / b.lh) * b.h;
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
  }
  async function moveTo(cx, cy) {
    const sx = b.x + (cx / b.lw) * b.w;
    const sy = b.y + (cy / b.lh) * b.h;
    await send('Input.dispatchMouseEvent', { type: 'mouseMoved', x: sx, y: sy, button: 'none', buttons: 0 });
  }
  async function shoot(name) {    const du = await send(
      'Runtime.evaluate',
      {
        expression:
          "(function(){var c=document.getElementById('screen');try{return c.toDataURL('image/png');}catch(e){return 'ERR:'+e.message;}})()",
        returnByValue: true,
      },
      60000
    );
    const val = du && du.result && du.result.result ? du.result.result.value : undefined;
    if (typeof val === 'string' && val.startsWith('data:image/png;base64,')) {
      const buf = Buffer.from(val.split(',')[1], 'base64');
      const fp = path.join(OUTDIR, name + '.png');
      fs.writeFileSync(fp, buf);
      console.log('WROTE ' + fp + ' ' + buf.length);
    } else console.log('NO_DATA ' + name);
  }
  /* 把页面上所有大尺寸画布各存一张 png（文件名带 hook 分配的 canvas id）。
   * 窗口 canvas 与 SOUI 的 memRT 都是 800x600 左右，只看最终 screen 分不清
   * "谁画的、画到哪"，一比就知道内容断在哪一级。 */
  async function dumpBig(prefix) {
    const r = await send(
      'Runtime.evaluate',
      {
        expression:
          "(function(){var out=[];Object.keys(window.__byid).forEach(function(k){var c=window.__byid[k];if(!c||c.width<600)return;try{out.push([+k,c.toDataURL('image/png')]);}catch(e){}});return JSON.stringify(out);})()",
        returnByValue: true,
      },
      60000
    );
    const v = r && r.result && r.result.result ? r.result.result.value : null;
    if (!v) {
      console.log('NO_DATA ' + prefix);
      return;
    }
    for (const [cid, du] of JSON.parse(v)) {
      if (typeof du !== 'string' || !du.startsWith('data:image/png;base64,')) continue;
      const buf = Buffer.from(du.split(',')[1], 'base64');
      const fp = path.join(OUTDIR, prefix + '_c' + cid + '.png');
      fs.writeFileSync(fp, buf);
      console.log('WROTE ' + fp + ' ' + buf.length);
    }
  }

  async function stat(label) {
    const r = await send('Runtime.evaluate', {
      expression: 'JSON.stringify({s:window.__st.save,r:window.__st.restore,c:window.__st.clip,seq:window.__st.seq})',
      returnByValue: true,
    });
    const v = r && r.result && r.result.result ? r.result.result.value : '{}';
    const o = JSON.parse(v);
    console.log(
      'STAT ' + label.padEnd(10) + ' save=' + o.s + ' restore=' + o.r +
        '  depth=' + (o.s - o.r) + '  clip=' + o.c + '  ops=' + o.seq
    );
  }

  if (TAB >= 0) {
    const tabs = JSON.parse(fs.readFileSync(path.join(__dirname, '..', '..', 'build', 'cmp', 'canvas2d', 'tabs.json'), 'utf8'));
    await clickAt(tabs[TAB].cx, Number(process.env.TAB_Y || 72));
    await new Promise((r) => setTimeout(r, 1500));
  }

  for (const step of STEPS) {
    const i = step.indexOf(':');
    const op = i < 0 ? step : step.slice(0, i);
    const arg = i < 0 ? '' : step.slice(i + 1);
    if (op === 'click') {
      const [x, y] = arg.split(',').map(Number);
      console.log('CLICK ' + x + ',' + y);
      await clickAt(x, y);
    } else if (op === 'move') {
      const [x, y] = arg.split(',').map(Number);
      console.log('MOVE ' + x + ',' + y);
      await moveTo(x, y);
    } else if (op === 'wait') {
      await new Promise((r) => setTimeout(r, Number(arg) || 0));
    } else if (op === 'shot') {
      await shoot(arg);
    } else if (op === 'stat') {
      await stat(arg || 'x');
    } else if (op === 'dumpbig') {
      await dumpBig(arg || 'big');
    } else if (op === 'eval') {
      const r = await send('Runtime.evaluate', { expression: arg, returnByValue: true, awaitPromise: true });
      const v = r && r.result && r.result.result ? r.result.result.value : JSON.stringify(r).slice(0, 300);
      console.log('EVAL ' + (typeof v === 'string' ? v : JSON.stringify(v)));
    } else console.log('SKIP ' + step);
  }

  if (process.env.DUMP) {
    const r = await send('Runtime.evaluate', { expression: 'JSON.stringify(window.__st.events)', returnByValue: true }, 30000);
    fs.writeFileSync(process.env.DUMP, r.result.result.value);
    console.log('DUMP ' + process.env.DUMP);
  }
  ws.close();
  process.exit(0);
})();
