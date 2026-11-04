/* 长跑探针：坦克大战跑起来后每隔一段时间采样，看「随游戏时长增长」的量。
 *
 * 关注点（canvas2d 后端「越玩越卡」的嫌疑）：
 *   - __swxCtx.length   JS 侧 ctx 注册表长度（只增不减 ⇒ 画布/上下文泄漏）
 *   - canvases          document.createElement('canvas') 累计次数（每帧新建多少块）
 *   - heap              usedJSHeapSize（MB）
 *   - fps / span / ops  每帧帧率、帧内 Canvas 调用耗时、调用次数
 *
 * 用法: node probe_growth.cjs [outDir] [总时长ms=30000] [采样间隔ms=3000]
 */
'use strict';
const { chromium } = require('playwright-core');
const http = require('http');
const fs = require('fs');
const path = require('path');

const root = process.argv[2] || path.resolve(__dirname, '..', '..', 'out');
const totalMs = parseInt(process.argv[3] || '30000', 10);
const stepMs = parseInt(process.argv[4] || '3000', 10);
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

  await page.addInitScript(() => {
    /* 1) 统计 canvas 元素创建次数 + 尺寸分布。
     * SwinxCanvasCreate 是「先设 width/height 再 getContext」，所以在 getContext
     * 时能读到真实尺寸：1x1 是 HDC__ 的占位画布，其余尺寸是位图/窗口后备画布。 */
    const ce = document.createElement.bind(document);
    window.__canvases = 0;
    window.__canvasSizes = Object.create(null);
    window.__pix = 0;
    document.createElement = function (tag, opts) {
      if (String(tag).toLowerCase() === 'canvas') window.__canvases++;
      return ce(tag, opts);
    };
    const gc = HTMLCanvasElement.prototype.getContext;
    HTMLCanvasElement.prototype.getContext = function (kind, opts) {
      if (kind === '2d' && !this.__swxCounted) {
        this.__swxCounted = true;
        const k = this.width + 'x' + this.height;
        window.__canvasSizes[k] = (window.__canvasSizes[k] || 0) + 1;
        window.__pix += this.width * this.height;
      }
      return gc.call(this, kind, opts);
    };
    /* 2) 帧内 Canvas 调用采样（同 probe_tank.cjs） */
    window.__raf = [];
    const raf = window.requestAnimationFrame.bind(window);
    let last = 0;
    window.requestAnimationFrame = (cb) => raf((t) => {
      if (last) window.__raf.push(t - last);
      last = t;
      return cb(t);
    });
    function install() {
      if (typeof CanvasRenderingContext2D === 'undefined') return false;
      const S = (window.__t = { frames: [], cur: null, presents: 0 });
      const now = () => performance.now();
      const proto = CanvasRenderingContext2D.prototype;
      const names = Object.getOwnPropertyNames(proto).filter((n) => {
        try { return typeof proto[n] === 'function' && n !== 'constructor'; } catch (e) { return false; }
      });
      const begin = () => { if (!S.cur) S.cur = { ops: 0, inJs: 0, t0: now() }; };
      const end = () => { if (S.cur) { S.cur.t1 = now(); S.frames.push(S.cur); S.cur = null; } };
      for (const n of names) {
        const orig = proto[n];
        proto[n] = function (...a) {
          begin();
          const t = now();
          const r = orig.apply(this, a);
          S.cur.inJs += now() - t;
          S.cur.ops++;
          if (n === 'putImageData') { S.cur.presents++; S.presents++; end(); }
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
  await page.waitForTimeout(600);

  // 让角色持续动起来：轮流按方向键 + 开火，保持游戏在高频重绘状态（最接近用户描述的场景）
  const seq = ['ArrowUp', 'ArrowRight', 'ArrowDown', 'ArrowLeft'];
  let keyIdx = 0;
  let keyTimer = setInterval(() => {
    const prev = seq[(keyIdx + 3) % 4];
    const cur = seq[keyIdx % 4];
    page.keyboard.up(prev).catch(() => {});
    page.keyboard.down(cur).catch(() => {});
    keyIdx++;
  }, 700);
  await page.keyboard.down('ArrowUp');

  const rows = [];
  const t0 = Date.now();
  while (Date.now() - t0 < totalMs) {
    const r = await page.evaluate(() => {
      const S = window.__t;
      const f = S.frames;
      const med = (a) => { const s = a.slice().sort((x, y) => x - y); return s.length ? s[s.length >> 1] : 0; };
      const span = f.map((x) => x.t1 - x.t0);
      const ops = f.map((x) => x.ops);
      /* 尺寸分布里除 1x1 之外的部分 = 真正占显存档的位图/窗口画布 */
      const sizes = Object.entries(window.__canvasSizes)
        .sort((a, b) => b[1] - a[1]).slice(0, 4).map(([k, v]) => k + '*' + v).join(' ');
      const out = {
        t: Math.round(performance.now()),
        swxCtx: window.__swxCtx ? window.__swxCtx.length : -1,
        canvases: window.__canvases,
        pixMB: +(window.__pix * 4 / 1048576).toFixed(1),
        sizes,
        heapMB: performance.memory ? +(performance.memory.usedJSHeapSize / 1048576).toFixed(1) : -1,
        frames: f.length,
        fps: 0,
        spanMed: +med(span).toFixed(2),
        opsMed: med(ops),
        rafMed: +med(window.__raf).toFixed(1),
      };
      const t1 = f.length ? f[f.length - 1].t1 : performance.now();
      out.fps = +((f.length * 1000) / Math.max(1, t1 - (f[0] ? f[0].t0 : t1))).toFixed(1);
      out.usPerOp = +(med(span) * 1000 / Math.max(1, med(ops))).toFixed(2);
      S.frames.length = 0; S.cur = null; window.__raf.length = 0;
      return out;
    });
    rows.push(r);
    await page.waitForTimeout(stepMs);
  }
  clearInterval(keyTimer);

  console.log('t(ms)  swxCtx  canvases  pixMB  heap(MB)  fps   spanMed  opsMed  us/op  rafMed  topSizes');
  for (const r of rows) {
    console.log(
      String(r.t).padStart(6),
      String(r.swxCtx).padStart(7),
      String(r.canvases).padStart(9),
      String(r.pixMB).padStart(6),
      String(r.heapMB).padStart(9),
      String(r.fps).padStart(5),
      String(r.spanMed).padStart(8),
      String(r.opsMed).padStart(7),
      String(r.usPerOp).padStart(6),
      String(r.rafMed).padStart(7),
      ' ' + (r.sizes || '')
    );
  }
  if (rows.length > 1) {
    const a = rows[0], b = rows[rows.length - 1];
    const dt = (b.t - a.t) / 1000;
    console.log(`\nΔcanvases=${b.canvases - a.canvases} over ${dt.toFixed(1)}s => ${((b.canvases - a.canvases) / dt).toFixed(0)} canvases/s`);
    console.log(`ΔswxCtx=${b.swxCtx - a.swxCtx} over ${dt.toFixed(1)}s => ${((b.swxCtx - a.swxCtx) / dt).toFixed(0)} ids/s`);
    console.log(`Δheap=${(b.heapMB - a.heapMB).toFixed(1)}MB  fps ${a.fps} -> ${b.fps}`);
  }
  await browser.close();
  server.close();
})();
