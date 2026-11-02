/* 逐个标签截图：自动从 canvas 的 fillText 调用里认出标签条上的标签文字，
 * 得到每个标签的中心 x，再依次点击 + 截图。两个版本字体度量不同也能各自对准。
 *
 * 用法：node tools/diag/cmp_shot.cjs <baseUrl> <tag>
 *   node tools/diag/cmp_shot.cjs http://127.0.0.1:8124/soui-wasm-demo.html cairo
 *   node tools/diag/cmp_shot.cjs http://127.0.0.1:8123/soui-wasm-demo.html canvas2d
 *
 * 产物：build/cmp/<tag>/00.png ... 以及 build/cmp/<tag>/tabs.json
 */
'use strict';
const fs = require('fs');
const http = require('http');
const path = require('path');

const PORT = 9222;
const PAGE = process.argv[2] || 'http://127.0.0.1:8124/soui-wasm-demo.html';
const TAG = process.argv[3] || 'ref';
const OUTDIR = process.env.SHOT_DIR || path.join(__dirname, '..', '..', 'build', 'cmp', TAG);
fs.mkdirSync(OUTDIR, { recursive: true });

/* 记录每次 fillText 的文本与坐标；用 key 去重，避免每帧累积成山。 */
const HOOK = `
(function(){
  var P = CanvasRenderingContext2D.prototype;
  var orig = P.fillText;
  window.__txt = window.__txt || Object.create(null);
  window.__txtN = 0;
  P.fillText = function(t, x, y){
    try {
      if (window.__txtN < 60000) {
        var k = t + '|' + Math.round(x) + '|' + Math.round(y);
        var e = window.__txt[k];
        if (e) { e.n++; e.w = Math.max(e.w, this.measureText(t).width); }
        else { window.__txt[k] = {t: String(t), x: x, y: y, w: this.measureText(t).width, n: 1}; window.__txtN++; }
      }
    } catch(e){}
    return orig.apply(this, arguments);
  };
})();
`;

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
  /* headless 的 --window-size 不生效（实测视口只有 500x320），canvas 又排在标题和
   * 说明文字下面（top≈681），点击坐标会直接落到视口外 —— pointerdown 一次都收不到,
   * 症状是"所有标签截图字节数完全一样"。这里强制一个够大的视口，并保证 canvas 是
   * 1:1 显示（实测 1000 宽的视口下 canvas 正好 800x600 CSS 像素）。 */
  await send('Emulation.setDeviceMetricsOverride', {
    width: 1000,
    height: 1400,
    deviceScaleFactor: 1,
    mobile: false,
  });
  await send('Page.addScriptToEvaluateOnNewDocument', { source: HOOK });

  const url = PAGE + '?t=' + Date.now();
  console.log('NAV ' + url);
  await send('Page.navigate', { url }, 40000);

  /* 等就绪：canvas 有尺寸 + 加载层 done。参考版 .data 20MB，给足时间。 */
  const t0 = Date.now();
  let ready = false;
  while (Date.now() - t0 < 120000) {
    await new Promise((r) => setTimeout(r, 1500));
    const r = await send(
      'Runtime.evaluate',
      {
        expression:
          "(function(){var c=document.getElementById('screen'),ld=document.getElementById('loading');return JSON.stringify({cw:c?c.width:0,ch:c?c.height:0,done:ld?ld.classList.contains('done'):null,failed:ld?ld.classList.contains('failed'):null});})()",
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
  await new Promise((r) => setTimeout(r, 2500));

  /* 定位标签：两条路。
   * ① canvas2d 后端文本走 fillText（带 TextDecoder），能直接从调用记录里拿到标签文字
   *    与宽度，语义最准；
   * ② cairo 后端是把整帧画进位图再 putImageData，压根不调 fillText —— 只能回到像素：
   *    在标签条那一带做"墨迹列"投影，按间隙切段。两版字体不同、标签宽度也不同，
   *    像素法反而更通用。 */
  const q = await send(
    'Runtime.evaluate',
    {
      expression: `(function(){
        var all = Object.keys(window.__txt || {}).map(function(k){return window.__txt[k];});
        var band = all.filter(function(e){return e.y >= 40 && e.y <= 110;});
        var g = {};
        band.forEach(function(e){ var yy = Math.round(e.y); (g[yy] = g[yy] || []).push(e); });
        var best = null, bestN = 0;
        Object.keys(g).forEach(function(k){ if (g[k].length > bestN) { bestN = g[k].length; best = g[k]; } });
        if (best && best.length >= 6) {
          var row = best.slice().sort(function(a,b){return a.x - b.x;});
          return JSON.stringify({src:'text', y: row[0].y, row: row.map(function(e){return {t:e.t, cx: Math.round(e.x + e.w/2)};})});
        }
        /* 像素法 */
        var c=document.getElementById('screen'), ctx=c.getContext('2d');
        var W=c.width, H=Math.min(c.height,140);
        var d=ctx.getImageData(0,0,W,H).data;
        var inkRow=new Int32Array(H);
        for(var yy=0;yy<H;yy++){var n=0;for(var xx=0;xx<W;xx++){var i=(yy*W+xx)*4;var lum=(d[i]*299+d[i+1]*587+d[i+2]*114)/1000;if(lum<150)n++;}inkRow[yy]=n;}
        /* 标签条：取 y∈[45,120] 里 ink 最多的连续 14 行作为文字带 */
        var y0=45,y1=120,bestS=-1,bestY=45;
        for(var s=y0;s<=y1-14;s++){var sum=0;for(var k=0;k<14;k++)sum+=inkRow[s+k];if(sum>bestS){bestS=sum;bestY=s;}}
        var ys=bestY, ye=bestY+14;
        var ink=new Uint8Array(W);
        for(var yy2=ys;yy2<ye;yy2++)for(var xx2=0;xx2<W;xx2++){var i2=(yy2*W+xx2)*4;var l2=(d[i2]*299+d[i2+1]*587+d[i2+2]*114)/1000;if(l2<150)ink[xx2]=1;}
        var segs=[],st=-1,gap=0;
        for(var xx3=0;xx3<W;xx3++){
          if(ink[xx3]){ if(st<0)st=xx3; gap=0; }
          else if(st>=0){ gap++; if(gap>=7){ segs.push([st,xx3-gap]); st=-1; gap=0; } }
        }
        if(st>=0) segs.push([st,W-1]);
        segs = segs.filter(function(s){return s[1]-s[0] >= 4;});
        return JSON.stringify({src:'pixel', y: Math.round((ys+ye)/2), row: segs.map(function(s,i){return {t:'#'+i, cx:(s[0]+s[1])>>1};})});
      })()`,
      returnByValue: true,
    },
    40000
  );
  const info = JSON.parse(q.result.result.value);
  console.log('TABSRC ' + info.src + '  y=' + info.y + '  count=' + info.row.length);
  console.log('TABS ' + JSON.stringify(info.row.map((e) => ({ t: e.t, cx: e.cx }))));
  if (!info.row.length) {
    console.log('NO_TABS');
    ws.close();
    process.exit(4);
  }

  /* canvas 在页面里的显示矩形（CSS 缩放 = devicePixelRatio） */
  const box = await send('Runtime.evaluate', {
    expression:
      "(function(){var c=document.getElementById('screen');var r=c.getBoundingClientRect();return JSON.stringify({x:r.left,y:r.top,w:r.width,h:r.height,cw:c.width,ch:c.height});})()",
    returnByValue: true,
  });
  const b = JSON.parse(box.result.result.value);
  console.log('CANVAS ' + JSON.stringify(b));

  const tabs = info.row.map((e) => ({ t: e.t, cx: e.cx }));
  fs.writeFileSync(path.join(OUTDIR, 'tabs.json'), JSON.stringify(tabs, null, 1));

  async function shoot(file) {
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
      fs.writeFileSync(file, buf);
      console.log('WROTE ' + path.basename(file) + ' ' + buf.length);
    } else {
      console.log('NO_DATA ' + file + ' ' + JSON.stringify(du).slice(0, 200));
    }
  }

  for (let i = 0; i < tabs.length; i++) {
    const cx = tabs[i].cx;
    const cy = info.y - 4; /* 文字基线往上 4px 仍在标签身上 */
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
    /* 让泵跑几帧 */
    await new Promise((r) => setTimeout(r, 1800));
    const name = String(i).padStart(2, '0');
    await shoot(path.join(OUTDIR, name + '.png'));
  }

  ws.close();
  process.exit(0);
})();
