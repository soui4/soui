/* 从 canvas 像素里量出标签文字的分段，输出每个标签的中心 x（canvas 内坐标）。
 * 直接 getImageData 分析标签条那一行的"墨迹列"，按间隔切段，不用靠猜。
 *
 * 用法：node build/tab_xs.cjs
 */
'use strict';
const http = require('http');

const PORT = 9222;

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
  function send(method, params = {}, ms = 30000) {
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
  await new Promise((res, rej) => { ws.onopen = res; ws.onerror = rej; });
  await send('Runtime.enable');

  const expr = `(function(){
    var c=document.getElementById('screen');
    var x=c.getContext('2d');
    var y0=56, h=16;
    var d=x.getImageData(0,y0,c.width,h).data;
    var W=c.width, ink=new Uint8Array(W);
    for(var yy=0;yy<h;yy++)for(var xx=0;xx<W;xx++){
      var i=(yy*W+xx)*4;
      var lum=(d[i]*299+d[i+1]*587+d[i+2]*114)/1000;
      if(lum<140) ink[xx]=1;
    }
    var segs=[],st=-1,gap=0;
    for(var xx2=0;xx2<W;xx2++){
      if(ink[xx2]){ if(st<0)st=xx2; gap=0; }
      else if(st>=0){ gap++; if(gap>=6){ segs.push([st,xx2-gap]); st=-1; gap=0; } }
    }
    if(st>=0) segs.push([st,W-1]);
    return JSON.stringify(segs.map(function(s){return {a:s[0],b:s[1],cx:(s[0]+s[1])>>1};}));
  })()`;

  const r = await send('Runtime.evaluate', { expression: expr, returnByValue: true });
  const v = r && r.result && r.result.result ? r.result.result.value : null;
  console.log('SEGS ' + v);
  ws.close();
  process.exit(0);
})();
