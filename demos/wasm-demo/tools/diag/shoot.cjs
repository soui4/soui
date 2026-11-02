/* 在 headless Edge 里重新加载 demo 页并截图（route C canvas2d 后端验收）。
 *
 * 用 CDP 而不是 --virtual-time-budget：demo 的主循环每帧真实忙等约 100ms，
 * 虚拟时间永远推不到预算，无头截图会超时。
 *
 * 用法：node build/shoot.cjs [等待毫秒]
 */
'use strict';
const fs = require('fs');
const http = require('http');
const path = require('path');

const PORT = 9222;
const PAGE = process.env.DEMO_URL || 'http://127.0.0.1:8123/soui-wasm-demo.html';
const WAIT = parseInt(process.argv[2] || '18000', 10);
const SHOT_DIR = process.env.SHOT_DIR || path.join(__dirname, '..', '..', 'build');
fs.mkdirSync(SHOT_DIR, { recursive: true });
const OUT = path.join(SHOT_DIR, 'shot_routec.png');

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
  let list = [];
  try {
    list = await getJson(`http://127.0.0.1:${PORT}/json/list`);
  } catch (e) {
    console.error('CDP 不可达：' + e.message);
    process.exit(2);
  }
  const target = list.find((t) => t.type === 'page');
  if (!target) {
    console.error('没有 page target：' + JSON.stringify(list));
    process.exit(2);
  }
  console.log('TARGET ' + target.url);

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
  console.log('WS_OPEN');

  await send('Runtime.enable');
  await send('Page.enable');
  await send('Log.enable');
  /* .html 会用 ?t= 破缓存，但 .wasm/.data 是页面自己 fetch 的，URL 不带时间戳，
   * 不关缓存就会一直跑旧模块 —— 截图字节数一模一样就是这种"看着跑了其实没换"的坑。 */
  await send('Network.enable');
  await send('Network.setCacheDisabled', { cacheDisabled: true });

  const url = PAGE + '?t=' + Date.now();
  console.log('NAVIGATE ' + url);
  const nav = await send('Page.navigate', { url }, 30000);
  console.log('NAV ' + JSON.stringify(nav).slice(0, 200));

  /* 轮询页面状态：能拿到 canvas 尺寸 + 加载层消失就算就绪 */
  const t0 = Date.now();
  let st = null;
  while (Date.now() - t0 < WAIT) {
    await new Promise((r) => setTimeout(r, 1500));
    const r = await send(
      'Runtime.evaluate',
      {
        expression:
          "(function(){var l=document.getElementById('log'),ph=document.getElementById('load-phase'),c=document.getElementById('screen'),ld=document.getElementById('loading');return JSON.stringify({log:l?l.textContent:'',phase:ph?ph.textContent:'',cw:c?c.width:0,ch:c?c.height:0,loadingDone:ld?ld.classList.contains('done'):null,failed:ld?ld.classList.contains('failed'):null});})()",
        returnByValue: true,
      },
      12000
    );
    if (r && r.__timeout) {
      console.log('EVAL_TIMEOUT（主线程可能仍在忙）');
      continue;
    }
    const v = r && r.result && r.result.result ? r.result.result.value : null;
    if (v) {
      st = JSON.parse(v);
      console.log('STATUS ' + v);
      if (st.loadingDone && st.cw > 0) break;
      if (st.failed) break;
    }
  }

  /* 再多等几帧，让泵把若干帧画完 */
  await new Promise((r) => setTimeout(r, 3000));

  const du = await send(
    'Runtime.evaluate',
    {
      expression:
        "(function(){var c=document.getElementById('screen');if(!c)return '';try{return c.toDataURL('image/png');}catch(e){return 'ERR:'+e.message;}})()",
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
    console.log('NO_CANVAS_DATA ' + JSON.stringify(du).slice(0, 400));
  }

  ws.close();
  process.exit(0);
})();
