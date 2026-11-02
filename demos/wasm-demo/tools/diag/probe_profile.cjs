/* 用 CDP Profiler 采样坦克页（含空闲/移动），按 self time 排出热点函数。
 * wasm 侧链接时保留了 name 段，所以栈里能看到 C++ 函数名。
 *
 * 用法: node probe_profile.cjs [outDir] [毫秒=3000]
 */
'use strict';
const { chromium } = require('playwright-core');
const http = require('http');
const fs = require('fs');
const path = require('path');

const root = process.argv[2] || path.resolve(__dirname, '..', '..', 'out');
const ms = parseInt(process.argv[3] || '3000', 10);
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
  const click = async (cx, cy) => {
    await page.mouse.move(box.x + cx * (box.w / box.cw), box.y + cy * (box.h / box.ch));
    await page.mouse.down(); await page.mouse.up(); await page.waitForTimeout(500);
  };
  await click(593, 72);   // Lua 游戏
  await click(72, 218);   // 坦克大战
  if (process.argv[4] !== 'nogame')
    await click(196, 505); // 开始游戏
  await page.waitForTimeout(900);

  const cdp = await page.context().newCDPSession(page);
  await cdp.send('Profiler.enable');
  await cdp.send('Profiler.setSamplingInterval', { interval: 200 }); // 200us 采样
  await cdp.send('Profiler.start');
  // 采样期间让玩家坦克持续移动（用户说这一段最卡）
  await page.keyboard.down('ArrowUp');
  await page.waitForTimeout(ms);
  await page.keyboard.up('ArrowUp');
  const { profile } = await cdp.send('Profiler.stop');

  // 自底向上累计 self time
  const self = new Map();
  const byId = new Map();
  for (const n of profile.nodes) byId.set(n.id, n);
  const total = new Map(); // 被采样到的次数
  for (const id of profile.samples) total.set(id, (total.get(id) || 0) + 1);
  const dt = (profile.endTime - profile.startTime) / 1000 / profile.samples.length;
  for (const [id, cnt] of total) {
    const n = byId.get(id);
    const cf = n.callFrame;
    let name = cf.functionName || '(anonymous)';
    if (!name && cf.url) name = cf.url.split('/').pop();
    if (/wasm-function/.test(name) && n.positionTicks) name += ' @' + (n.positionTicks[0] ? n.positionTicks[0].line : '?');
    const key = name + (cf.url && cf.url.endsWith('.wasm') ? ' [wasm]' : '');
    self.set(key, (self.get(key) || 0) + cnt);
  }
  const rows = [...self.entries()].map(([k, c]) => [k, +(c * dt).toFixed(1), c]).sort((a, b) => b[1] - a[1]).slice(0, 22);
  console.log('总耗时(ms)', ((profile.endTime - profile.startTime) / 1000).toFixed(0), '采样数', profile.samples.length);
  for (const [k, t, c] of rows) console.log(String(t).padStart(8), 'ms ', k);

  // 最热的那个节点，往上打 8 层调用链，看是谁在调
  const parentOf = new Map();
  for (const n of profile.nodes) if (n.children) for (const ch of n.children) parentOf.set(ch, n.id);
  const topId = [...total.entries()].filter(([id]) => {
    const n = byId.get(id); return n && /localtime|date_now/.test(n.callFrame.functionName || '');
  }).sort((a, b) => b[1] - a[1])[0];
  if (topId) {
    console.log('--- 调用链 ---');
    let cur = topId[0];
    const chain = [];
    for (let i = 0; i < 9 && cur; ++i) {
      const n = byId.get(cur);
      if (!n) break;
      chain.push(n.callFrame.functionName || '(anon)');
      cur = parentOf.get(cur);
    }
    console.log(chain.join('  <-  '));
  }
  fs.writeFileSync(path.join(__dirname, 'profile_tank.json'), JSON.stringify(profile));
  await browser.close();
  server.close();
})();
