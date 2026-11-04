/* 页面内 60fps 抓帧 + 确定性自动玩（按列轮转硬降，容易消行）。
 * 目的：真实消行触发的得分动画（飞分/光波/碎片）逐帧取证。
 * 用法: SWX_HEADED=1 node _play60.cjs [tag] [帧数]
 */
'use strict';
const { chromium } = require('playwright-core');
const http = require('http');
const fs = require('fs');
const path = require('path');

const root = path.resolve(__dirname, '..', '..', process.env.SWX_OUT || 'out');
const tag = process.argv[2] || 'p60';
const N = parseInt(process.argv[3] || '400', 10);
const outDir = path.join(__dirname, 'frames_' + tag);
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
    headless: false, channel: 'msedge',
    args: ['--window-position=40,40', '--window-size=1440,1040'],
  });
  const page = await browser.newPage({ viewport: { width: 1400, height: 1000 }, deviceScaleFactor: parseFloat(process.env.DSF || '1') });
  await page.goto(`http://127.0.0.1:${port}/soui-wasm-demo.html`, { timeout: 30000 });
  await page.waitForFunction(() => document.querySelector('canvas') && document.querySelector('canvas').width > 100, { timeout: 30000 });
  await page.waitForTimeout(1500);
  const box = await page.evaluate(() => {
    const c = document.getElementById('screen');
    const r = c.getBoundingClientRect();
    var lw = Module._SwinxFbLogicalWidth ? Module._SwinxFbLogicalWidth() : c.width;
    var lh = Module._SwinxFbLogicalHeight ? Module._SwinxFbLogicalHeight() : c.height;
    return { x: r.x, y: r.y, w: r.width, h: r.height, cw: lw, ch: lh };
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

  /* 确定性自动玩：每块先转到底再按列轮转硬降，填满行就会消 */
  let stop = false, col = 0;
  (async () => {
    while (!stop) {
      await page.keyboard.press('ArrowUp').catch(() => {});
      for (let i = 0; i < col; ++i) await page.keyboard.press('ArrowRight').catch(() => {});
      await page.keyboard.press('Space').catch(() => {});
      col = (col + 2) % 9;
      try { await page.waitForTimeout(180); } catch { break; }
    }
  })();

  /* 页面内逐帧抓 __swxDisplay（不依赖 CDP 往返，采样率 ~60fps） */
  const frames = await page.evaluate(async (n) => {
    const c2d = globalThis.__swxDisplay;
    const cv = c2d.canvas || c2d;   /* __swxDisplay 是 2D context，不是 canvas */
    const out = [];
    for (let i = 0; i < n; i++) {
      await new Promise((r) => requestAnimationFrame(r));
      out.push(cv.toDataURL('image/png'));
    }
    return out;
  }, N);
  stop = true;
  frames.forEach((d, i) => fs.writeFileSync(path.join(outDir, String(i).padStart(4, '0') + '.png'), Buffer.from(d.split(',')[1], 'base64')));
  console.log('frames', frames.length);
  await browser.close();
  server.close();
})();
