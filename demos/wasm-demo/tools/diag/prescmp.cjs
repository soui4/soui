/* 直接对比"窗口离屏画布"与"显示画布"：任何差异都说明上屏（present）漏了区域，
 * 也就是屏幕上残留旧像素 —— 正是"擦除不干净"的机制。
 * 用法: node _prescmp.cjs [帧数]
 */
'use strict';
const { chromium } = require('playwright-core');
const http = require('http');
const fs = require('fs');
const path = require('path');
const root = path.resolve(__dirname, '..', '..', process.env.SWX_OUT || 'out');
const N = parseInt(process.argv[2] || '240', 10);
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
  const browser = await chromium.launch({ headless: false, channel: 'msedge', args: ['--window-position=40,40', '--window-size=1440,1040'] });
  const page = await browser.newPage({ viewport: { width: 1400, height: 1000 }, deviceScaleFactor: parseFloat(process.env.DSF || '1') });
  await page.goto(`http://127.0.0.1:${port}/soui-wasm-demo.html`, { timeout: 30000 });
  await page.waitForFunction(() => document.querySelector('canvas') && document.querySelector('canvas').width > 100, { timeout: 30000 });
  await page.waitForTimeout(1500);
  const box = await page.evaluate(() => {
    const c = document.getElementById('screen');
    const r = c.getBoundingClientRect();
    const lw = Module._SwinxFbLogicalWidth ? Module._SwinxFbLogicalWidth() : c.width;
    const lh = Module._SwinxFbLogicalHeight ? Module._SwinxFbLogicalHeight() : c.height;
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
  await page.waitForTimeout(500);
  let stop = false, col = 0;
  (async () => {
    while (!stop) {
      await page.keyboard.press('ArrowUp').catch(() => {});
      for (let i = 0; i < col; ++i) await page.keyboard.press('ArrowRight').catch(() => {});
      await page.keyboard.press('Space').catch(() => {});
      col = (col + 2) % 9;
      /* 主流程跑完会关掉浏览器，自动操作线程要能自己退出，否则尾部抛未捕获异常 */
      try { await page.waitForTimeout(180); } catch { break; }
    }
  })();
  const res = await page.evaluate(async (n) => {
    const disp = globalThis.__swxDisplay;
    const dw = (disp.canvas || disp).width, dh = (disp.canvas || disp).height;
    /* 找到窗口离屏画布：尺寸与显示画布一致、且不是显示画布自己 */
    let win = null;
    const tbl = globalThis.__swxCtx || [];
    for (let i = 1; i < tbl.length; i++) {
      const c = tbl[i];
      if (!c || !c.canvas) continue;
      if (c === disp) continue;
      if (c.canvas.width === dw && c.canvas.height === dh) {
        if (!win || i < win.id) win = { id: i, ctx: c };
      }
    }
    if (!win) return { err: '未找到窗口离屏画布', n: tbl.length, dw, dh };
    const bad = [];
    for (let k = 0; k < n; k++) {
      await new Promise((r) => requestAnimationFrame(r));
      const A = win.ctx.getImageData(0, 0, dw, dh).data;   /* 离屏：应该的 */
      const B = disp.getImageData(0, 0, dw, dh).data;      /* 显示：实际看到的 */
      let cnt = 0, x0 = 1e9, y0 = 1e9, x1 = -1, y1 = -1, mx = 0;
      for (let y = 0; y < dh; y++) for (let x = 0; x < dw; x++) {
        const i = (y * dw + x) * 4;
        const d = Math.max(Math.abs(A[i] - B[i]), Math.abs(A[i + 1] - B[i + 1]), Math.abs(A[i + 2] - B[i + 2]));
        if (d > 2) { cnt++; if (d > mx) mx = d; if (x < x0) x0 = x; if (x > x1) x1 = x; if (y < y0) y0 = y; if (y > y1) y1 = y; }
      }
      if (cnt > 0) bad.push({ k, cnt, mx, box: cnt ? [x0, y0, x1, y1] : null });
    }
    return { id: win.id, dw, dh, frames: n, badCount: bad.length, bad: bad.slice(0, 12), total: bad.length };
  }, N);
  stop = true;
  console.log(JSON.stringify(res, null, 1));
  await browser.close(); server.close();
})();
