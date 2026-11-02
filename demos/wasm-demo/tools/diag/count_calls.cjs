/* 统计 canvas2d 后端每帧的 Canvas 2D API 调用次数（monkey patch，不改 wasm）。
 * 用法: node count_calls.cjs <outDir> [urlSuffix] [clickSpec] [seconds]
 */
const { chromium } = require('playwright-core');
const http = require('http');
const fs = require('fs');
const path = require('path');

const root = process.argv[2];
const urlArg = process.argv[3] || '';
const clickArg = process.argv[4] || '';
const secs = parseInt(process.argv[5] || '4', 10);

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

  // 在所有页面脚本之前 patch Canvas 2D
  await page.addInitScript(() => {
    function install() {
    const names = ['fillRect', 'fillText', 'strokeText', 'drawImage', 'save', 'restore', 'setTransform',
      'beginPath', 'fill', 'stroke', 'arc', 'ellipse', 'moveTo', 'lineTo', 'bezierCurveTo',
      'clip', 'putImageData', 'getImageData', 'createLinearGradient', 'createRadialGradient',
      'clearRect', 'rect', 'closePath', 'translate', 'rotate', 'scale', 'measureText',
      'setLineDash', 'quadraticCurveTo', 'arcTo'];
    if (typeof CanvasRenderingContext2D === 'undefined') return false;
    window.__c2d = { counts: {}, frames: 0, t0: performance.now() };
    const proto = CanvasRenderingContext2D.prototype;
    for (const n of names) {
      const orig = proto[n];
      if (typeof orig !== 'function') continue;
      proto[n] = function (...a) { window.__c2d.counts[n] = (window.__c2d.counts[n] || 0) + 1; return orig.apply(this, a); };
    }
    // 属性赋值计数（fillStyle/strokeStyle/font 等）
    const props = ['fillStyle', 'strokeStyle', 'font', 'globalAlpha', 'lineWidth', 'textAlign', 'textBaseline', 'imageSmoothingEnabled', 'globalCompositeOperation'];
    for (const p of props) {
      const d = Object.getOwnPropertyDescriptor(proto, p);
      if (!d || !d.set) continue;
      Object.defineProperty(proto, p, {
        get: d.get,
        set(v) { window.__c2d.counts['set:' + p] = (window.__c2d.counts['set:' + p] || 0) + 1; return d.set.call(this, v); },
      });
    }
    const raf = window.requestAnimationFrame;
    window.requestAnimationFrame = function (cb) {
      return raf(function (t) { window.__c2d.frames++; return cb(t); });
    };
    window.__c2dInstalled = true;
    return true;
    }
    if (!install()) {
      // document 未就绪时 CanvasRenderingContext2D 还不存在，等 DOM 可用再装
      const iv = setInterval(() => { if (install()) clearInterval(iv); }, 10);
    }
  });

  await page.goto(`http://127.0.0.1:${port}/soui-wasm-demo.html${urlArg}`, { timeout: 30000 });
  await page.waitForFunction(() => { const c = document.querySelector('canvas'); return c && c.width > 100; }, { timeout: 30000 });
  await page.waitForTimeout(1200);

  // 先清零统计，再触发动画，统计窗口覆盖动画期
  await page.evaluate(() => { window.__c2d.counts = {}; window.__c2d.frames = 0; window.__c2d.t0 = performance.now(); });
  if (clickArg) {
    const cw = await page.evaluate(() => document.querySelector('canvas').width);
    const ch = await page.evaluate(() => document.querySelector('canvas').height);
    const box0 = await page.evaluate(() => {
      const r = document.querySelector('canvas').getBoundingClientRect();
      return { x: r.x, y: r.y, w: r.width, h: r.height };
    });
    for (const one of clickArg.split(';')) {
      const [cx, cy] = one.split(',').map(Number);
      const sx = box0.x + cx * (box0.w / cw);
      const sy = box0.y + cy * (box0.h / ch);
      await page.mouse.move(sx, sy);
      await page.mouse.down();
      await page.mouse.up();
      await page.waitForTimeout(1200);
    }
  }

  await page.waitForTimeout(secs * 1000);
  const r = await page.evaluate(() => {
    const d = window.__c2d;
    const total = Object.values(d.counts).reduce((a, b) => a + b, 0);
    const perFrame = d.frames ? (total / d.frames) : 0;
    return { frames: d.frames, total, perFrame: +perFrame.toFixed(1), counts: d.counts };
  });
  console.log(`[raf] frames=${r.frames} 总调用=${r.total} 平均每帧=${r.perFrame} 次`);
  const sorted = Object.entries(r.counts).sort((a, b) => b[1] - a[1]);
  console.log('[top] ' + sorted.slice(0, 18).map(([k, v]) => `${k}=${(v / Math.max(1, r.frames)).toFixed(1)}`).join(' '));
  await browser.close();
  server.close();
})();
