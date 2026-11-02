// 比对 .wasm 声明的 import 与 .js 胶水是否真的提供了同名函数
// 用法: node check_imports.cjs <目录>   (默认 demos/wasm-demo/out)
const fs = require('fs');
const path = require('path');

const dir = process.argv[2] || path.resolve(__dirname, '../../../../demos/wasm/out');
const wasmPath = path.join(dir, 'soui-wasm-demo.wasm');
const jsPath = path.join(dir, 'soui-wasm-demo.js');

const mod = new WebAssembly.Module(fs.readFileSync(wasmPath));
const imports = WebAssembly.Module.imports(mod);
const js = fs.readFileSync(jsPath, 'utf8');

console.log('wasm:', wasmPath);
console.log('js  :', jsPath);
console.log('stdout:', fs.statSync(jsPath).size, 'bytes | 导入了', imports.length, '项');

const missing = [];
const fnImports = imports.filter((i) => i.kind === 'function');
for (const it of fnImports) {
  // 胶水里的三种形态：
  //   "名字": 值的显式键 / 名字:值 / ES6 简写 {..., 名字, ...}
  const n = it.name.replace(/[$]/g, '\\$');
  const quoted = new RegExp('["\']' + n + '["\']\\s*:');
  const direct = new RegExp('\\b' + n + '\\s*[=:]');
  const shorthand = new RegExp('[{,]\\s*' + n + '\\s*[,}]');
  if (!quoted.test(js) && !direct.test(js) && !shorthand.test(js)) {
    missing.push(it);
  }
}

console.log('函数型 import:', fnImports.length, '| JS 中找不到定义的:', missing.length);
for (const m of missing.slice(0, 30)) console.log('  MISSING', m.module + '.' + m.name);
