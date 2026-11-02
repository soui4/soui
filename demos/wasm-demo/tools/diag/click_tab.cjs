/* 点一个标签页并截图，用于验证九宫格皮肤在其它页面的渲染。
 *
 * 用法：node build/click_tab.cjs <标签序号(0-based)> <输出文件名>
 * 标签条字号 14px，逐个标签的 x 位置靠 SOUI 自己算，这里按 demo 的固定布局量：
 * 直接用 canvas 内坐标点击，再换算到视口坐标交给 Input.dispatchMouseEvent。
 */
'use strict';
const fs = require('fs');
const http = require('http');
const path = require('path');

const PORT = 9222;
const PAGE = process.env.DEMO_URL || 'http://127.0.0.1:8123/soui-wasm-demo.html';
const IDX = parseInt(process.argv[2] || '1', 10);
const SHOT_DIR = process.env.SHOT_DIR || path.join(__dirname, '..', '..', 'build');
fs.mkdirSync(SHOT_DIR, { recursive: true });
const OUT = path.join(SHOT_DIR, process.argv[3] || 'shot_tab.png');

function getJson(url) {
  return new Promise((res, rej) => {
    http
      .get(url, (r) => {
        let d = '';
        r.on('data', (c) => (d += c));
        r.on('end', () => {
          try {
            res(JSON.parse(d));
          } catch (e) {
            rej(e);
          }
        });
      })
      .on('error', rej);
  });
}

(async () => {
  const list = await getJson(`http://127.0.0.1:${PORT}/json/list`);
  const target = list.find((t) => t.type === 'page');
  const ws = new WebSocket(target.webSocketDebuggerUrl);
  let id = 0;
  const pending = new Map();
  function send(method, params = {}, ms = 20000) {
    const _id = ++id;
    const p = new Promise((r) => pending.set(_id, r));
    ws.send(JSON.stringify({ id: _id, method, params }));
    return Promise.race([p, new Promise((r) => setTimeout(() => r({ __timeout: method }), ms))]);
  }
  ws.onmessage = (ev) => {
    const m = JSON.parse(ev.data);
    if (m.id && pending.has(m.id)) {
      pending.get(m.id)(m);
      pending.delete(m.id);
    }
  };
  await new Promise((res, rej) => {
    ws.onopen = res;
    ws.onerror = rej;
  });
  await send('Runtime.enable');

  /* 标签条在 y≈56..74（demo 固定布局），第 i 个标签中心按下表；这里改成让页面
   * 自己报 canvas 位置，点击点 = canvas 左上 + 标签内坐标。 */
  const box = await send('Runtime.evaluate', {
    expression:
      "(function(){var c=document.getElementById('screen');var r=c.getBoundingClientRect();return JSON.stringify({x:r.left,y:r.top,w:r.width,h:r.height,cw:c.width,ch:c.height});})()",
    returnByValue: true,
  });
  const b = JSON.parse(box.result.result.value);
  console.log('canvas box ' + JSON.stringify(b));

  /* 标签条的 15 个标签宽度不等，按经验给一组中心 x（canvas 内坐标）。
   * 概览/基础控件/更多控件/MVC/列表与树/布局/图形控件/表格/图表/编辑器/
   * 扩展控件/动画/Lua 游戏/模态表单 —— 从截图量出来的累积位置。 */
  const TABX = [45, 104, 175, 240, 300, 355, 410, 470, 515, 563, 625, 680, 725, 775];
  /* TAB_X 可显式给 canvas 内 x，用来精确定位某个标签 */
  const cx = process.env.TAB_X ? parseFloat(process.env.TAB_X) : TABX[Math.min(IDX, TABX.length - 1)];
  const cy = process.env.CLICK_Y ? parseFloat(process.env.CLICK_Y) : 64; // 默认标签条垂直中心
  const sx = b.x + (cx / b.cw) * b.w;
  const sy = b.y + (cy / b.ch) * b.h;

  for (const type of ['mousePressed', 'mouseReleased']) {
    await send('Input.dispatchMouseEvent', {
      type,
      x: sx,
      y: sy,
      button: 'left',
      clickCount: 1,
      buttons: type === 'mousePressed' ? 1 : 0,
    });
  }
  await new Promise((r) => setTimeout(r, 2500));

  const du = await send(
    'Runtime.evaluate',
    {
      expression:
        "(function(){var c=document.getElementById('screen');try{return c.toDataURL('image/png');}catch(e){return 'ERR:'+e.message;}})()",
      returnByValue: true,
    },
    40000
  );
  const val = du && du.result && du.result.result ? du.result.result.value : undefined;
  if (typeof val === 'string' && val.startsWith('data:image/png;base64,')) {
    const buf = Buffer.from(val.split(',')[1], 'base64');
    fs.writeFileSync(OUT, buf);
    console.log('WROTE ' + OUT + ' bytes=' + buf.length);
  } else {
    console.log('NO_CANVAS_DATA ' + JSON.stringify(du).slice(0, 300));
  }
  ws.close();
  process.exit(0);
})();
