/* 运行时 clip 栈取证：不依赖颜色/游戏状态，直接验证 canvas2d 后端的
 * clip 语义是否被 C2DStateCache 的 "same rect -> skip" 与 per-blit
 * SAVE/RESTORE 缺失破坏。
 *
 * ClearRect() 的实现是 save -> clip(lprc) -> fillRect(copy) -> restore。
 * fillRect 受「当前累积 clip」限制：如果此刻设备 clip 已经是残留的窄条，
 * 这次擦除只能清掉窄条内的像素，旧动画位置就留在屏幕上（飞分残影）。
 *
 * 本脚本 hook CanvasRenderingContext2D 原型，自己维护一份 clip 栈镜像：
 *   save/restore 压弹栈，beginPath/rect/clip 求交，
 *   在 clearRect 与 fillRect 时判断「有效 clip 是否把请求矩形截断」。
 *
 * 用法: SWX_OUT=out.canvas2d.before SWX_TAG=before DSF=1.25 node _cliptrace.cjs 2400
 */
'use strict';
const { chromium } = require('playwright-core');
const http = require('http');
const fs = require('fs');
const path = require('path');

const root = path.resolve(__dirname, '..', '..', process.env.SWX_OUT || 'out');
const N = parseInt(process.argv[2] || '2400', 10);
const TAG = process.env.SWX_TAG || 'x';
const mime = { '.html': 'text/html', '.js': 'text/javascript', '.wasm': 'application/wasm', '.data': 'application/octet-stream' };
const server = http.createServer((req, res) => {
  let p = decodeURIComponent(req.url.split('?')[0]);
  if (p === '/') p = '/soui-wasm-demo.html';
  const f = path.join(root, p);
  if (!fs.existsSync(f)) { res.writeHead(404); res.end('nf'); return; }
  res.writeHead(200, { 'Content-Type': mime[path.extname(f)] || 'application/octet-stream' });
  fs.createReadStream(f).pipe(res);
});

const INIT = () => {
  const P = CanvasRenderingContext2D.prototype;
  const T = {
    ctxs: [],
    saves: 0, restores: 0, clips: 0, clears: 0, fills: 0,
    /* fillRect/clearRect 请求矩形被有效 clip 截掉一半以上 */
    truncatedClear: 0, truncatedFill: 0,
    /* 更严格：请求宽度 >100，有效 clip 宽度 <20 —— 整窗擦除被窄条截断 */
    narrowClear: 0, narrowFill: 0,
    zeroClip: 0, maxDepth: 0, minClipW: 1e9,
    samples: [],
  };
  globalThis.__clipTrace = T;
  const st = new WeakMap();
  const get = (ctx) => {
    let s = st.get(ctx);
    if (!s) {
      s = { stack: [], depth: 0, clip: null, pending: null };
      st.set(ctx, s);
      T.ctxs.push(ctx);
    }
    return s;
  };
  const inter = (a, b) => {
    if (!a) return b ? { l: b.l, t: b.t, r: b.r, b: b.b } : null;
    if (!b) return { l: a.l, t: a.t, r: a.r, b: a.b };
    return { l: Math.max(a.l, b.l), t: Math.max(a.t, b.t), r: Math.min(a.r, b.r), b: Math.min(a.b, b.b) };
  };
  const area = (x) => Math.max(0, x.r - x.l) * Math.max(0, x.b - x.t);
  const check = (s, x, y, w, h, isClear) => {
    const req = { l: x, t: y, r: x + w, b: y + h };
    const eff = s.clip ? inter(s.clip, req) : req;
    const a = area(eff), ra = area(req);
    if (isClear) T.clears++; else T.fills++;
    if (s.clip) {
      const cw = s.clip.r - s.clip.l, ch = s.clip.b - s.clip.t;
      if (cw < T.minClipW) T.minClipW = cw;
      if (cw <= 0 || ch <= 0) T.zeroClip++;
    }
    if (ra > 0 && a < ra * 0.5) {
      if (isClear) T.truncatedClear++; else T.truncatedFill++;
    }
    if (w > 100 && s.clip && (s.clip.r - s.clip.l) < 20) {
      if (isClear) T.narrowClear++; else T.narrowFill++;
      if (T.samples.length < 6) T.samples.push({ op: isClear ? 'clear' : 'fill', req: [x, y, w, h], clip: [s.clip.l, s.clip.t, s.clip.r, s.clip.b], depth: s.depth });
    }
  };
  const oSave = P.save, oRestore = P.restore, oBegin = P.beginPath, oRect = P.rect;
  const oClip = P.clip, oClear = P.clearRect, oFill = P.fillRect, oRound = P.roundRect;
  P.save = function () { const s = get(this); s.stack.push(s.clip); s.depth++; T.saves++; if (s.depth > T.maxDepth) T.maxDepth = s.depth; return oSave.apply(this, arguments); };
  P.restore = function () { const s = get(this); if (s.stack.length) s.clip = s.stack.pop(); s.depth--; T.restores++; return oRestore.apply(this, arguments); };
  P.beginPath = function () { get(this).pending = null; return oBegin.apply(this, arguments); };
  P.rect = function (x, y, w, h) { const s = get(this); const r = { l: x, t: y, r: x + w, b: y + h }; s.pending = s.pending ? { l: Math.min(s.pending.l, r.l), t: Math.min(s.pending.t, r.t), r: Math.max(s.pending.r, r.r), b: Math.max(s.pending.b, r.b) } : r; return oRect.apply(this, arguments); };
  P.roundRect = function (x, y, w, h) { const s = get(this); const r = { l: x, t: y, r: x + w, b: y + h }; s.pending = s.pending ? { l: Math.min(s.pending.l, r.l), t: Math.min(s.pending.t, r.t), r: Math.max(s.pending.r, r.r), b: Math.max(s.pending.b, r.b) } : r; return oRound.apply(this, arguments); };
  P.clip = function () { const s = get(this); T.clips++; s.clip = inter(s.clip, s.pending); return oClip.apply(this, arguments); };
  P.clearRect = function (x, y, w, h) { check(get(this), x, y, w, h, true); return oClear.apply(this, arguments); };
  P.fillRect = function (x, y, w, h) { check(get(this), x, y, w, h, false); return oFill.apply(this, arguments); };
};

(async () => {
  await new Promise((r) => server.listen(0, '127.0.0.1', r));
  const port = server.address().port;
  const browser = await chromium.launch({
    headless: false, channel: 'msedge',
    args: ['--window-position=40,40', '--window-size=1440,1040'],
  });
  const page = await browser.newPage({ viewport: { width: 1400, height: 1000 }, deviceScaleFactor: parseFloat(process.env.DSF || '1.25') });
  await page.addInitScript(INIT);
  await page.goto(`http://127.0.0.1:${port}/soui-wasm-demo.html${process.env.SWX_QS || ''}`, { timeout: 30000 });
  await page.waitForFunction(() => document.querySelector('canvas') && document.querySelector('canvas').width > 100, { timeout: 30000 });
  await page.waitForTimeout(1500);
  const box = await page.evaluate(() => {
    const c = document.getElementById('screen');
    const r = c.getBoundingClientRect();
    const lw = Module._SwinxFbLogicalWidth ? Module._SwinxFbLogicalWidth() : c.width;
    const lh = Module._SwinxFbLogicalHeight ? Module._SwinxFbLogicalHeight() : c.height;
    return { x: r.x, y: r.y, w: r.width, h: r.height, cw: lw, ch: lh };
  });
  const click = async (cx, cy) => {
    const x = box.x + cx * (box.w / box.cw), y = box.y + cy * (box.h / box.ch);
    await page.mouse.move(x, y); await page.mouse.down(); await page.mouse.up();
    await page.waitForTimeout(450);
  };
  for (const [x, y] of [[572, 70], [61, 194], [220, 501]]) await click(x, y);
  await page.waitForTimeout(600);
  /* 清零，只统计进入游戏页之后的稳定窗口 */
  await page.evaluate(() => { const T = globalThis.__clipTrace; for (const k of ['saves','restores','clips','clears','fills','truncatedClear','truncatedFill','narrowClear','narrowFill','zeroClip','maxDepth']) T[k] = 0; T.minClipW = 1e9; T.samples.length = 0; });
  const res = await page.evaluate(async (n) => {
    for (let i = 0; i < n; i++) await new Promise((r) => requestAnimationFrame(r));
    const T = globalThis.__clipTrace;
    return { saves: T.saves, restores: T.restores, clips: T.clips, clears: T.clears, fills: T.fills,
             truncatedClear: T.truncatedClear, truncatedFill: T.truncatedFill,
             narrowClear: T.narrowClear, narrowFill: T.narrowFill, zeroClip: T.zeroClip,
             maxDepth: T.maxDepth, minClipW: T.minClipW === 1e9 ? null : T.minClipW,
             ctxCount: T.ctxs.length, samples: T.samples };
  }, N);
  console.log('==== ' + TAG + ' (' + N + ' frames) ====');
  console.log(JSON.stringify(res, null, 2));
  fs.writeFileSync(path.join(__dirname, '_cliptrace_' + TAG + '.json'), JSON.stringify(res, null, 2));
  await browser.close();
  server.close();
})();
