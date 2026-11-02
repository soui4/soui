/* 把 JS 注册表里的离屏窗口画布导出成 PNG，和显示画布对比，用于判断
 * "内容错位"发生在绘制阶段还是上屏（present）阶段。
 * 用法: node dump_surface.cjs <outDir> [页签x] [输出前缀]
 */
const { chromium } = require('playwright-core');
const http = require('http');
const fs = require('fs');
const path = require('path');

const root = process.argv[2];
const tabx = Number(process.argv[3] || '140');
const prefix = process.argv[4] || '/tmp/dump';
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
  await page.waitForFunction(() => document.querySelector('canvas') && document.querySelector('canvas').width > 100, { timeout: 30000 });
  await page.waitForTimeout(1800);
  if (tabx > 0) {
    const box = await page.evaluate(() => {
      const r = document.querySelector('canvas').getBoundingClientRect();
      const c = document.querySelector('canvas');
      return { x: r.x, y: r.y, w: r.width, h: r.height, cw: c.width, ch: c.height };
    });
    await page.mouse.move(box.x + tabx * (box.w / box.cw), box.y + 72 * (box.h / box.ch));
    await page.mouse.down();
    await page.mouse.up();
    await page.waitForTimeout(1500);
  }

  const info = await page.evaluate(() => {
    const g = globalThis;
    const out = [];
    const tab = g.__swxCtx || [];
    for (let i = 1; i < tab.length; ++i) {
      const c = tab[i];
      if (!c || !c.canvas) continue;
      const cv = c.canvas;
      out.push({ id: i, w: cv.width, h: cv.height, url: cv.width > 100 ? cv.toDataURL('image/png') : '' });
    }
    // 显示画布
    const disp = document.querySelector('canvas');
    return { surfaces: out, disp: { w: disp.width, h: disp.height, url: disp.toDataURL('image/png') } };
  });

  const save = (u, name) => {
    if (!u) return;
    const b = Buffer.from(u.split(',')[1], 'base64');
    fs.writeFileSync(name, b);
    console.log('WROTE', name, b.length);
  };
  for (const s of info.surfaces) save(s.url, `${prefix}_surface_${s.id}_${s.w}x${s.h}.png`);
  save(info.disp.url, `${prefix}_display.png`);
  await browser.close();
  server.close();
})();
