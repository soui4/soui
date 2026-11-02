/* 解析 wasm 的 FUNCTION/CODE/IMPORT 段，给出每个函数索引的函数体在文件里的偏移，
 * 用来标定 node 栈帧里 `wasm-function[N]:0xOFF` 的 OFF 究竟是什么坐标系。 */
'use strict';
const fs = require('fs');

const file = process.argv[2];
const buf = fs.readFileSync(file);

function leb(b, p) {
  let r = 0, s = 0, c;
  do {
    c = b[p++];
    r |= (c & 0x7f) << s;
    s += 7;
  } while (c & 0x80);
  return [r >>> 0, p];
}

let p = 8; /* magic + version */
const secs = {};
while (p < buf.length) {
  const id = buf[p++];
  let size;
  [size, p] = leb(buf, p);
  secs[id] = { start: p, size };
  p += size;
}

/* IMPORT 段：数一下有多少个 func import（函数索引从这里开始） */
let pImp = secs[2].start; /* 2 = IMPORT 段（1 是 TYPE） */
let nImp;
[nImp, pImp] = leb(buf, pImp);
let funcImports = 0;
for (let i = 0; i < nImp; i++) {
  let l;
  [l, pImp] = leb(buf, pImp);
  pImp += l; /* module */
  [l, pImp] = leb(buf, pImp);
  pImp += l; /* name */
  const kind = buf[pImp++];
  if (kind === 0) {
    let t;
    [t, pImp] = leb(buf, pImp); /* typeidx */
    funcImports++;
  } else if (kind === 1) {
    pImp++; /* elemtype */
    let fl;
    [fl, pImp] = leb(buf, pImp);
    pImp++; /* limits flags */
    let v;
    [v, pImp] = leb(buf, pImp);
    if (fl & 1) [v, pImp] = leb(buf, pImp);
  } else if (kind === 2) {
    let fl;
    [fl, pImp] = leb(buf, pImp);
    pImp++;
    let v;
    [v, pImp] = leb(buf, pImp);
    if (fl & 1) [v, pImp] = leb(buf, pImp);
  } else if (kind === 3) {
    pImp += 2;
  }
}

/* CODE 段：每个函数体前面是 size，函数体起始 = 记录起始 + size 字段长度 */
let pCode = secs[10].start;
let nFuncs = 0;
[nFuncs, pCode] = leb(buf, pCode);
const bodies = [];
for (let i = 0; i < nFuncs; i++) {
  const recStart = pCode;
  let bsize;
  [bsize, pCode] = leb(buf, pCode);
  bodies.push({ idx: funcImports + i, hdr: recStart, body: pCode, size: bsize });
  pCode += bsize;
}

console.log('imports(func)=' + funcImports + ' code funcs=' + nFuncs);
console.log('CODE payload start=0x' + secs[10].start.toString(16) + ' size=0x' + secs[10].size.toString(16));

/* 找出包含该地址的函数（node 给的是指令 pc，落在 [body, body+size) 内） */
const want = process.argv.slice(3).map((s) => parseInt(s, 16));
for (const w of want) {
  let hit = null;
  for (const b of bodies) {
    if (w >= b.body && w < b.body + b.size) {
      hit = b;
      break;
    }
  }
  if (hit)
    console.log(
      `0x${w.toString(16)}  在函数 idx=${hit.idx} 内，body=0x${hit.body.toString(16)} size=0x${hit.size.toString(
        16
      )} 函数内偏移=0x${(w - hit.body).toString(16)}  codeRelBody=0x${(hit.body - secs[10].start).toString(16)}`
    );
  else console.log(`0x${w.toString(16)}  不在任何函数体内`);
}
