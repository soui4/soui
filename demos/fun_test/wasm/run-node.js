/* ---------------------------------------------------------------------------
 * wasm fun_test 的 node 运行器
 *
 * 干什么：在 node 里加载 out.<backend>/fun_test-wasm.js 并跑完整个 gtest 可执行文件。
 * gtest 的 stdout 原样透传，退出码就是 gtest 的退出码（0 = 全部通过）。
 *
 * 为什么需要假 DOM：canvas2d 后端（swinx/src/gdi/canvas2d/）用 emscripten::val 持有
 * 离屏 canvas 与 2D context，node 里没有 document。这里的桩件与
 * demos/wasm-demo/smoke.js 同一套口径（那份是演示烟测的内联版本，为了不动既有烟测
 * 这里单独落一份）。
 *
 * 桩件的边界 —— 这一点决定了本腿能证明什么：
 *   - 绘制调用（fillRect/drawImage/putImageData/fillText…）是 no-op，不产生像素；
 *   - getImageData / createImageData 返回长度 = w*h*4 的真实形状 Uint8ClampedArray，
 *     但内容全 0 —— 所以"写进去能读回来"这类像素往返断言在 node 腿必然失败，
 *     那是桩件不给像素，不是实现的错。像素语义的判定权在浏览器腿
 *     （直接打开产物目录里的 fun_test-wasm.html）。
 *   - measureText 给按字号的近似值，够文本度量类断言（正数、单调）用。
 * 因此本腿的证明目标是：句柄/对象生命周期、参数编组、区域布尔运算、返回值与
 * GetLastError 口径、wasm↔JS 边界不崩不泄。
 *
 * 用法：
 *   node demos/fun_test/wasm/run-node.js [产物 js 路径] [--gtest_filter=...] [--gtest_list_tests]
 *   默认产物路径 out.canvas2d/fun_test-wasm.js（相对 demos/fun_test）。
 * ------------------------------------------------------------------------- */

const fs = require('fs');
const path = require('path');

const FUN_TEST_DIR = path.resolve(__dirname, '..');

/* ------------------------------------------------------------------ 参数 */

const argv = process.argv.slice(2);
let jsPath = null;
const gtestArgs = [];
for (const a of argv) {
    if (a.endsWith('.js')) jsPath = path.resolve(a);
    else gtestArgs.push(a);
}
if (!jsPath) jsPath = path.join(FUN_TEST_DIR, 'out.canvas2d', 'fun_test-wasm.js');
if (!fs.existsSync(jsPath)) {
    console.error('没有 ' + jsPath + '\n先构建 wasm 腿（独立顶层工程）：\n' +
        '  emcmake cmake -S <soui4>/demos/fun_test/wasm -B <build> -G Ninja ' +
        '-DSWINX_WASM_GDI_BACKEND=canvas2d\n  cmake --build <build> --target fun_test');
    process.exit(2);
}

/* ------------------------------------------------------------------ 假 DOM */

/* 每个方法都要真实形状的返回值的位置，见文件头"桩件的边界"。 */
function makeCtx(canvas) {
    const real = {
        canvas,
        font: '',
        fillStyle: '',
        strokeStyle: '',
        textAlign: 'left',
        textBaseline: 'alphabetic',
        globalAlpha: 1,
        imageSmoothingEnabled: true,
        globalCompositeOperation: 'source-over',
        lineWidth: 1,
        save() {}, restore() {},
        beginPath() {}, closePath() {},
        moveTo() {}, lineTo() {}, bezierCurveTo() {},
        rect() {}, clip() {}, setTransform() {}, setLineDash() {}, roundRect() {},
        fill() {}, stroke() {},
        fillRect() {}, clearRect() {}, strokeRect() {},
        fillText() {}, strokeText() {}, drawImage() {},
        putImageData() {},
        getImageData(x, y, w, h) {
            const ww = Math.max(0, w | 0), hh = Math.max(0, h | 0);
            return { data: new Uint8ClampedArray(ww * hh * 4), width: ww, height: hh };
        },
        createImageData(w, h) {
            const ww = Math.max(0, w | 0), hh = Math.max(0, h | 0);
            return { data: new Uint8ClampedArray(ww * hh * 4), width: ww, height: hh };
        },
        measureText(s) {
            const m = /(\d+(?:\.\d+)?)px/.exec(this.font || '');
            const px = m ? parseFloat(m[1]) : 13;
            return {
                width: (s ? s.length : 0) * px * 0.6,
                fontBoundingBoxAscent: px * 0.8,
                fontBoundingBoxDescent: px * 0.2,
                actualBoundingBoxAscent: px * 0.8,
                actualBoundingBoxDescent: px * 0.2,
            };
        },
    };
    /* 兜底：未知方法调用变 no-op，漏一个桩不至于整跑挂。 */
    return new Proxy(real, {
        get(t, k) {
            if (k in t) return t[k];
            if (typeof k === 'string') return () => undefined;
            return undefined;
        },
        set(t, k, v) { t[k] = v; return true; },
    });
}

function makeCanvas() {
    const canvas = {
        width: 1, height: 1, style: {},
        classList: { add() {}, remove() {}, contains() { return false; } },
        addEventListener() {}, removeEventListener() {}, appendChild() {}, removeChild() {},
        getBoundingClientRect() { return { left: 0, top: 0, width: this.width, height: this.height }; },
        getContext(_type, _opts) { if (!this._ctx) this._ctx = makeCtx(this); return this._ctx; },
        parentNode: null,
    };
    Object.defineProperty(canvas, '_ctx', { value: makeCtx(canvas), writable: true, configurable: true });
    return canvas;
}

if (typeof ImageData === 'undefined') {
    global.ImageData = class { constructor(data, w, h) { this.data = data; this.width = w; this.height = h; } };
}
if (typeof requestAnimationFrame === 'undefined') global.requestAnimationFrame = () => 0;
global.document = {
    createElement() { return makeCanvas(); },
    createElementNS() { return makeCanvas(); },
    getElementById() { return null; },
    querySelector() { return null; },
    addEventListener() {}, removeEventListener() {},
    body: makeCanvas(),
};

/* ------------------------------------------------------------------ 跑 */

/* 参数要在 emscripten 解析 argv 之前放进 process.argv —— 非 MODULARIZE 的产物
 * 在 require 的瞬间就执行 main()。 */
process.argv = [process.argv[0], jsPath].concat(gtestArgs);

const t0 = Date.now();
require(jsPath);
if (gtestArgs.indexOf('--gtest_list_tests') >= 0) console.error('（列出用例耗时 ' + (Date.now() - t0) + ' ms）');
