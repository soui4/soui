/* shot_dpr.cjs -- 用 Playwright 起一个静态服务器 + 无头 Edge，在指定 devicePixelRatio
 * 下打开产物，打印 DPI 链路的状态（可选截图）。
 *
 * 为什么不用 CDP 版 shoot_dpi.cjs：那个要求手工起服务器、手工开 9222 调试端口，适合
 * 已经在跑的浏览器；这个自带服务器和浏览器，一行命令就能跑多档，用于回归 dpI 档位表。
 *
 *   node shot_dpr.cjs <产物目录> <dsf> [输出 png]
 *   node shot_dpr.cjs ../../demos/wasm/out 1.125 /tmp/dpi_1125.png
 *
 * 输出：dpr / canvas backing 尺寸 / CSS 尺寸 / 帧缓冲尺寸，外加 wasm main() 打印的
 * 「dpi dpr=... -> scale=...%」那一行（档位是向上贴内置档位后的结果）。
 */
'use strict';
const { chromium } = require('playwright-core');
const http = require('http');
const fs = require('fs');
const path = require('path');

const root = process.argv[2];
const dsf = parseFloat(process.argv[3] || '1.25');
const outPng = process.argv[4];
if (!root) {
  console.error('用法: node shot_dpr.cjs <产物目录> <dsf> [输出 png]');
  process.exit(1);
}

const MIME = {
  '.html': 'text/html',
  '.js': 'text/javascript',
  '.wasm': 'application/wasm',
  '.data': 'application/octet-stream',
};

const server = http.createServer((req, res) => {
  let p = decodeURIComponent(req.url.split('?')[0]);
  if (p === '/') p = '/soui-wasm-demo.html';
  const f = path.join(root, p);
  if (!fs.existsSync(f)) {
    res.writeHead(404);
    res.end('not found');
    return;
  }
  res.writeHead(200, { 'Content-Type': MIME[path.extname(f)] || 'application/octet-stream' });
  fs.createReadStream(f).pipe(res);
});

(async () => {
  await new Promise((r) => server.listen(0, '127.0.0.1', r));
  const port = server.address().port;
  const browser = await chromium.launch({ headless: true, channel: 'msedge' });
  const page = await browser.newPage({ viewport: { width: 1400, height: 1100 }, deviceScaleFactor: dsf });

  const dpiLine = [];
  page.on('console', (m) => {
    const t = m.text();
    if (t.indexOf('[soui-wasm] dpi') >= 0) dpiLine.push(t);
  });

  await page.goto(`http://127.0.0.1:${port}/soui-wasm-demo.html`, { timeout: 30000 });
  await page.waitForFunction(
    () => {
      const c = document.querySelector('canvas');
      return c && c.width > 100;
    },
    { timeout: 30000 }
  );
  await page.waitForTimeout(2000);

  const info = await page.evaluate(() => {
    const c = document.getElementById('screen');
    const r = c.getBoundingClientRect();
    /* 宿主建的几个节点按旧 id 保留（soui-host.js），顺手确认它们都在：
     * 少了说明 soui-host.js 没被拷进产物目录，或者页面没引用它。 */
    const ui = {};
    ['screen-wrap', 'loading', 'load-phase', 'ime-proxy', 'log'].forEach((id) => {
      ui[id] = !!document.getElementById(id);
    });
    return {
      dpr: window.devicePixelRatio,
      backing: [c.width, c.height],
      css: [Math.round(r.width), Math.round(r.height)],
      fb: [Module._SwinxFbWidth(), Module._SwinxFbHeight()],
      ui,
    };
  });

  console.log(JSON.stringify({ dsf, ...info, log: dpiLine.join(' | ') }));

  if (outPng) {
    const el = await page.$('#screen');
    await el.screenshot({ path: outPng });
    console.log('saved ' + outPng);
  }

  await browser.close();
  server.close();
})().catch((e) => {
  console.error(e.message);
  process.exit(1);
});
