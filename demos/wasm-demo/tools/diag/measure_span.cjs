/* 量一次"整帧绘制跨度"：把 Canvas 2D 的每个方法都打上时间戳，以 putImageData
 * （即上屏）为帧边界，统计
 *   ops  —— 一帧里的 Canvas API 调用次数
 *   span —— 从本帧第一个调用到最后一个调用的墙钟跨度（≈ wasm 侧整帧绘制耗时，
 *           因为 wasm 在这段时间里除了跨边界调 JS 什么也没干）
 *   inJs —— 这些调用体内部真正花掉的时间之和；span - inJs ≈ 跨边界 + 参数封送开销
 *
 * 用法: node measure_span.cjs <outDir> [urlSuffix] [clickSpec] [seconds]
 */
const { chromium } = require('playwright-core');
const http = require('http');
const fs = require('fs');
const path = require('path');

const root = process.argv[2] || path.resolve(__dirname, '..', '..', 'out');
const urlArg = process.argv[3] || '';
const clickArg = process.argv[4] || '';
const secs = parseInt(process.argv[5] || '4', 10);

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
  page.on('console', m => console.log('[c]', m.text()));

  await page.addInitScript(() => {
    function install() {
      if (typeof CanvasRenderingContext2D === 'undefined') return false;
      const S = window.__span = { frames: [], cur: null, windows: [] };
      const now = () => performance.now();
      const proto = CanvasRenderingContext2D.prototype;
      const names = Object.getOwnPropertyNames(proto).filter(n => {
        try { return typeof proto[n] === 'function' && n !== 'constructor'; } catch (e) { return false; }
      });
      function begin() {
        if (!S.cur) S.cur = { ops: 0, inJs: 0, t0: now(), names: {} };
      }
      function end() {
        if (S.cur) { S.cur.t1 = now(); S.frames.push(S.cur); S.cur = null; }
      }
      for (const n of names) {
        const orig = proto[n];
        if (n === 'putImageData') {
          proto[n] = function (...a) {
            begin();
            const t = now();
            const r = orig.apply(this, a);
            S.cur.inJs += now() - t;
            S.cur.ops++;
            S.cur.names[n] = (S.cur.names[n] || 0) + 1;
            end();                       // 上屏即一帧结束
            return r;
          };
        } else {
          proto[n] = function (...a) {
            begin();
            const t = now();
            const r = orig.apply(this, a);
            S.cur.inJs += now() - t;
            S.cur.ops++;
            S.cur.names[n] = (S.cur.names[n] || 0) + 1;
            return r;
          };
        }
      }
      // 属性 setter 也要计入（fillStyle/font/... 全部走 setter）
      let p = proto;
      while (p && p !== Object.prototype) {
        for (const key of Object.getOwnPropertyNames(p)) {
          const d = Object.getOwnPropertyDescriptor(p, key);
          if (!d || !d.set || !d.configurable) continue;
          try {
            Object.defineProperty(proto, key, {
              configurable: true, enumerable: d.enumerable,
              get: d.get,
              set: function (v) {
                begin();
                const t = now();
                d.set.call(this, v);
                S.cur.inJs += now() - t;
                S.cur.ops++;
                S.cur.names['=' + key] = (S.cur.names['=' + key] || 0) + 1;
              }
            });
          } catch (e) { }
        }
        p = Object.getPrototypeOf(p);
      }
      return true;
    }
    let tries = 0;
    const iv = setInterval(() => {
      if (install() || ++tries > 500) clearInterval(iv);
    }, 10);
    window.__spanDiag = () => ({ installed: !!window.__span, frames: window.__span ? window.__span.frames.length : -1, t: performance.now() });
  });

  await page.goto(`http://127.0.0.1:${port}/soui-wasm-demo.html${urlArg}`, { timeout: 30000 });
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
      await page.waitForTimeout(1200);
    }
  }

  // 清零统计窗口前的状态（确认 patch 真的装上、初始上屏确实走了 putImageData）
  console.log('[diag-before-clear]', JSON.stringify(await page.evaluate(() => window.__spanDiag())));
  // 启动序列（加载 -> 首帧 -> Lua 页面初始化）是确定性的，留一份快照做跨版本对照：
  // 交互式 wiggle 触发的重绘取决于点到哪个页签、Lua 页面在跑什么，跨次不可比。
  await page.evaluate(() => { window.__startup = window.__span.frames.slice(); window.__span.frames.length = 0; window.__span.cur = null; });

  // 持续制造重绘：鼠标在按钮上来回移动（hover 变化 -> 重绘），比等动画可靠。
  const wiggle = urlArg.indexOf('wiggle=1') >= 0;
  if (wiggle) {
    const box = await page.evaluate(() => {
      const r = document.querySelector('canvas').getBoundingClientRect();
      return { x: r.x, y: r.y, w: r.width, h: r.height };
    });
    // 点标签栏不同位置 -> 切页 -> 整窗重绘（比 hover 可靠：demo 皮肤没有 hover 态）
    const tabPts = [200, 320, 440, 560, 680, 140];
    const t1 = Date.now() + secs * 1000;
    let k = 0;
    while (Date.now() < t1) {
      const cx = tabPts[k++ % tabPts.length];
      await page.mouse.move(box.x + cx, box.y + 72);
      await page.mouse.down();
      await page.mouse.up();
      await page.waitForTimeout(150);
    }  } else {
    await page.waitForTimeout(secs * 1000);
  }

  const res = await page.evaluate(() => {
    const f = window.__startup || [];
    if (!f.length) return { n: 0 };
    const stat = (arr) => {
      const span = arr.map(x => x.t1 - x.t0), ops = arr.map(x => x.ops), injs = arr.map(x => x.inJs);
      const avg = a => a.reduce((s, v) => s + v, 0) / a.length;
      const sorted = span.slice().sort((a, b) => a - b);
      return {
        n: arr.length,
        opsAvg: +avg(ops).toFixed(1),
        spanAvg: +avg(span).toFixed(2),
        spanP50: +sorted[Math.floor(sorted.length / 2)].toFixed(2),
        spanP95: +sorted[Math.floor(sorted.length * 0.95)].toFixed(2),
        spanMax: +sorted[sorted.length - 1].toFixed(2),
        inJsAvg: +avg(injs).toFixed(2),
        gapPerOp_us: +(((avg(span) - avg(injs)) / Math.max(1, avg(ops))) * 1000).toFixed(2)
      };
    };
    // 直方图：按 ops 数量分段，看看哪些帧是"整窗重绘"
    const buckets = {};
    for (const x of f) {
      const k = x.ops >= 500 ? '500+' : x.ops >= 200 ? '200-499' : x.ops >= 50 ? '50-199' : '<50';
      (buckets[k] = buckets[k] || []).push(x);
    }
    const out = { all: stat(f), big: stat(f.filter(x => x.ops >= 200)) };
    for (const k of Object.keys(buckets)) out['b' + k] = stat(buckets[k]);
    // 大帧（整窗重绘）的方法直方图：按出现次数降序
    const hist = {};
    let bigN = 0;
    for (const x of f) {
      if (x.ops < 500) continue;
      ++bigN;
      for (const k in x.names) hist[k] = (hist[k] || 0) + x.names[k];
    }
    out.histBig = Object.entries(hist).sort((a, b) => b[1] - a[1])
      .map(([k, v]) => k + ':' + (v / Math.max(1, bigN)).toFixed(1)).join('  ');
    out.totalOps = f.reduce((s, x) => s + x.ops, 0);
    out.wallMs = +(f[f.length - 1].t1 - f[0].t0).toFixed(0);
    /* 启动序列里"整窗重绘"式的大帧总耗时（确定性负载，用于跨版本对比） */
    const bigs = f.filter(x => x.ops >= 500);
    out.bigSpanSum = +bigs.reduce((s, x) => s + (x.t1 - x.t0), 0).toFixed(1);
    out.bigInJsSum = +bigs.reduce((s, x) => s + x.inJs, 0).toFixed(1);
    out.nBig = bigs.length;
    return out;
  });
  console.log(JSON.stringify(res, null, 1));
  await browser.close();
  server.close();
})();
