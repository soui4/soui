/* 坦克大战音效探针：验证 lua 侧的事件到底有没有走到 PlaySound。
 *
 * 做法：在页面里把 Module.swinxAudioPlay 包一层，按 wav 文件名计数（顺便统计
 * 被 SND_NOSTOP 丢掉的那些 —— 敌人开炮走的就是这条）。不需要真的能出声：
 * 无头 Edge 里 AudioContext 常常是 suspended，解码播放不会发生，但"请求已提交"
 * 这一步正是 lua/swinx 的责任边界。
 *
 * 用法: node probe_sfx.cjs [outDir]
 */
'use strict';
const { chromium } = require('playwright-core');
const http = require('http');
const fs = require('fs');
const path = require('path');

const root = process.argv[2] || path.resolve(__dirname, '..', '..', 'out');
const mime = {
  '.html': 'text/html', '.js': 'text/javascript',
  '.wasm': 'application/wasm', '.data': 'application/octet-stream'
};

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
  page.on('pageerror', (e) => console.log('[err]', e.message));

  await page.addInitScript(() => {
    window.__sfx = Object.create(null);
    window.__sfxDrop = Object.create(null);
    (function hook() {
      const M = window.Module;
      if (!M || typeof M.swinxAudioPlay !== 'function') { setTimeout(hook, 20); return; }
      if (M.__sfxHooked) return;
      M.__sfxHooked = true;
      const orig = M.swinxAudioPlay.bind(M);
      M.swinxAudioPlay = function (ptr, len, flags, pathPtr) {
        let key = '';
        if (pathPtr) {
          const h = M.HEAPU8;
          let p = pathPtr >>> 0;
          while (h[p] !== 0) { key += String.fromCharCode(h[p]); p++; }
        }
        const name = key ? key.split('/').pop() : '(stop)';
        const ok = orig(ptr, len, flags, pathPtr);
        (ok ? window.__sfx : window.__sfxDrop)[name] =
          ((ok ? window.__sfx : window.__sfxDrop)[name] || 0) + 1;
        return ok;
      };
    })();
  });

  await page.goto(`http://127.0.0.1:${port}/soui-wasm-demo.html`, { timeout: 30000 });
  await page.waitForFunction(() => {
    const c = document.querySelector('canvas');
    return c && c.width > 100;
  }, { timeout: 30000 });
  await page.waitForTimeout(1800);

  const box = await page.evaluate(() => {
    const c = document.getElementById('screen');
    const r = c.getBoundingClientRect();
    return { x: r.x, y: r.y, w: r.width, h: r.height, cw: c.width, ch: c.height };
  });
  const click = async (cx, cy) => {
    await page.mouse.move(box.x + cx * (box.w / box.cw), box.y + cy * (box.h / box.ch));
    await page.mouse.down();
    await page.mouse.up();
    await page.waitForTimeout(500);
  };

  const counts = () => page.evaluate(() => ({ ok: window.__sfx, drop: window.__sfxDrop }));
  const stages = {};

  await click(593, 72);        // 「Lua 游戏」页签
  await click(72, 218);        // 「坦克大战」内页签
  await page.waitForTimeout(300);
  await click(196, 505);       // 「开始游戏」
  await page.waitForTimeout(1600);
  stages.afterStart = await counts();

  // 向上连射：玩家在 (4,12) 朝上，正上方 (4,11) 是砖墙 ⇒ 应命中 tank_brick
  for (let i = 0; i < 6; ++i) {
    await page.keyboard.press('Space');
    await page.waitForTimeout(300);
  }
  stages.afterFire = await counts();

  // 停留 4 秒，让敌人出生并开炮
  await page.waitForTimeout(4000);
  stages.afterWait = await counts();

  const fmt = (o) => {
    const keys = Object.keys(o).filter((k) => k !== '(stop)').sort();
    return keys.map((k) => k + '=' + o[k]).join(' ') || '(无)';
  };
  for (const [name, c] of Object.entries(stages)) {
    console.log('[%s] 播放: %s', name, fmt(c.ok));
    const drop = fmt(c.drop);
    if (drop !== '(无)') console.log('[%s] 被 NOSTOP 丢弃: %s', name, drop);
  }

  const final = stages.afterWait;
  const need = ['tank_start.wav', 'tank_fire.wav', 'tank_brick.wav'];
  const missing = need.filter((k) => !final.ok[k]);
  console.log(missing.length ? '缺失: ' + missing.join(', ') : '必备音效全部触发');

  await browser.close();
  server.close();
  process.exit(missing.length ? 1 : 0);
})();
