/*
 * node_shim.cjs -- 在 Node 里跑 soui-wasm-demo.js，用 canvas 桩件定位挂死点。
 *
 * 目的：浏览器里主线程被独占，CDP 的 Runtime.evaluate / Page.captureScreenshot 全部超时，
 * 拿不到任何线索。Node 里同一份 .js 走 ENVIRONMENT_IS_NODE 分支（.data 用 fs 直读），
 * 所有 GDI → canvas 调用都落在桩件上，可以统计"哪些 2D API 被调了、谁一次都没被调"，
 * 并用 fs.writeSync(1,...) 同步打点：挂死前最后一行就是现场。
 *
 * 用法：node build/node_shim.cjs [泵帧数]
 */
'use strict';
const fs = require('fs');
const path = require('path');

const OUT = path.join(__dirname, '..', '..', 'out');
const WASM = path.join(OUT, 'soui-wasm-demo.wasm');
const DATA = path.join(OUT, 'soui-wasm-demo.data');
const JS = path.join(OUT, 'soui-wasm-demo.js');

/* 同步打点：管道/重定向下 console.log 是异步的，挂死会丢尾巴。 */
function mark(s) {
  fs.writeSync(1, '[mark] ' + s + '\n');
}

const t0 = process.hrtime.bigint();
function ms() {
  return Number((process.hrtime.bigint() - t0) / 1000000n) / 1000;
}
function tick(s) {
  mark(ms().toFixed(3) + 's  ' + s);
}

tick('start  node=' + process.version);

/* ── canvas 2D 统计 ─────────────────────────────────────────────── */
const calls = Object.create(null);
const TRACE = process.env.SHIM_TRACE === '1';
let traceN = 0;
const TRACE_MAX = parseInt(process.env.SHIM_TRACE_MAX || '4000', 10);
function note(k) {
  calls[k] = (calls[k] || 0) + 1;
  if (TRACE) {
    if (traceN < TRACE_MAX) fs.writeSync(1, '[trace] ' + ++traceN + ' ' + k + '\n');
    else if (traceN === TRACE_MAX) {
      ++traceN;
      fs.writeSync(1, '[trace] ... truncated at ' + TRACE_MAX + '\n');
    }
  }
}
const texts = [];
let nonAscii = 0;
let bytesRead = 0;
let bytesPut = 0;

function makeGradient() {
  return {
    addColorStop(o, c) {
      note('gradient.addColorStop');
    },
  };
}

/* ctx 用 Proxy：gdi_canvas2d.cpp 用到的 2D API 有几十个，逐个实现既啰嗦又易漏，
 * 统一兜成"记一次调用 + 返回合理默认值"；已被赋值的属性（fillStyle 等）存在 target 上，
 * 读回时 from `k in target` 短路，保证样式往返正确。 */
function makeCtx(canvas) {
  const target = { canvas };
  return new Proxy(target, {
    get(t, k) {
      if (typeof k === 'symbol') return t[k];
      if (k in t) return t[k];
      return function (...args) {
        note(k);
        /* 记录前若干条文本，用来验证「C++ → JS 字符串」这一层的解码是否正确：
         * 正常应是中文，乱码则会看到 å_å°±ä¸_ 这类拉丁扩展字符。 */
        if (k === 'fillText' || k === 'measureText') {
          const s = String(args[0] == null ? '' : args[0]);
          /* 只收非 ASCII 的样本，并统计总共有多少条带非 ASCII 的文本 */
          if (s && /[^\x00-\x7f]/.test(s)) {
            nonAscii++;
            if (texts.length < 10) texts.push(k + ':' + JSON.stringify(s));
          }
        }
        switch (k) {
          case 'measureText': {
            /* 粗略度量：中文字宽≈字号，ASCII≈字号*0.55。测试只用来看布局是否收敛。 */
            const s = String(args[0] == null ? '' : args[0]);
            const st = t.fontSize || 14;
            let w = 0;
            for (const ch of s) w += ch.charCodeAt(0) > 0x2e80 ? st : st * 0.55;
            return {
              width: w,
              actualBoundingBoxAscent: st * 0.8,
              actualBoundingBoxDescent: st * 0.2,
              actualBoundingBoxLeft: 0,
              actualBoundingBoxRight: w,
            };
          }
          case 'getImageData': {
            const sw = Math.max(1, args[2] | 0);
            const sh = Math.max(1, args[3] | 0);
            bytesRead += sw * sh * 4;
            /* 填成不透明蓝，便于肉眼判断"读回的像素是否真的被用上了" */
            const d = new Uint8ClampedArray(sw * sh * 4);
            for (let i = 0; i < d.length; i += 4) {
              d[i] = 0x20;
              d[i + 1] = 0x60;
              d[i + 2] = 0xc0;
              d[i + 3] = 0xff;
            }
            return { data: d, width: sw, height: sh };
          }
          case 'createImageData': {
            const sw = Math.max(1, typeof args[0] === 'number' ? args[0] : 1);
            const sh = Math.max(1, typeof args[1] === 'number' ? args[1] : 1);
            return { data: new Uint8ClampedArray(sw * sh * 4), width: sw, height: sh };
          }
          case 'createLinearGradient':
          case 'createRadialGradient':
          case 'createConicGradient':
            return makeGradient();
          case 'createPattern':
            return { setTransform() {} };
          case 'isPointInPath':
          case 'isPointInStroke':
            return false;
          default:
            if (k.slice(0, 6) === 'create') return {};
            return undefined;
        }
      };
    },
    set(t, k, v) {
      note('set:' + String(k));
      t[k] = v;
      /* 记下最近一次 font 里的字号，供 measureText 估算 */
      if (k === 'font' && typeof v === 'string') {
        const m = /(\d+(?:\.\d+)?)px/.exec(v);
        if (m) t.fontSize = parseFloat(m[1]);
      }
      return true;
    },
    has() {
      return true;
    },
  });
}

function makeCanvas(w, h) {
  const canvas = {
    tagName: 'CANVAS',
    width: w | 0 || 1,
    height: h | 0 || 1,
    style: {},
    classList: { add() {}, remove() {}, contains: () => false },
    addEventListener() {},
    removeEventListener() {},
    getBoundingClientRect: () => ({ left: 0, top: 0, width: canvas.width, height: canvas.height }),
    focus() {},
    toDataURL: () => 'data:,',
  };
  let ctx = null;
  canvas.getContext = function (type) {
    note('getContext:' + type);
    if (type !== '2d') return null;
    if (!ctx) ctx = makeCtx(canvas);
    return ctx;
  };
  return canvas;
}

function makeElement(tag) {
  if (String(tag).toLowerCase() === 'canvas') return makeCanvas(1, 1);
  return {
    tagName: String(tag).toUpperCase(),
    style: {},
    value: '',
    addEventListener() {},
    appendChild() {},
    removeChild() {},
    setAttribute() {},
    focus() {},
  };
}

globalThis.document = {
  createElement: (tag) => {
    note('document.createElement:' + tag);
    return makeElement(tag);
  },
  getElementById: () => makeCanvas(800, 600),
  body: { appendChild() {}, style: {} },
  documentElement: { style: {} },
  addEventListener() {},
  activeElement: null,
};

/* PutPixels 用 new ImageData(Uint8ClampedArray, w, h)，Node 没有这个全局类型。 */
globalThis.ImageData = class ImageData {
  constructor(a, b, c) {
    if (a && a.data) {
      this.data = a.data;
      this.width = a.width;
      this.height = a.height;
      return;
    }
    this.data = a;
    this.width = b | 0;
    this.height = c | 0;
    bytesPut += this.width * this.height * 4;
    note('new ImageData');
  }
};

tick('canvas shim ready');

/* ── 加载模块 ───────────────────────────────────────────────────── */
const wasmBytes = fs.readFileSync(WASM);
const dataBytes = fs.readFileSync(DATA);
tick('wasm ' + wasmBytes.length + ' B, data ' + dataBytes.length + ' B');

const createSouiDemo = require(JS);

const Module = {
  canvas: makeCanvas(800, 600),
  print: (...a) => {
    fs.writeSync(1, '[stdout] ' + a.join(' ') + '\n');
  },
  printErr: (...a) => {
    fs.writeSync(1, '[stderr] ' + a.join(' ') + '\n');
  },
  souiFatal: (m) => tick('souiFatal: ' + m),
  souixClipboardWrite: () => {},
  swinxAudioPlay: () => false,
  swinxPresent: (l, t, r, b) => {
    note('swinxPresent');
    presents++;
    lastDirty = [l, t, r, b];
  },
  getPreloadedPackage: () => {
    /* Node 分支自己会 fs.readFileSync，这里抢在前面省一次同步读；返回 ArrayBuffer */
    return dataBytes.buffer.slice(dataBytes.byteOffset, dataBytes.byteOffset + dataBytes.byteLength);
  },
  instantiateWasm: (imports, cb) => {
    tick('instantiateWasm: ' + Object.keys(imports).join(','));
    WebAssembly.instantiate(wasmBytes, imports).then(
      (r) => {
        tick('instantiated');
        cb(r.instance, r.module);
      },
      (e) => {
        tick('instantiate FAILED: ' + e);
        throw e;
      }
    );
    return {};
  },
};
let presents = 0;
let lastDirty = null;

(async () => {
  try {
    await createSouiDemo(Module);
    tick('createSouiDemo resolved');
  } catch (e) {
    tick('createSouiDemo threw: ' + e);
    if (e && e.stack) {
      for (const line of String(e.stack).split('\n').slice(0, 40))
        fs.writeSync(1, '    | ' + line + '\n');
    }
  }

  const exp = Module.asm || {};
  const api = (n) => (typeof Module['_' + n] === 'function' ? Module['_' + n] : null);
  tick(
    'fb: ' +
      (api('SwinxFbWidth') ? api('SwinxFbWidth')() : '?') +
      'x' +
      (api('SwinxFbHeight') ? api('SwinxFbHeight')() : '?') +
      ' stride ' +
      (api('SwinxFbStride') ? api('SwinxFbStride')() : '?') +
      ' bits@' +
      (api('SwinxFbBits') ? api('SwinxFbBits')() : '?') +
      ' requests=' +
      (api('SwinxFbRequests') ? api('SwinxFbRequests')() : '?') +
      ' presents=' +
      (api('SwinxFbPresents') ? api('SwinxFbPresents')() : '?')
  );

  const pump = api('SwinxDemoPump');
  if (!pump) {
    tick('NO SwinxDemoPump export');
  } else {
    const frames = parseInt(process.argv[2] || '5', 10);
    for (let i = 0; i < frames; i++) {
      const a = ms();
      pump();
      tick(
        'pump#' +
          i +
          ' ' +
          (ms() - a).toFixed(3) +
          's  presents=' +
          presents +
          ' dirty=' +
          JSON.stringify(lastDirty)
      );
    }
  }

  /* 检查帧缓冲是否真有像素（不是全 0） */
  const getBits = api('SwinxFbBits');
  const getW = api('SwinxFbWidth');
  if (getBits && getW) {
    const ptr = getBits() >>> 2;
    const w = getW();
    const stride = api('SwinxFbStride')();
    const h = api('SwinxFbHeight')();
    const u32 = Module.HEAPU32;
    let nz = 0;
    const hist = new Map();
    const total = Math.min(w * h, 800 * 600);
    for (let y = 0; y < h; y += 3) {
      for (let x = 0; x < w; x += 3) {
        const v = u32[ptr + y * stride + x];
        if (v !== 0) nz++;
        hist.set(v, (hist.get(v) || 0) + 1);
      }
    }
    const top = [...hist.entries()].sort((a, b) => b[1] - a[1]).slice(0, 8);
    tick('fb non-zero sampled pixels: ' + nz + '/' + Math.ceil(h / 3) * Math.ceil(w / 3));
    tick(
      'fb top colors: ' +
      top.map(([v, c]) => '0x' + (v >>> 0).toString(16).padStart(8, '0') + 'x' + c).join(' ')
    );
  }
  tick('presents(js)=' + presents + ' imageDataBytesRead=' + bytesRead + ' put=' + bytesPut);

  /* 打印 canvas API 调用统计 */
  const keys = Object.keys(calls).sort((a, b) => calls[b] - calls[a]);
  tick('canvas calls distinct=' + keys.length + ' total=' + keys.reduce((s, k) => s + calls[k], 0));
  for (const k of keys) fs.writeSync(1, '    ' + String(calls[k]).padStart(7) + '  ' + k + '\n');

  tick('带非 ASCII 的文本条数=' + nonAscii + '，前几条：');
  for (const t of texts) fs.writeSync(1, '    ' + t + '\n');

  tick('done');
})();
