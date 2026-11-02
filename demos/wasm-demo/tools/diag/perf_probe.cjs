/* 测 wasm demo 的运行帧率：加载页面 → (可选)点击进入动画 → 统计 rAF 频率 → 截图
 * 用法: node perf_probe.cjs <outDir> [urlSuffix] [clickX,clickY] [seconds] [shotPath]
 */
const { chromium } = require('playwright-core');
const http = require('http');
const fs = require('fs');
const path = require('path');

const root = process.argv[2] || 'D:/work/soui4/demos/wasm-demo/out';
const urlArg = process.argv[3] || '';
const clickArg = process.argv[4] || '';
const secs = parseInt(process.argv[5] || '3', 10);
const shot = process.argv[6] || '';

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
  const page = await browser.newPage({ viewport: { width: 1400, height: 1000 } });
  const errors = [];
  page.on('console', m => { if (m.type() === 'error') errors.push(m.text()); console.log('[c]', m.text()); });
  page.on('pageerror', e => errors.push(e.message));

  await page.goto(url, { timeout: 30000 });
  await page.waitForFunction(() => document.querySelector('canvas') && document.querySelector('canvas').width > 100, { timeout: 30000 });
  await page.waitForTimeout(1500);

  if (clickArg) {
    const cw = await page.evaluate(() => document.querySelector('canvas').width);
    const ch = await page.evaluate(() => document.querySelector('canvas').height);
    const box0 = await page.evaluate(() => {
      const r = document.querySelector('canvas').getBoundingClientRect();
      return { x: r.x, y: r.y, w: r.width, h: r.height };
    });
    console.log('[box]', JSON.stringify(box0), 'canvas', cw, 'x', ch);
    for (const one of clickArg.split(';')) {
      const [cx, cy] = one.split(',').map(Number);
      const sx = box0.x + cx * (box0.w / cw);
      const sy = box0.y + cy * (box0.h / ch);
      await page.mouse.move(sx, sy);
      await page.mouse.down();
      await page.mouse.up();
      console.log(`[click] canvas(${cx},${cy}) -> page(${sx.toFixed(1)},${sy.toFixed(1)})`);
      await page.waitForTimeout(1200);
    }
  }

  // 统计 rAF 频率
  const fps = await page.evaluate(async (s) => {
    let frames = 0;
    const t0 = performance.now();
    return await new Promise(resolve => {
      function tick() {
        frames++;
        if (performance.now() - t0 < s * 1000) requestAnimationFrame(tick);
        else resolve({ frames, ms: performance.now() - t0, fps: frames / ((performance.now() - t0) / 1000) });
      }
      requestAnimationFrame(tick);
    });
  }, secs);
  console.log(`[fps] frames=${fps.frames} over ${fps.ms.toFixed(0)}ms => ${fps.fps.toFixed(1)} fps`);

  // wasm 侧是否有帧耗时统计（swinx 的 present 计时，若存在）
  const canvasInfo = await page.evaluate(() => {
    const c = document.querySelector('canvas');
    return { w: c.width, h: c.height };
  });
  console.log('[canvas]', JSON.stringify(canvasInfo));

  if (shot) {
    await page.screenshot({ path: shot });
    console.log('[shot]', shot);
  }
  if (errors.length) console.log('[errors]', errors.slice(0, 5).join(' | '));
  await browser.close();
  server.close();
})();
