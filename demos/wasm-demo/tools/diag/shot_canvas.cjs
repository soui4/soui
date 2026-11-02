/* 截 canvas 元素的 1:1 区域图，便于精确量坐标 */
const { chromium } = require('playwright-core');
const http = require('http');
const fs = require('fs');
const path = require('path');

const root = process.argv[2];
const shot = process.argv[3];
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
  await page.goto(`http://127.0.0.1:${port}/soui-wasm-demo.html`, { timeout: 30000 });
  await page.waitForFunction(() => { const c = document.querySelector('canvas'); return c && c.width > 100; }, { timeout: 30000 });
  await page.waitForTimeout(1500);
  const box = await page.evaluate(() => {
    const r = document.querySelector('canvas').getBoundingClientRect();
    return { x: r.x, y: r.y, width: r.width, height: r.height };
  });
  await page.screenshot({ path: shot, clip: box });
  console.log('[clip]', JSON.stringify(box), '->', shot);
  await browser.close();
  server.close();
})();
