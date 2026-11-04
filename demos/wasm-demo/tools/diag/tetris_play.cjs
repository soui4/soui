/* 自动玩俄罗斯方块 + 全程高速连拍。
 * 目的：真实玩法（随机落点 + 硬降 + 消行 + 各个动画）下抓帧，
 * 之后离线扫描"同一帧出现多处金色文字/光带"这类擦除残留。
 *
 * 用法: node tetris_play.cjs [tag] [帧数]
 */
'use strict';
const { chromium } = require('playwright-core');
const http = require('http');
const fs = require('fs');
const path = require('path');

const root = path.resolve(__dirname, '..', '..', process.env.SWX_OUT || 'out');
const tag = process.argv[2] || 'p';
const N = parseInt(process.argv[3] || '420', 10);
const outDir = path.join(__dirname, 'tetris_play_' + tag);
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
  await new Promise((r) => server.listen(0, '127.0.0.1', r));
  const port = server.address().port;
  const browser = await chromium.launch({
    headless: process.env.SWX_HEADED ? false : true,
    channel: 'msedge',
    args: process.env.SWX_HEADED ? ['--window-position=40,40', '--window-size=1440,1040'] : [],
  });
  const page = await browser.newPage({ viewport: { width: 1400, height: 1000 }, deviceScaleFactor: parseFloat(process.env.DSF || '1') });
  const cdp = await page.context().newCDPSession(page);

  await page.goto(`http://127.0.0.1:${port}/soui-wasm-demo.html`, { timeout: 30000 });
  await page.waitForFunction(() => document.querySelector('canvas') && document.querySelector('canvas').width > 100, { timeout: 30000 });
  await page.waitForTimeout(1500);
  const box = await page.evaluate(() => {
    const c = document.getElementById('screen');
    const r = c.getBoundingClientRect();
    return { x: r.x, y: r.y, w: r.width, h: r.height, cw: c.width, ch: c.height };
  });
  const click = async (cx, cy) => {
    const x = box.x + cx * (box.w / box.cw), y = box.y + cy * (box.h / box.ch);
    await page.mouse.move(x, y); await page.mouse.down(); await page.mouse.up();
    await page.waitForTimeout(450);
  };
  await click(593, 72);
  await click(58, 201);
  await click(220, 504);
  await page.waitForTimeout(600);

  /* 自动操作：并发跑，与截图互不阻塞（截图走独立 CDP session） */
  let stop = false;
  (async () => {
    let k = 0;
    while (!stop) {
      k = (k + 3) % 10;
      for (let i = 0; i < 5; ++i) { await page.keyboard.press('ArrowLeft').catch(() => {}); }
      for (let i = 0; i < k; ++i) { await page.keyboard.press('ArrowRight').catch(() => {}); }
      await page.keyboard.press('Space').catch(() => {});
      /* 抓完帧会关掉浏览器，自动操作线程要能自己退出 */
      try { await page.waitForTimeout(220); } catch { break; }
    }
  })();

  const clip = { x: Math.round(box.x), y: Math.round(box.y), width: Math.round(box.w), height: Math.round(box.h), scale: 1 };
  const t0 = Date.now();
  for (let i = 0; i < N; ++i) {
    const r = await cdp.send('Page.captureScreenshot', { format: 'png', clip });
    fs.writeFileSync(path.join(outDir, String(i).padStart(4, '0') + '.png'), Buffer.from(r.data, 'base64'));
  }
  stop = true;
  console.log('frames', N, 'in', Date.now() - t0, 'ms');
  await browser.close();
  server.close();
})();
