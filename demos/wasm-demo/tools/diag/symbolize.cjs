/* 把 node 报的 wasm 栈帧翻成符号（用 link map）。
 *
 * 标定：link map 的 CODE 段 Off 比 wasm 文件里函数体起始偏移小 5（map 的 CODE 项
 * 指向段头 size 前缀）。经验证 abort 的 map Off 0x4323f8 与首个函数 hdr-5 一致。
 * 输入：从 stdin 读 node 的栈文本（含 wasm-function[N]:0xOFF），输出符号化结果。 */
'use strict';
const fs = require('fs');
const path = require('path');

const DELTA = 5;
const mapPath = process.argv[2] || path.join(__dirname, '..', '..', 'out', 'soui-wasm-demo.map');

const syms = [];
for (const line of fs.readFileSync(mapPath, 'utf8').split('\n')) {
  const m = /^\s+\S\s+([0-9a-f]+)\s+([0-9a-f]+)\s+(\S+)\s*$/.exec(line);
  if (!m) continue;
  const size = parseInt(m[2], 16);
  if (!size) continue;
  const name = m[3];
  if (/^(TYPE|IMPORT|FUNCTION|TABLE|MEMORY|GLOBAL|EXPORT|ELEM|CODE|DATA|DATACOUNT)$/.test(name)) continue;
  syms.push({ off: parseInt(m[1], 16), size, name });
}
syms.sort((a, b) => a.off - b.off);

function resolve(pcFileOff) {
  const a = pcFileOff - DELTA;
  let lo = 0,
    hi = syms.length - 1,
    best = null;
  while (lo <= hi) {
    const mid = (lo + hi) >> 1;
    if (syms[mid].off <= a) {
      best = syms[mid];
      lo = mid + 1;
    } else hi = mid - 1;
  }
  if (!best) return '<below first symbol>';
  const rel = a - best.off;
  const inRange = rel < best.size;
  /* 名字去掉对象文件前缀，只留符号名 */
  let name = best.name.replace(/^.*?:(\((.*)\))$/, '$2').replace(/^\(|\)$/g, '');
  return `${name} +0x${rel.toString(16)}${inRange ? '' : '  (超出该符号 size，可能是相邻符号对齐误差)'}`;
}

let text = '';
process.stdin.setEncoding('utf8');
process.stdin.on('data', (d) => (text += d));
process.stdin.on('end', () => {
  const re = /wasm-function\[(\d+)\]:0x([0-9a-f]+)/;
  for (const line of text.split('\n')) {
    const m = re.exec(line);
    if (!m) continue;
    const pc = parseInt(m[2], 16);
    console.log(`idx ${m[1].padStart(6)}  0x${m[2].padStart(6)}  ${resolve(pc)}`);
  }
});
