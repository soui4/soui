/* 粗检渲染：依次点几个标签页各截一张图，用肉眼对照有没有裁切/拼接/文字异常。
 * 用法: node shot_check.cjs [outDir]
 */
'use strict';
const { chromium } = require('playwright-core');
const http = require('http');
const fs = require('fs');
const path = require('path');

const root = process.argv[2] || path.resolve(__dirname, '..', '..', 'out');
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
  await page.goto(`http://127.0.0.1:${port}/soui-wasm-demo.html`, { timeout: 30000 });
  await page.waitForFunction(() => document.querySelector('canvas') && document.querySelector('canvas').width > 100, { timeout: 30000 });
  await page.waitForTimeout(1800);

  const box = await page.evaluate(() => {
    const c = document.getElementById('screen');
    const r = c.getBoundingClientRect();
    return { x: r.x, y: r.y, w: r.width, h: r.height, cw: c.width, ch: c.height };
  });
  const click = async (cx, cy, ms = 700) => {
    await page.mouse.move(box.x + cx * (box.w / box.cw), box.y + cy * (box.h / box.ch));
    await page.mouse.down(); await page.mouse.up();
    await page.waitForTimeout(ms);
  };
  const shot = async (name) => {
    const el = await page.$('#screen');
    await el.screenshot({ path: path.join(__dirname, `check_${name}.png`) });
    console.log('saved check_' + name + '.png');
  };

  await shot('00_overview');
  await click(110, 72); await shot('01_basic');      // 基础控件
  await click(412, 72); await shot('02_graph');       // 图形控件
  await click(490, 72); await shot('03_table');       // 表格
  await click(256, 72); await shot('04_mvc');         // MVC 列表

  await browser.close();
  server.close();
})();
