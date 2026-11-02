/* 只截标签条那一条（页面坐标裁剪），用于放大检查小色块是 UI 元素还是渲染异常。 */
'use strict';
const fs = require('fs');
const http = require('http');
const path = require('path');
const PORT = 9222;
function getJson(u){return new Promise((res,rej)=>{http.get(u,r=>{let d='';r.on('data',c=>d+=c);r.on('end',()=>{try{res(JSON.parse(d));}catch(e){rej(e);}});}).on('error',rej);});}
(async()=>{
  const list = await getJson(`http://127.0.0.1:${PORT}/json/list`);
  const t = list.find(x=>x.type==='page' && /soui-wasm-demo/.test(x.url));
  if(!t){console.error('no target');process.exit(2);}
  const ws = new WebSocket(t.webSocketDebuggerUrl);
  let id=0; const pend=new Map();
  const send=(m,p={},ms=20000)=>{const _id=++id;const pr=new Promise(r=>pend.set(_id,r));ws.send(JSON.stringify({id:_id,method:m,params:p}));return Promise.race([pr,new Promise(r=>setTimeout(()=>r({__timeout:m}),ms))]);};
  ws.onmessage=e=>{const m=JSON.parse(e.data);if(m.id&&pend.has(m.id)){pend.get(m.id)(m);pend.delete(m.id);}};
  await new Promise((res,rej)=>{ws.onopen=res;ws.onerror=rej;});
  await send('Runtime.enable');
  const r = await send('Runtime.evaluate',{expression:"(function(){var c=document.getElementById('screen').getBoundingClientRect();return JSON.stringify({x:c.left,y:c.top,w:c.width,h:c.height});})()",returnByValue:true});
  const box = JSON.parse(r.result.result.value);
  console.log('canvas box', JSON.stringify(box));
  const shot = await send('Page.captureScreenshot',{format:'png',clip:{x:box.x,y:box.y+50,width:box.w,height:44,scale:2}},30000);
  if(shot && shot.result && shot.result.data){
    const b=Buffer.from(shot.result.data,'base64');
    fs.writeFileSync(path.join(process.env.SHOT_DIR || path.join(__dirname, '..', '..', 'build'), 'shot_tabs2.png'), b);
    console.log('WROTE shot_tabs2.png', b.length);
  } else console.log('NO_SHOT', JSON.stringify(shot).slice(0,300));
  ws.close(); process.exit(0);
})();
