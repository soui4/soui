/* 钩住 canvas 2D 的图元调用，记录参数在当前上下文里的真实取值：
 *   - 坐标是不是分数（亚像素 → Canvas 会抗锯齿，GDI 不会）
 *   - 调用时 globalAlpha 是多少（没被 restore 回去会整片变淡）
 *   - 变换矩阵的 e/f（视图原点/DPR 有没有偏移）
 *   - fillStyle / strokeStyle
 *
 * 用法：node tools/diag/hook_draw.cjs [采集毫秒]
 */
'use strict';
const http = require('http');
const PORT = 9222;
const MS = parseInt(process.argv[2] || '2500', 10);

const HOOK = `
(function(){
  var P = CanvasRenderingContext2D.prototype;
  window.__dr = [];
  function rec(kind, self, args){
    if (window.__dr.length > 40000) return;
    var t = self.getTransform ? self.getTransform() : {a:1,b:0,c:0,d:1,e:0,f:0};
    var nums = [];
    for (var i=0;i<args.length;i++) if (typeof args[i] === 'number') nums.push(args[i]);
    var frac = 0;
    for (var j=0;j<nums.length;j++) if (Math.abs(nums[j] - Math.round(nums[j])) > 1e-6) frac++;
    var fs = self.fillStyle, ss = self.strokeStyle;
    if (typeof fs !== 'string') fs = '[grad/pat]';
    if (typeof ss !== 'string') ss = '[grad/pat]';
    window.__dr.push({k:kind, n:nums, f:frac, a:self.globalAlpha, e:t.e, f2:t.f, s:fs, s2:ss});
  }
  ['fillRect','strokeRect','clearRect','fillText','strokeText'].forEach(function(name){
    var o = P[name];
    P[name] = function(){ rec(name, this, arguments); return o.apply(this, arguments); };
  });
  ['stroke','fill'].forEach(function(name){
    var o = P[name];
    P[name] = function(){ rec(name, this, arguments); return o.apply(this, arguments); };
  });
})();
`;

function getJson(url) {
  return new Promise((res, rej) => {
    http
      .get(url, (r) => {
        let d = '';
        r.on('data', (c) => (d += c));
        r.on('end', () => res(JSON.parse(d)));
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
  await send('Page.enable');
  await send('Network.enable');
  await send('Network.setCacheDisabled', { cacheDisabled: true });
  await send('Emulation.setDeviceMetricsOverride', { width: 1000, height: 1400, deviceScaleFactor: 1, mobile: false });
  await send('Page.addScriptToEvaluateOnNewDocument', { source: HOOK });
  await send('Page.navigate', { url: (process.env.DEMO_URL || 'http://127.0.0.1:8125/soui-wasm-demo.html') + '?t=' + Date.now() }, 40000);

  const t0 = Date.now();
  while (Date.now() - t0 < 90000) {
    await new Promise((r) => setTimeout(r, 1500));
    const r = await send('Runtime.evaluate', {
      expression: "(function(){var c=document.getElementById('screen'),l=document.getElementById('loading');return JSON.stringify({cw:c?c.width:0,done:l?l.classList.contains('done'):null,n:(window.__dr||[]).length});})()",
      returnByValue: true,
    });
    if (r && r.__timeout) continue;
    const v = r.result && r.result.result ? r.result.result.value : null;
    if (v) {
      const st = JSON.parse(v);
      console.log('STATUS ' + v);
      if (st.done && st.cw > 0 && st.n > 0) break;
    }
  }
  await new Promise((r) => setTimeout(r, MS));

  const out = await send('Runtime.evaluate', {
    expression: `(function(){
      var a = window.__dr || [];
      var byKind = {}, frac = 0, alpha = 0, transf = 0;
      a.forEach(function(e){
        byKind[e.k] = (byKind[e.k]||0)+1;
        if (e.f>0) frac++;
        if (Math.abs(e.a-1) > 1e-6) alpha++;
        if (Math.abs(e.e)>1e-6 || Math.abs(e.f2)>1e-6) transf++;
      });
      var exFrac = a.filter(function(e){return e.f>0;}).slice(0,10).map(function(e){return e.k+'('+e.n.join(',')+')';});
      var exAlpha = a.filter(function(e){return Math.abs(e.a-1)>1e-6;}).slice(0,10).map(function(e){return e.k+' a='+e.a.toFixed(3)+' '+e.s;});
      return JSON.stringify({total:a.length, byKind:byKind, frac:frac, alpha:alpha, transf:transf, exFrac:exFrac, exAlpha:exAlpha});
    })()`,
    returnByValue: true,
  });
  const v = out.result.result.value;
  const o = JSON.parse(v);
  console.log('TOTAL ' + o.total + '  byKind ' + JSON.stringify(o.byKind));
  console.log('分数坐标调用 ' + o.frac + '   非1 alpha ' + o.alpha + '   非零变换 ' + o.transf);
  console.log('分数样例:');
  o.exFrac.forEach((s) => console.log('   ' + s));
  console.log('alpha 样例:');
  o.exAlpha.forEach((s) => console.log('   ' + s));
  ws.close();
  process.exit(0);
})();
