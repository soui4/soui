/* 快速崩溃检查：导航 → 等待 → 点击页签 → 心跳，捕获 console/异常/崩溃栈。
 * 用法：node tools/diag/crash_check.cjs [dpr=1.5]
 */
'use strict';
const http = require('http');
function getJson(url) {
  return new Promise((res, rej) => {
    http.get(url, (r) => { let d = ''; r.on('data', (c) => (d += c)); r.on('end', () => { try { res(JSON.parse(d)); } catch (e) { rej(e); } }); }).on('error', rej);
  });
}
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

(async () => {
  const DSF = parseFloat(process.argv[2] || '1.5');
  const list = await getJson('http://127.0.0.1:9222/json/list');
  const target = list.find((t) => t.type === 'page');
  const ws = new WebSocket(target.webSocketDebuggerUrl);
  let id = 0;
  const pending = new Map();
  function send(method, params = {}) {
    const _id = ++id;
    const p = new Promise((r) => pending.set(_id, r));
    ws.send(JSON.stringify({ id: _id, method, params }));
    return p;
  }
  const logs = [];
  ws.addEventListener('message', (ev) => {
    const m = JSON.parse(ev.data);
    if (m.id && pending.has(m.id)) { pending.get(m.id)(m); pending.delete(m.id); return; }
    if (m.method === 'Runtime.exceptionThrown') {
      const d = m.params.exceptionDetails;
      logs.push('EXC: ' + (d.exception && (d.exception.description || d.exception.value) || d.text));
    } else if (m.method === 'Runtime.consoleAPICalled') {
      logs.push(m.params.type + ': ' + m.params.args.map((a) => a.value || a.description || '').join(' ').slice(0, 400));
    }
  });
  ws.addEventListener('close', () => { console.log('--- WS CLOSED ---'); logs.slice(-10).forEach((l) => console.log('  | ' + l)); process.exit(1); });
  await new Promise((r) => ws.addEventListener('open', r, { once: true }));

  await send('Runtime.enable');
  await send('Page.enable');
  await send('Emulation.setDeviceMetricsOverride', { width: 900, height: 700, deviceScaleFactor: DSF, mobile: false });
  await send('Page.navigate', { url: 'http://127.0.0.1:8125/soui-wasm-demo.html' });
  await sleep(9000);

  let r = await send('Runtime.evaluate', { expression: 'window.__hb=1; window.__hb', returnByValue: true });
  console.log('heartbeat1:', JSON.stringify(r.result && r.result.result));

  await send('Input.dispatchMouseEvent', { type: 'mousePressed', x: 585, y: 73, button: 'left', clickCount: 1 });
  await send('Input.dispatchMouseEvent', { type: 'mouseReleased', x: 585, y: 73, button: 'left', clickCount: 1 });
  await sleep(1500);

  r = await send('Runtime.evaluate', { expression: 'window.__hb+1', returnByValue: true });
  console.log('heartbeat2:', JSON.stringify(r.result && r.result.result));
  await sleep(500);
  console.log('=== logs (' + logs.length + ') ===');
  logs.slice(-15).forEach((l) => console.log('  | ' + l));
  process.exit(0);
})().catch((e) => { console.error('fail:', e); process.exit(2); });
