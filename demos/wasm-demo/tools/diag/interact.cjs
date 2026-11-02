/* 交互式探针：切标签 → 按画布坐标点击 → 在指定时刻截帧。
 *
 * 用途：验证"点了按钮之后画面到底动了没有"。cmp_shot 只截静态页，动画类缺陷
 * （补间 / 值动画 / 模态遮罩）必须看时间序列。
 *
 * 用法：
 *   node tools/diag/interact.cjs <url> <tabIndex|-> [step ...]
 *   step 形式：
 *     click:X,Y        在画布坐标 (X,Y) 点击（800x600 逻辑像素）
 *     wait:MS          等 MS 毫秒
 *     shot:NAME        截一帧到 build/cmp/live/NAME.png
 *     eval:JS          在页面里求值并打印结果（用于读 window.__x 之类）
 *
 * 例（动画页，点第一个按钮，连拍 6 帧）：
 *   node tools/diag/interact.cjs http://127.0.0.1:8125/soui-wasm-demo.html 11 \
 *     click:400,150 wait:60 shot:a0 wait:250 shot:a1 wait:250 shot:a2
 *
 * 产物目录可用 LIVE_DIR 覆盖。
 */
'use strict';
const fs = require('fs');
const http = require('http');
const path = require('path');

const PORT = 9222;
const PAGE = process.argv[2];
if (!PAGE) {
  console.error('usage: interact.cjs <url> <tabIndex|-> [steps...]');
  process.exit(1);
}
const TAB = process.argv[3] === '-' ? -1 : Number(process.argv[3]);
const STEPS = process.argv.slice(4);
const OUTDIR = process.env.LIVE_DIR || path.join(__dirname, '..', '..', 'build', 'cmp', 'live');
if (STEPS.some((s) => s.startsWith('shot:'))) fs.mkdirSync(OUTDIR, { recursive: true });

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
  if (!target) {
    console.error('no page target');
    process.exit(2);
  }
  const ws = new WebSocket(target.webSocketDebuggerUrl);
  let id = 0;
  const pending = new Map();
  function send(method, params = {}, ms = 25000) {
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
  await send('Page.enable');
  await send('Network.enable');
  await send('Network.setCacheDisabled', { cacheDisabled: true });
  /* DSF 环境变量模拟 HiDPI（同 shoot_dpi.cjs）：不设则 1（默认逻辑分辨率） */
  await send('Emulation.setDeviceMetricsOverride', {
    width: 1000,
    height: 1400,
    deviceScaleFactor: parseFloat(process.env.DSF || '1'),
    mobile: false,
  });

  const url = PAGE + '?t=' + Date.now();
  console.log('NAV ' + url);
  await send('Page.navigate', { url }, 40000);

  const t0 = Date.now();
  let ready = false;
  while (Date.now() - t0 < 120000) {
    await new Promise((r) => setTimeout(r, 1200));
    const r = await send(
      'Runtime.evaluate',
      {
        expression:
          "(function(){var c=document.getElementById('screen'),ld=document.getElementById('loading');return JSON.stringify({cw:c?c.width:0,done:ld?ld.classList.contains('done'):null,failed:ld?ld.classList.contains('failed'):null});})()",
        returnByValue: true,
      },
      15000
    );
    if (r && r.__timeout) continue;
    const v = r && r.result && r.result.result ? r.result.result.value : null;
    if (!v) continue;
    const st = JSON.parse(v);
    console.log('STATUS ' + v + '  (' + ((Date.now() - t0) / 1000).toFixed(1) + 's)');
    if (st.failed) break;
    if (st.done && st.cw > 0) {
      ready = true;
      break;
    }
  }
  if (!ready) {
    console.log('NOT_READY');
    ws.close();
    process.exit(3);
  }
  await new Promise((r) => setTimeout(r, 2000));

  const box = await send('Runtime.evaluate', {
    expression:
      "(function(){var c=document.getElementById('screen');var r=c.getBoundingClientRect();return JSON.stringify({x:r.left,y:r.top,w:r.width,h:r.height,cw:c.width,ch:c.height,lw:(typeof Module!=='undefined'&&Module._SwinxFbLogicalWidth)?Module._SwinxFbLogicalWidth():c.width,lh:(typeof Module!=='undefined'&&Module._SwinxFbLogicalHeight)?Module._SwinxFbLogicalHeight():c.height});})()",
    returnByValue: true,
  });
  const b = JSON.parse(box.result.result.value);
  console.log('CANVAS ' + JSON.stringify(b));

  /* 步骤坐标是**应用逻辑坐标**（800x600 口径，与 dpr 无关）。canvas 的 CSS 尺寸
   * 等于逻辑尺寸（soui.html 按逻辑值设 style），所以换算用 CSS 宽（b.w），不能
   * 用 canvas 的 width 属性（b.cw 是物理像素）—— dpr!=1 时按物理换算会把所有
   * 点击等比缩小，页签/棋盘全部点偏。 */
  async function clickAt(cx, cy) {
    const sx = b.x + (cx / b.lw) * b.w;
    const sy = b.y + (cy / b.lh) * b.h;
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
  }

  async function shoot(name) {
    const du = await send(
      'Runtime.evaluate',
      {
        expression:
          "(function(){var c=document.getElementById('screen');try{return c.toDataURL('image/png');}catch(e){return 'ERR:'+e.message;}})()",
        returnByValue: true,
      },
      60000
    );
    const val = du && du.result && du.result.result ? du.result.result.value : undefined;
    if (typeof val === 'string' && val.startsWith('data:image/png;base64,')) {
      const buf = Buffer.from(val.split(',')[1], 'base64');
      const fp = path.join(OUTDIR, name + '.png');
      fs.writeFileSync(fp, buf);
      console.log('WROTE ' + fp + ' ' + buf.length);
    } else {
      console.log('NO_DATA ' + name);
    }
  }

  /* 切到指定标签：读 cmp_shot 写出的 tabs.json（各标签中心 x），点该行。 */
  if (TAB >= 0) {
    const tabsFile = path.join(__dirname, '..', '..', 'build', 'cmp', 'canvas2d', 'tabs.json');
    const tabs = JSON.parse(fs.readFileSync(tabsFile, 'utf8'));
    const cy = Number(process.env.TAB_Y || 72);
    console.log('TABCLICK #' + TAB + ' ' + JSON.stringify(tabs[TAB]));
    await clickAt(tabs[TAB].cx, cy);
    await new Promise((r) => setTimeout(r, 1500));
  }

  for (const step of STEPS) {
    const i = step.indexOf(':');
    const op = i < 0 ? step : step.slice(0, i);
    const arg = i < 0 ? '' : step.slice(i + 1);
    if (op === 'click') {
      const [x, y] = arg.split(',').map(Number);
      console.log('CLICK ' + x + ',' + y);
      await clickAt(x, y);
    } else if (op === 'down') {
      const [x, y] = arg.split(',').map(Number);
      const sx = b.x + (x / b.lw) * b.w;
      const sy = b.y + (y / b.lh) * b.h;
      await send('Input.dispatchMouseEvent', {
        type: 'mousePressed',
        x: sx,
        y: sy,
        button: 'left',
        clickCount: 1,
        buttons: 1,
      });
      console.log('DOWN ' + x + ',' + y);
    } else if (op === 'move') {
      const [x, y] = arg.split(',').map(Number);
      const sx = b.x + (x / b.lw) * b.w;
      const sy = b.y + (y / b.lh) * b.h;
      await send('Input.dispatchMouseEvent', {
        type: 'mouseMoved',
        x: sx,
        y: sy,
        button: 'left',
        buttons: 1,
      });
    } else if (op === 'up') {
      const [x, y] = arg.split(',').map(Number);
      const sx = b.x + (x / b.lw) * b.w;
      const sy = b.y + (y / b.lh) * b.h;
      await send('Input.dispatchMouseEvent', {
        type: 'mouseReleased',
        x: sx,
        y: sy,
        button: 'left',
        clickCount: 1,
        buttons: 0,
      });
    } else if (op === 'key') {
      /* key:<combo> —— 组合键，+ 连接：key:Ctrl+A  key:Shift+End  key:a
       * 修饰键映射 ControlLeft/ShiftLeft，普通字符用 KeyX / 单字符 code。
       * 页面侧 vkFromEvent 按 e.code 翻 VK，所以 code 必须给对。 */
      const MODS = { ctrl: ['ControlLeft', 'Control', 17, 2], shift: ['ShiftLeft', 'Shift', 16, 8], alt: ['AltLeft', 'Alt', 18, 1] };
      const parts = arg.split('+').map((s) => s.trim());
      const mods = [];
      let main = null;
      for (const p of parts) {
        const m = MODS[p.toLowerCase()];
        if (m) mods.push(m);
        else main = p;
      }
      async function keyEv(type, code, key, vk, text, modifiers) {
        const params = { type, code, key, windowsVirtualKeyCode: vk, modifiers: modifiers || 0 };
        if (text) { params.text = text; params.unmodifiedText = text; }
        await send('Input.dispatchKeyEvent', params);
      }
      for (const [code, key, vk] of mods) await keyEv('keyDown', code, key, vk, '');
      if (main) {
        let code, key, vk, text;
        if (main.length === 1 && /[a-zA-Z]/.test(main)) {
          code = 'Key' + main.toUpperCase(); key = main.toLowerCase(); vk = main.toUpperCase().charCodeAt(0);
          text = /[A-Z]/.test(main) && mods.length ? main : main.toLowerCase();
        } else if (main.length === 1) {
          code = ''; key = main; vk = main.toUpperCase().charCodeAt(0); text = main;
        } else {
          code = main; key = main; vk = 0; text = '';
        }
        const mbits = mods.reduce((a, m) => a | m[3], 0);
        if (vk === 0) {
          /* 命名键（End/Home/Left...）：只发 keyDown/keyUp，vk 由页面按 code 翻 */
          await keyEv('keyDown', code, key, 0, '');
          await keyEv('keyUp', code, key, 0, '');
        } else {
          await keyEv('keyDown', code, key, vk, text, mbits);
          await keyEv('keyUp', code, key, vk, text, mbits);
        }
      }
      for (const [code, key, vk] of mods.reverse()) await keyEv('keyUp', code, key, vk, '');
      console.log('KEY ' + arg);
    } else if (op === 'wait') {
      await new Promise((r) => setTimeout(r, Number(arg) || 0));
    } else if (op === 'shot') {
      await shoot(arg);
    } else if (op === 'eval') {
      const r = await send('Runtime.evaluate', { expression: arg, returnByValue: true, awaitPromise: true });
      const v = r && r.result && r.result.result ? r.result.result.value : JSON.stringify(r).slice(0, 300);
      console.log('EVAL ' + (typeof v === 'string' ? v : JSON.stringify(v)));
    } else {
      console.log('SKIP ' + step);
    }
  }

  ws.close();
  process.exit(0);
})();
