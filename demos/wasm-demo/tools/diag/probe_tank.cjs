/* 坦克大战（Lua 页）动画期帧内开销探针。
 * 干的事：进 Lua 游戏页 → 切到坦克大战 → 点开始 → 采样若干帧，
 * 统计每帧 ops / span / inJs、上屏次数，以及 Canvas API 的方法直方图。
 *
 * 用法: node probe_tank.cjs [outDir] [采样毫秒=3000]
 */
'use strict';
const { chromium } = require('playwright-core');
const http = require('http');
const fs = require('fs');
const path = require('path');

const root = process.argv[2] || path.resolve(__dirname, '..', '..', 'out');
const sampleMs = parseInt(process.argv[3] || '3000', 10);
const mime = { '.html': 'text/html', '.js': 'text/javascript', '.wasm': 'application/wasm', '.data': 'application/octet-stream' };

const server = http.createServer((req, res) => {
  let p = decodeURIComponent(req.url.split('?')[0]);
  if (p === '/') p = '/soui-wasm-demo.html';
  const f = path.join(root, p);
  if (!fs.existsSync(f)) { res.writeHead(404); res.end('nf'); return; }
  res.writeHead(200, { 'Content-Type': mime[path.extname(f)] || 'application/octet-stream' });
  fs.createReadStream(f).pipe(res);
});

(async () => {
  await new Promise((r) => server.listen(0, '127.0.0.1', r));
  const port = server.address().port;
  const browser = await chromium.launch({ headless: true, channel: 'msedge' });
  const page = await browser.newPage({ viewport: { width: 1400, height: 1000 } });
  page.on('pageerror', (e) => console.log('[err]', e.message));
  page.on('console', (m) => { const t = m.text(); if (/svgc/.test(t)) console.log('[c]', t); });

  await page.addInitScript(() => {
    // 与后端无关的流畅度指标：rAF 间隔。cairo 版一帧只有一次 putImageData，
    // 用 Canvas 方法计数看不出东西，但 rAF 间隔直接反映"能不能跟上 60Hz"。
    window.__raf = [];
    window.__rafLog = [];   // [{d, dp}] d=rAF 间隔，dp=这期间上屏次数
    const raf = window.requestAnimationFrame.bind(window);
    let last = 0, lastP = 0;
    window.requestAnimationFrame = (cb) => raf((t) => {
      if (last) {
        window.__raf.push(t - last);
        const p = window.__t ? window.__t.presents : 0;
        window.__rafLog.push({ d: +(t - last).toFixed(1), dp: p - lastP });
        lastP = p;
      }
      last = t;
      return cb(t);
    });
    function install() {
      if (typeof CanvasRenderingContext2D === 'undefined') return false;
      const S = (window.__t = { frames: [], cur: null, hist: Object.create(null), presents: 0 });
      const now = () => performance.now();
      const proto = CanvasRenderingContext2D.prototype;
      const names = Object.getOwnPropertyNames(proto).filter((n) => {
        try { return typeof proto[n] === 'function' && n !== 'constructor'; } catch (e) { return false; }
      });
      const begin = () => { if (!S.cur) S.cur = { ops: 0, inJs: 0, presents: 0, t0: now() }; };
      const end = () => { if (S.cur) { S.cur.t1 = now(); S.frames.push(S.cur); S.cur = null; } };
      for (const n of names) {
        const orig = proto[n];
        proto[n] = function (...a) {
          begin();
          const t = now();
          const r = orig.apply(this, a);
          S.cur.inJs += now() - t;
          S.cur.ops++;
          S.hist[n] = (S.hist[n] || 0) + 1;
          if (n === 'putImageData') { S.cur.presents++; S.presents++; end(); }
          // canvas2d 后端的上屏是 drawImage(离屏画布 → 显示画布)，接收者是
          // window.__swxDisplay；把它也当作帧边界。
          else if (n === 'drawImage' && this === window.__swxDisplay) { S.cur.presents++; S.presents++; end(); }
          return r;
        };
      }
      return true;
    }
    let tries = 0;
    const iv = setInterval(() => { if (install() || ++tries > 500) clearInterval(iv); }, 5);
  });

  await page.goto(`http://127.0.0.1:${port}/soui-wasm-demo.html`, { timeout: 30000 });
  await page.waitForFunction(() => document.querySelector('canvas') && document.querySelector('canvas').width > 100, { timeout: 30000 });
  await page.waitForTimeout(1800);

  const box = await page.evaluate(() => {
    const c = document.getElementById('screen');
    const r = c.getBoundingClientRect();
    return { x: r.x, y: r.y, w: r.width, h: r.height, cw: c.width, ch: c.height };
  });
  const click = async (cx, cy) => {
    await page.mouse.move(box.x + cx * (box.w / box.cw), box.y + cy * (box.h / box.ch));
    await page.mouse.down();
    await page.mouse.up();
    await page.waitForTimeout(500);
  };

  await click(593, 72);   // 「Lua 游戏」页签
  await click(72, 218);   // 「坦克大战」内页签
  await click(196, 505);  // 「开始游戏」
  await page.waitForTimeout(900); // 跳过开局那一帧

  /* 每段开始前清零并记起点：fps 的分母必须是这一段真实经过的时间。
   * 之前 fire 段只按了 6 次空格（约 1.1s），却用 3000ms 当分母，算出来 24fps，
   * 而 rafP95 只有 16.8ms —— 那是分母错，不是掉帧。 */
  const reset = () => page.evaluate(() => {
    const S = window.__t;
    S.frames.length = 0; S.presents = 0; S.cur = null;
    S.t0 = performance.now();
  });

  const snap = () => page.evaluate((ms) => {
    const S = window.__t;
    const f = S.frames;
    const med = (a) => { const s = a.slice().sort((x, y) => x - y); return s.length ? s[s.length >> 1] : 0; };
    const pct = (a, p) => { const s = a.slice().sort((x, y) => x - y); return s.length ? s[Math.min(s.length - 1, Math.floor(s.length * p))] : 0; };
    const span = f.map((x) => x.t1 - x.t0);
    const ops = f.map((x) => x.ops);
    const dur = performance.now() - (S.t0 || performance.now());
    const r = {
      frames: f.length,
      ms: Math.round(dur),
      fps: +(f.length / (dur / 1000)).toFixed(1),
      rafMed: +med(window.__raf || []).toFixed(1),
      rafP95: +pct(window.__raf || [], 0.95).toFixed(1),
      opsMed: med(ops), opsP95: pct(ops, 0.95), opsMax: Math.max(0, ...ops),
      spanMed: +med(span).toFixed(2), spanP95: +pct(span, 0.95).toFixed(2),
      inJsMed: +med(f.map((x) => x.inJs)).toFixed(2),
      presents: S.presents
    };
    // 最慢的 6 个 rAF 间隔 + 那期间上屏几次：dp=0 说明时间没花在绘制上
    r.slowTicks = (window.__rafLog || []).slice().sort((a, b) => b.d - a.d).slice(0, 6);
    window.__rafLog = [];
    window.__raf = [];
    // 阶段结束后清零，供下一段复用
    S.frames.length = 0; S.presents = 0; S.cur = null;
    return r;
  }, sampleMs);

  const phases = {};
  // 段 1：静止（只有定时器在跑）
  await reset();
  await page.waitForTimeout(sampleMs);
  phases.idle = await snap();

  // 段 2：按住上方向键移动自己的坦克 —— 用户说这一段最卡
  await reset();
  await page.keyboard.down('ArrowUp');
  await page.waitForTimeout(sampleMs);
  await page.keyboard.up('ArrowUp');
  phases.moveUp = await snap();

  // 段 3：横向移动
  await reset();
  await page.keyboard.down('ArrowLeft');
  await page.waitForTimeout(sampleMs);
  await page.keyboard.up('ArrowLeft');
  phases.moveLeft = await snap();

  // 段 4：连射
  await reset();
  for (let i = 0; i < 6; ++i) { await page.keyboard.press('Space'); await page.waitForTimeout(180); }
  phases.fire = await snap();

  const res = { phases };
  res.methods = await page.evaluate(() =>
    Object.entries(window.__t.hist).sort((a, b) => b[1] - a[1]).slice(0, 16).map(([k, v]) => k + ':' + v));
  console.log(JSON.stringify(res));

  console.log(JSON.stringify(res));
  const el = await page.$('#screen');
  await el.screenshot({ path: path.join(__dirname, 'tank_play.png') });
  await browser.close();
  server.close();
})();
