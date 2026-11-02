/* 用无头 chromium 打开 wasm demo，抓 console/pageerror/abort 信息 */
const { chromium } = require('playwright-core');
const http = require('http');
const fs = require('fs');
const path = require('path');

const root = process.argv[2] || 'D:/work/soui4/demos/wasm-demo/out';
const urlArg = process.argv[3] || '';
const mime = { '.html': 'text/html', '.js': 'text/javascript', '.wasm': 'application/wasm', '.data': 'application/octet-stream', '.map': 'application/json' };

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
  const url = `http://127.0.0.1:${port}/soui-wasm-demo.html${urlArg}`;
  console.log('[open]', url);

  const browser = await chromium.launch({ headless: true, channel: 'msedge' });
  const page = await browser.newPage();
  page.on('console', m => console.log('[console]', m.type(), m.text()));
  page.on('pageerror', e => console.log('[pageerror]', e.message));
  page.on('crash', () => console.log('[crash] page crashed'));

  try {
    await page.goto(url, { timeout: 30000 });
    await page.waitForTimeout(8000);
    const canvas = await page.evaluate(() => {
      const c = document.querySelector('canvas');
      return c ? { w: c.width, h: c.height, styleW: c.style.width } : null;
    });
    console.log('[canvas]', JSON.stringify(canvas));
  } catch (e) {
    console.log('[nav-error]', e.message);
  }
  await browser.close();
  server.close();
})();
