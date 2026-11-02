/*
 * smoke.js -- soui-wasm-demo 的无头端到端烟测（node，不需要浏览器）。
 *
 * 它验证的是这次 wasm 移植里最容易坏、也最没法靠人工点几下确认的三件事：
 *
 *   1. 模块能加载、main() 能跑完、不 abort / 不抛致命错误
 *      —— 这等于整条启动链（SApplication → SAppCfg::DoConfig → 资源 →
 *         SHostWnd::CreateEx → 控件树 → 渲染后端初始化）都走通了。
 *   2. 渲染 + 上屏整链能跑：手动驱动 SwinxDemoPump 后，SwinxFbPresents
 *      计数必须增长（swinxFbPresent 在调宿主回调前先 ++，所以即便没装
 *      swinxPresent 钩子，计数也会涨）。这说明窗口、控件树、脏矩形、绘制、
 *      上屏回调都真的执行过。
 *   3. 输入注入路径不崩：鼠标按下/抬起、键盘、滚轮、IME 成品文本、剪贴板写入，
 *      经 SwinxInjectMouse/Key/Text 进到窗口的消息泵，事件映射与控制处理
 *      不抛、不 abort。
 *
 * 它不能证明的东西：画面是否好看、布局对不对、交互是否正确、字体字形对不对
 * —— 那些仍然要在 Chromium 里手动过一遍（见 readme.md 的验证清单）。
 *
 * 用法：
 *   source <emsdk>/emsdk_env.sh
 *   bash demos/wasm-demo/build-soui.sh
 *   node demos/wasm-demo/smoke.js
 *
 * 环境变量：
 *   DEMO_JS         wasm 模块的 .js 路径（缺省 ./out/soui-wasm-demo.js）
 *
 * 成功退出码 0，任何一条断言不成立退出码 1 并把失败项打到 stderr。
 *
 * 注意：本文件在 node 里跑，没有 DOM。canvas2d 后端在渲染时会调
 * document.createElement('canvas').getContext('2d')，这里用一个最小但够用的
 * 假 canvas / 2D context 顶上（所有绘制方法是 no-op，只有 measureText /
 * getImageData / createImageData 返回带正确形状的对象，否则布局与读像素会崩）。
 */

'use strict';

const path = require('path');
const fs = require('fs');

/* ------------------------------------------------------------------ 参数 */

const jsPath = path.resolve(
    process.argv[2] || process.env.DEMO_JS || path.join(__dirname, 'out', 'soui-wasm-demo.js'));
if (!fs.existsSync(jsPath)) {
    console.error('没有 ' + jsPath + ' —— 先运行 demos/wasm-demo/build-soui.sh');
    process.exit(1);
}
const outDir = path.dirname(jsPath);

/* ------------------------------------------------------------------ 假 DOM */

/* canvas2d 后端只借浏览器 2D 上下文做绘制，本烟测不需要真像素，所有方法 no-op。
 * 但有几处拿返回值做进一步计算，必须给真实形状：
 *   - measureText：布局按 fontBoundingBoxAscent/Descent 算行高，返回 0 会让整页
 *     内容逐行上移；这里给一个与字号成比例的近似值。
 *   - getImageData / createImageData：SwinxCanvasReadPixels / ReadPixels 会把它的
 *     .data（Uint8ClampedArray）回读到 wasm 堆，长度必须 = w*h*4，否则越界。
 *   - ctx.canvas：blit / present 时 drawImage(src.canvas, ...) 要用，必须回指。
 */
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
            // 字号从 this.font 里抠（如 "13px ..."），没有就用 13。
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
    // 兜底：未知属性读取返回 undefined，未知方法调用变成 no-op，避免漏一个就崩。
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
    // getContext 在第一次调用后才建 ctx，这里确保 .canvas 指向自身。
    Object.defineProperty(canvas, '_ctx', { value: makeCtx(canvas), writable: true, configurable: true });
    return canvas;
}

if (typeof ImageData === 'undefined') {
    global.ImageData = class { constructor(data, w, h) { this.data = data; this.width = w; this.height = h; } };
}
if (typeof requestAnimationFrame === 'undefined') {
    // emscripten_set_main_loop 在 node 里靠 rAF/setTimeout 起循环；这里给个 no-op
    // 防止它因 rAF 缺失而抛（循环本就由烟测手动驱动 SwinxDemoPump）。
    global.requestAnimationFrame = () => 0;
}
global.document = {
    createElement() { return makeCanvas(); },
    createElementNS() { return makeCanvas(); },
    getElementById() { return null; },
    querySelector() { return null; },
    addEventListener() {}, removeEventListener() {},
    body: makeCanvas(),
};

/* ------------------------------------------------------------------ 断言与工具 */

const failures = [];
function check(name, ok, detail) {
    console.log((ok ? 'PASS ' : 'FAIL ') + name + (detail ? '  [' + detail + ']' : ''));
    if (!ok) failures.push(name);
}
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

function writeStr(m, s) {
    const bytes = Buffer.from(s, 'utf8');
    const ptr = m._malloc(bytes.length + 1);
    if (!ptr) return 0;
    m.HEAPU8.set(bytes, ptr);
    m.HEAPU8[ptr + bytes.length] = 0;
    return ptr;
}

/* ------------------------------------------------------------------ 主流程 */

(async () => {
    let fatal = null;
    const createSouiDemo = require(jsPath);
    const m = await createSouiDemo({
        noExitRuntime: true,
        locateFile: (f) => path.join(outDir, f),
        print: () => {},
        printErr: (s) => { if (!/readlink|fopen|Could not connect|404/.test(s)) console.error('[wasm!] ' + s); },
        souiFatal: (w) => { fatal = w; console.error('[FATAL] ' + w); },
        // 音效钩子：node 没有 WebAudio，给个只记录的假实现，让 PlaySound 能返回 TRUE、
        // 走到"资源查名→拼前缀→swinx 读 wav→交页面"这条链（否则音频后端直接返回 FALSE）。
        swinxAudioPlay: () => true,
    });

    /* 1. 模块加载 + 导出符号齐全 */
    check('模块加载成功', !!m);
    const syms = [
        '_SwinxDemoPump', '_SwinxDemoMouse', '_SwinxDemoWheel', '_SwinxDemoKey',
        '_SwinxDemoText', '_SwinxDemoCaretPos', '_SwinxClipboardSetBrowserText',
        '_SwinxFbWidth', '_SwinxFbHeight', '_SwinxFbBits',
        '_SwinxFbPresents', '_SwinxFbRequests', '_malloc', '_free',
    ];
    const noExport = syms.filter((s) => typeof m[s] !== 'function');
    check('导出符号齐全', noExport.length === 0, noExport.join(','));

    /* 2. 帧缓冲几何有效（main() 跑过、SwinxSetCanvasScale 设置过） */
    const fbW = m._SwinxFbWidth();
    const fbH = m._SwinxFbHeight();
    const fbBits = m._SwinxFbBits();
    check('帧缓冲尺寸有效', fbW > 0 && fbH > 0, fbW + 'x' + fbH);
    check('帧缓冲指针非空', fbBits !== 0);

    /* 3. 手动驱动消息泵 → 渲染 + 上屏，fbo 计数器必须增长 */
    const presents0 = m._SwinxFbPresents();
    const requests0 = m._SwinxFbRequests();
    for (let i = 0; i < 8; ++i) {
        m._SwinxDemoPump();
        await sleep(0);
    }
    const presents1 = m._SwinxFbPresents();
    const requests1 = m._SwinxFbRequests();
    check('上屏计数随泵增长', presents1 > presents0, presents0 + '->' + presents1);
    check('帧请求计数随泵增长', requests1 > requests0, requests0 + '->' + requests1);

    /* 4. 输入注入路径不崩 */
    m._SwinxDemoMouse(0x0201 /* WM_LBUTTONDOWN */, 400, 300, 1 /* MK_LBUTTON */);
    m._SwinxDemoMouse(0x0202 /* WM_LBUTTONUP */, 400, 300, 0);
    m._SwinxDemoKey(0x0100 /* WM_KEYDOWN */, 0x41 /* 'A' */, 0);
    m._SwinxDemoKey(0x0101 /* WM_KEYUP */, 0x41, 0);
    m._SwinxDemoWheel(400, 300, 120 /* WHEEL_DELTA */, 0);
    const textPtr = writeStr(m, 'Hello 中文');
    if (textPtr) { m._SwinxDemoText(textPtr); m._free(textPtr); }
    const clipPtr = writeStr(m, 'clipboard-text');
    let clipOk = true;
    if (clipPtr) {
        clipOk = m._SwinxClipboardSetBrowserText(clipPtr) !== 0;
        m._free(clipPtr);
    }
    m._SwinxDemoPump();
    check('输入注入（鼠标/键盘/滚轮/IME/剪贴板）不崩', true);
    check('剪贴板写入返回成功', clipOk);

    /* 5. 插入符位置读取不崩（焦点不在编辑控件时返回 0 也合法） */
    const caretPtr = m._malloc(8);
    const caretRet = caretPtr ? m._SwinxDemoCaretPos(caretPtr) : -1;
    let caretOk = true;
    if (caretPtr) {
        if (caretRet === 1) {
            const heap = m.HEAP32;
            const cx = heap[caretPtr >> 2], cy = heap[(caretPtr >> 2) + 1];
            caretOk = cx >= 0 && cy >= 0 && cx <= fbW && cy <= fbH;
        }
        m._free(caretPtr);
    }
    check('插入符位置读取合法', caretOk, 'ret=' + caretRet);

    /* 6. 全程无致命错误 */
    check('无致命错误', fatal === null);

    await sleep(0);
    if (failures.length) {
        console.error('\nFAILED: ' + failures.length + ' 条断言未通过 —— ' + failures.join('; '));
        process.exit(1);
    }
    console.log('\nOK: 全部断言通过');
    process.exit(0);
})().catch((e) => {
    console.error('[smoke 异常] ' + (e && e.stack ? e.stack : e));
    process.exit(1);
});
