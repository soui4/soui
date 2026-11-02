/* 看"空闲时"到底在重绘什么：把 Module.swinxPresent 包一层，记录每次上屏的脏矩形。
 * 用法: node probe_present.cjs <outDir> [urlSuffix] [clickSpec] [秒]
 */
const { chromium } = require('playwright-core');
const http = require('http');
const fs = require('fs');
const path = require('path');

const root = process.argv[2] || path.resolve(__dirname, '..', '..', 'out');
const urlArg = process.argv[3] || '';
const clickArg = process.argv[4] || '';
const secs = parseFloat(process.argv[5] || '2');
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
  page.on('console', m => { if (process.env.V) console.log('[c]', m.text()); });
  await page.addInitScript(() => {
    window.__rects = [];
    const iv = setInterval(() => {
      if (!window.Module || typeof window.Module.swinxPresent !== 'function') return;
      const orig = window.Module.swinxPresent;
      window.Module.swinxPresent = function (l, t, r, b) {
        if (window.__rects.length < 400)
          window.__rects.push([Math.round(l), Math.round(t), Math.round(r), Math.round(b), performance.now() | 0]);
        return orig.apply(this, arguments);
      };
      clearInterval(iv);
    }, 5);
  });
  await page.goto(`http://127.0.0.1:${port}/soui-wasm-demo.html${urlArg}`, { timeout: 30000 });
  await page.waitForFunction(() => document.querySelector('canvas') && document.querySelector('canvas').width > 100, { timeout: 30000 });
  await page.waitForTimeout(1500);
  if (clickArg) {
    const box = await page.evaluate(() => { const r = document.querySelector('canvas').getBoundingClientRect(); return { x: r.x, y: r.y, w: r.width, h: r.height }; });
    const cw = await page.evaluate(() => document.querySelector('canvas').width);
    const ch = await page.evaluate(() => document.querySelector('canvas').height);
    for (const one of clickArg.split(';')) {
      const [cx, cy] = one.split(',').map(Number);
      await page.mouse.move(box.x + cx * (box.w / cw), box.y + cy * (box.h / ch));
      await page.mouse.down();
      await page.mouse.up();
      await page.waitForTimeout(1200);
    }
  }
  await page.evaluate(() => { window.__rects.length = 0; });
  await page.waitForTimeout(secs * 1000);
  const rects = await page.evaluate(() => window.__rects);
  // 统计出现最多的矩形
  const cnt = new Map();
  for (const [l, t, r, b] of rects) {
    const k = `${l},${t},${r},${b} (${r - l}x${b - t})`;
    cnt.set(k, (cnt.get(k) || 0) + 1);
  }
  const top = [...cnt.entries()].sort((a, b) => b[1] - a[1]).slice(0, 8);
  console.log('总上屏次数', rects.length, '速率', (rects.length / secs).toFixed(0), '/s');
  for (const [k, v] of top) console.log('  ', v, k);
  const gaps = [];
  for (let i = 1; i < rects.length; ++i) gaps.push(rects[i][4] - rects[i - 1][4]);
  gaps.sort((a, b) => a - b);
  if (gaps.length) console.log('间隔 p50/p90/max(ms)', gaps[gaps.length >> 1], Math.round(gaps[Math.floor(gaps.length * 0.9)]), gaps[gaps.length - 1]);
  await browser.close();
  server.close();
})();
