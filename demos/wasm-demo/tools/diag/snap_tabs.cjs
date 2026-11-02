/* 逐个页签截图到目录，用于跨版本像素级对比（md5）。
 * 用法: node snap_tabs.cjs <outDir> <输出目录> [页签x列表=140,200,260,...]
 */
const { chromium } = require('playwright-core');
const http = require('http');
const fs = require('fs');
const path = require('path');

const root = process.argv[2];
const outDir = process.argv[3];
const tabs = (process.argv[4] || '140,200,260,320,380,440,500,560,620,680').split(',').map(Number);
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
  fs.mkdirSync(outDir, { recursive: true });
  await new Promise(r => server.listen(0, '127.0.0.1', r));
  const port = server.address().port;
  const browser = await chromium.launch({ headless: true, channel: 'msedge' });
  const page = await browser.newPage({ viewport: { width: 1400, height: 1000 } });
  const errs = [];
  page.on('pageerror', e => errs.push(e.message));
  page.on('console', m => { if (m.type() === 'error') errs.push(m.text()); });

  await page.goto(`http://127.0.0.1:${port}/soui-wasm-demo.html`, { timeout: 30000 });
  await page.waitForFunction(() => document.querySelector('canvas') && document.querySelector('canvas').width > 100, { timeout: 30000 });
  await page.waitForTimeout(1800);

  const box = await page.evaluate(() => {
    const r = document.querySelector('canvas').getBoundingClientRect();
    const c = document.querySelector('canvas');
    return { x: r.x, y: r.y, w: r.width, h: r.height, cw: c.width, ch: c.height };
  });

  for (const tx of tabs) {
    await page.mouse.move(box.x + tx * (box.w / box.cw), box.y + 72 * (box.h / box.ch));
    await page.mouse.down();
    await page.mouse.up();
    await page.waitForTimeout(900);
    const clip = await page.evaluate(() => {
      const c = document.querySelector('canvas');
      const r = c.getBoundingClientRect();
      return { x: Math.round(r.x), y: Math.round(r.y), w: Math.round(r.width), h: Math.round(r.height) };
    });
    const buf = await page.screenshot({ clip: { x: clip.x, y: clip.y, width: clip.w, height: clip.h } });
    const fp = path.join(outDir, `tab_${tx}.png`);
    fs.writeFileSync(fp, buf);
    console.log('WROTE', fp, buf.length);
  }
  console.log('errors:', errs.length ? errs.slice(0, 5) : 'none');
  await browser.close();
  server.close();
})();
