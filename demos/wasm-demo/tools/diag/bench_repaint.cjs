/* 确定性重绘基准：在两个页签之间来回点，每次切页都是一次整窗重绘。
 * 统计这些"整窗重绘帧"的 Canvas 2D 调用数与绘制跨度（span），用于跨版本对比。
 * 用法: node bench_repaint.cjs <outDir> [轮数=10] [页签x,页签x=140,200]
 */
const { chromium } = require('playwright-core');
const http = require('http');
const fs = require('fs');
const path = require('path');

const root = process.argv[2] || path.resolve(__dirname, '..', '..', 'out');
const rounds = parseInt(process.argv[3] || '10', 10);
const tabs = (process.argv[4] || '140,200').split(',').map(Number);
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
  await new Promise(r => server.listen(0, '127.0.0.1', r));
  const port = server.address().port;
  const browser = await chromium.launch({ headless: true, channel: 'msedge' });
  const page = await browser.newPage({ viewport: { width: 1400, height: 1000 } });
  page.on('pageerror', e => console.log('[err]', e.message));
  page.on('console', m => { if (m.type() === 'error') console.log('[c]', m.text()); });

  await page.addInitScript(() => {
    function install() {
      if (typeof CanvasRenderingContext2D === 'undefined') return false;
      const S = window.__b = { frames: [], cur: null };
      const now = () => performance.now();
      const proto = CanvasRenderingContext2D.prototype;
      const names = Object.getOwnPropertyNames(proto).filter(n => {
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
          if (n === 'putImageData') end();
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
  await page.waitForTimeout(1500);

  const box = await page.evaluate(() => {
    const r = document.querySelector('canvas').getBoundingClientRect();
    const c = document.querySelector('canvas');
    return { x: r.x, y: r.y, w: r.width, h: r.height, cw: c.width, ch: c.height };
  });
  const click = async (cx, cy) => {
    await page.mouse.move(box.x + cx * (box.w / box.cw), box.y + cy * (box.h / box.ch));
    await page.mouse.down();
    await page.mouse.up();
  };

  // 预热（第一遍有 JIT/字体缓存，丢掉）
  for (let i = 0; i < 2; ++i) { await click(tabs[i % tabs.length], 72); await page.waitForTimeout(300); }
  await page.evaluate(() => { window.__b.frames.length = 0; window.__b.cur = null; });

  for (let i = 0; i < rounds; ++i) {
    await click(tabs[i % tabs.length], 72);
    await page.waitForTimeout(250);
  }
  await page.waitForTimeout(300);

  const res = await page.evaluate(() => {
    const f = window.__b.frames;
    const big = f.filter(x => x.ops >= 500);
    const span = arr => arr.map(x => x.t1 - x.t0);
    const sum = a => a.reduce((s, v) => s + v, 0);
    const med = a => { const s = a.slice().sort((x, y) => x - y); return s.length ? s[s.length >> 1] : 0; };
    // 一轮点击偶尔会多出/少掉一两帧（按下、抬起、切页动画各可能来一帧），平均 ops
    // 就会被这些"重帧"带偏 ⇒ 不同次、不同版本之间比的不是同一件事。取 ops 出现
    // 次数最多的那一档（mode），只在 ops 完全相同的帧之间比 span/inJs，才可比。
    const hist = new Map();
    for (const x of big) hist.set(x.ops, (hist.get(x.ops) || 0) + 1);
    let modeOps = 0, modeN = 0;
    for (const [k, v] of hist) if (v > modeN || (v === modeN && k > modeOps)) { modeOps = k; modeN = v; }
    const same = big.filter(x => x.ops === modeOps);
    return {
      frames: f.length,
      bigN: big.length,
      bigOps: big.length ? +(sum(big.map(x => x.ops)) / big.length).toFixed(1) : 0,
      bigOpsTotal: big.reduce((s, x) => s + x.ops, 0),
      bigSpanMed: +med(span(big)).toFixed(2),
      bigSpanSum: +sum(span(big)).toFixed(1),
      bigInJsMed: +med(big.map(x => x.inJs)).toFixed(2),
      bigInJsSum: +sum(big.map(x => x.inJs)).toFixed(1),
      allOps: f.reduce((s, x) => s + x.ops, 0),
      sameOps: modeOps,
      sameN: same.length,
      sameSpanMed: +med(span(same)).toFixed(2),
      sameInJsMed: +med(same.map(x => x.inJs)).toFixed(2)
    };
  });
  console.log(JSON.stringify(res));
  await browser.close();
  server.close();
})();
