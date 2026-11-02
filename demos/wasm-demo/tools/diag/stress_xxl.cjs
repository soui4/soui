/* 消消乐压测：进 Lua 游戏页后连点棋盘格子，捕获控制台错误/异常/崩溃。
 * 用法：node tools/diag/stress_xxl.cjs [点击次数=60] [间隔ms=120] [dpr=1]
 * 前置：8125 静态服务 + 9222 CDP（headless Edge）。
 */
'use strict';
const fs = require('fs');
const http = require('http');

function getJson(url) {
  return new Promise((res, rej) => {
    http.get(url, (r) => { let d = ''; r.on('data', (c) => (d += c)); r.on('end', () => { try { res(JSON.parse(d)); } catch (e) { rej(e); } }); }).on('error', rej);
  });
}
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

(async () => {
  const N = parseInt(process.argv[2] || '60', 10);
  const GAP = parseInt(process.argv[3] || '120', 10);
  const DSF = parseFloat(process.argv[4] || '1');

  const list = await getJson('http://127.0.0.1:9222/json/list');
  const target = list.find((t) => t.type === 'page');
  if (!target) { console.error('no page target'); process.exit(2); }
  const ws = new WebSocket(target.webSocketDebuggerUrl);
  let id = 0;
  const pending = new Map();
  function send(method, params = {}) {
    const _id = ++id;
    const p = new Promise((r) => pending.set(_id, r));
    ws.send(JSON.stringify({ id: _id, method, params }));
    return p;
  }
  const errors = [];
  const conlog = [];
  ws.addEventListener('message', (ev) => {
    const m = JSON.parse(ev.data);
    if (m.id && pending.has(m.id)) { pending.get(m.id)(m); pending.delete(m.id); return; }
    if (m.method === 'Runtime.exceptionThrown') {
      const d = m.params.exceptionDetails;
      errors.push('EXC: ' + (d.exception && (d.exception.description || d.exception.value) || d.text));
    } else if (m.method === 'Runtime.consoleAPICalled') {
      const line = m.params.type + ': ' + m.params.args.map((a) => a.value || a.description || '').join(' ');
      conlog.push(line);
      if (['error', 'warning'].includes(m.params.type)) errors.push(line);
    } else if (m.method === 'Log.entryAdded') {
      const e = m.params.entry;
      conlog.push('LOG:' + e.level + ': ' + (e.text || ''));
      if (e.level === 'error') errors.push('LOG: ' + (e.text || '').slice(0, 240));
    }
  });
  ws.addEventListener('close', () => {
    console.log('=== WS CLOSED (renderer crashed?) clicked=' + clicked + ' ===');
    conlog.slice(-15).forEach((l) => console.log('  | ' + l));
    process.exit(1);
  });
  await new Promise((r) => ws.addEventListener('open', r, { once: true }));

  await send('Runtime.enable');
  await send('Page.enable');
  await send('Log.enable');
  await send('Emulation.setDeviceMetricsOverride', { width: 900, height: 700, deviceScaleFactor: DSF, mobile: false });
  await send('Page.navigate', { url: 'http://127.0.0.1:8125/soui-wasm-demo.html' });
  await sleep(8000);

  // canvas 在页面里的 CSS 位置与逻辑尺寸（点击坐标按逻辑 → CSS 1:1 加偏移）
  const boxR = await send('Runtime.evaluate', {
    expression: "(function(){var c=document.getElementById('screen');var r=c.getBoundingClientRect();return JSON.stringify({x:r.left,y:r.top,w:r.width,h:r.height});})()",
    returnByValue: true,
  });
  const box = JSON.parse(boxR.result.result.value);
  const clickL = async (lx, ly) => {
    const x = box.x + lx, y = box.y + ly;
    await send('Input.dispatchMouseEvent', { type: 'mousePressed', x, y, button: 'left', clickCount: 1 });
    await send('Input.dispatchMouseEvent', { type: 'mouseReleased', x, y, button: 'left', clickCount: 1 });
  };

  // 进 Lua 游戏页（顶栏页签 "Lua 游戏" 逻辑坐标 585,73）
  await clickL(585, 73);
  await sleep(900);

  let clicked = 0;
  for (let i = 0; i < N; i++) {
    // 场景：每 12 步按一次「重新开始」(218,240)，每 17 步按「洗牌」(243,282)，
    // 其余随机点棋盘格子 —— 覆盖"动画进行中重建棋盘/飞行副本"的危险路径
    let x, y;
    const phase = i % 40 === 25 ? 'restart' : (i % 53 === 9 ? 'shuffle' : 'board');
    if (phase === 'restart') { x = 218; y = 240; }
    else if (phase === 'shuffle') { x = 243; y = 282; }
    else {
      // 相邻对点击：偶数步选 (col,row)，奇数步点它右边一格 → 触发真实交换/消除
      const pair = Math.floor(i / 2);
      const row = pair % 8;
      const col = (pair * 3) % 7; // 0..6，保证右边一列存在
      const cc = i % 2 === 0 ? col : col + 1;
      x = Math.max(400, Math.min(690, 409 + cc * 38 + 19));
      y = Math.max(210, Math.min(520, 238 + row * 38 + 19));
    }
    // hover 扫掠：真实用户会在棋盘上滑动鼠标（触发 hover 失效/重绘）
    const mx = 400 + ((i * 37) % 290), my = 210 + ((i * 53) % 310);
    await send('Input.dispatchMouseEvent', { type: 'mouseMoved', x: box.x + mx, y: box.y + my });
    await clickL(x, y);
    clicked++;
    await sleep(GAP);
    if (i % 10 === 9) {
      const r = await send('Runtime.evaluate', { expression: '2+3', returnByValue: true }).catch(() => null);
      const v = r && r.result && r.result.result ? r.result.result.value : null;
      console.log('progress: ' + (i + 1) + '/' + N + ' clicks, errors=' + errors.length + ' state=' + v);
      if (v !== 5) {
        console.log('=== DEAD after ' + clicked + ' clicks ===');
        conlog.slice(-20).forEach((l) => console.log('  | ' + l));
        process.exit(1);
      }
    }
  }
  await sleep(2000);
  // 末态截图（确认游戏真的在玩：得分/金币/棋盘变化）
  try {
    const shot = await send('Page.captureScreenshot', { format: 'png' });
    const data = shot && shot.result && shot.result.data;
    if (data) {
      fs.mkdirSync('build/dpi', { recursive: true });
      fs.writeFileSync('build/dpi/stress_final.png', Buffer.from(data, 'base64'));
      console.log('WROTE build/dpi/stress_final.png');
    }
  } catch (e) { /* ignore */ }
  console.log('=== DONE clicked=' + clicked + ' errors=' + errors.length + ' ===');
  errors.slice(0, 20).forEach((e) => console.log(e));
  if (errors.length) {
    console.log('--- last console ---');
    conlog.slice(-20).forEach((l) => console.log('  | ' + l));
  }
  process.exit(errors.length ? 1 : 0);
})().catch((e) => { console.error('script fail:', e); process.exit(2); });
