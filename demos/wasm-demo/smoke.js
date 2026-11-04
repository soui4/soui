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
/* node 也没有 location，而 main() 开头就用 location.search 读 ?dpr= 参数
 * （emscripten_run_script_string 里的 eval 找不到 location 会直接 ReferenceError，
 * 整个模块加载失败）。给一个空查询串的对象，等价于浏览器地址栏没带参数。 */
if (typeof location === 'undefined') {
    global.location = { search: '', pathname: '/soui.html', href: 'file:///soui.html' };
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

/* Win32 的 lpstrFilter 是"描述\0模式\0…\0\0"的双空结尾串，写进线性内存要逐段带上
 * 自己的终止符（writeStr 只会补一个 0）。 */
function writeFilter(m, parts) {
    const bufs = parts.map((s) => Buffer.from(s, 'utf8'));
    let total = 1; // 整串结尾的那个 0：最后一段的 \0 就是筛选表的空行
    for (const b of bufs) total += b.length + 1;
    const ptr = m._malloc(total);
    if (!ptr) return 0;
    let off = 0;
    for (const b of bufs) {
        m.HEAPU8.set(b, ptr + off);
        off += b.length;
        m.HEAPU8[ptr + off] = 0;
        off += 1;
    }
    m.HEAPU8[ptr + off] = 0;
    return ptr;
}

function readStr(m, ptr) {
    if (!ptr) return '';
    const h = m.HEAPU8; // 线性内存可能已扩容，视图每次重取
    let s = '';
    for (let i = ptr >>> 0; h[i] !== 0; ++i) s += String.fromCharCode(h[i]);
    return Buffer.from(Array.from(s, (c) => c.charCodeAt(0))).toString('utf8');
}

function readInts(m, buf, n) {
    const h = m.HEAP32;
    const out = [];
    for (let i = 0; i < n; ++i) out.push(h[(buf >> 2) + i]);
    return out;
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
        '_SwinxDemoText', '_SwinxDemoCaretPos', '_SwinxDemoTaskLoopSelfTest',
        '_SwinxDemoFilePickSelfTest', '_SwinxDemoFilePickPath', '_SwinxFilePickDone',
        '_SwinxFilterToAcceptA',
        '_SwinxClipboardSetBrowserText',
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

    /* 6. ITaskLoop 的 wasm 后端：一条队列走完 start → 投递 → 排空 → stop → Release。
     * pOut 的 12 个字段含义见 soui_main.cpp 里 SwinxDemoTaskLoopSelfTest 的注释。
     * 两条后端（泵 / worker 线程）都要成立的是"任务一条不多一条不少、结果都回到 UI
     * 线程"；只有 -pthread 构建额外要求"任务跑在非 UI 线程上"—— 这正是这个模块存在
     * 的理由，也是泵后端给不了的东西。 */
    const stBuf = m._malloc(48);
    if (!stBuf) {
        check('自测：分配结果缓冲', false);
    } else {
        const stView = () => new Int32Array(m.HEAPU8.buffer, stBuf, 12);
        stView().fill(0);
        m._SwinxDemoTaskLoopSelfTest(stBuf);
        // 自测期间线性内存可能增长，视图必须重新取
        const st = Array.from(stView());
        const [hasPt, workers, posted, execTotal, workerRuns, uiRuns, inlineRuns,
            beatRuns, pendingAfter, runningAfterStop, liveDelta, runningLive] = st;
        console.log('  [taskloop] pt=' + hasPt + ' workers=' + workers + ' posted=' + posted +
            ' exec=' + execTotal + ' offUi=' + workerRuns + ' ui=' + uiRuns + ' inline=' + inlineRuns +
            ' beat=' + beatRuns + ' pending=' + pendingAfter + ' afterStop=' + runningAfterStop +
            ' liveDelta=' + liveDelta);
        check('自测：队列在运行中', runningLive === 1, 'isRunning=' + runningLive);
        check('自测：后台任务一条不多一条不少', posted === 8 && execTotal === 8,
            'posted=' + posted + ' exec=' + execTotal);
        check('自测：结果全部回到 UI 线程执行', uiRuns === posted, 'ui=' + uiRuns + '/' + posted);
        check('自测：send 语义（waitUntilDone）内联执行', inlineRuns === 1, 'inline=' + inlineRuns);
        check('自测：心跳任务执行过', beatRuns >= 1, 'beat=' + beatRuns);
        check('自测：排空后没有残留任务', pendingAfter === 0, 'pending=' + pendingAfter);
        check('自测：stop 之后队列不再运行', runningAfterStop === 0, 'isRunning=' + runningAfterStop);
        check('自测：Release 之后实例从注册表摘掉', liveDelta === 0, 'liveDelta=' + liveDelta);
        if (hasPt === 1) {
            check('自测：-pthread 构建真的起了 worker 线程', workers === 1, 'workers=' + workers);
            check('自测：后台任务跑在非 UI 线程上', workerRuns === posted,
                'offUi=' + workerRuns + '/' + posted);
        } else {
            check('自测：没带 -pthread 时退回泵后端', workers === 0 && workerRuns === 0,
                'workers=' + workers + ' offUi=' + workerRuns);
        }
        m._free(stBuf);
    }

    /* 7. 异步文件选择（SwinxPickFileAsyncA）。node 里没有浏览器，这里装一个假的宿主
     * 钩子，形状和 soui-host.js 里那份真钩子一致：记下 (nId, title, accept)，再用
     * setTimeout 走同一条回投路 —— _malloc 名字与字节、调 _SwinxFilePickDone、当场
     * free（所有权约定：C++ 只读，JS 负责释放）。
     * pOut 的阶段含义见 soui_main.cpp 里 SwinxDemoFilePickSelfTest 的注释。 */
    const pickSeen = [];
    const pickPayloads = [
        { name: 'avatar.txt', bytes: Buffer.from('SOUI-WASM-PICK-OK', 'utf8') },
        /* 名字里带 "../" 和空格：浏览器给的名字会被拼成 wasm 文件系统里的一个真实路径，
         * 消毒规则（去分隔符、压掉 ".."、空格转下划线）由这条钉住。 */
        { name: '../../etc/pass wd.txt', bytes: Buffer.from('X', 'utf8') },
    ];
    m.swinxPickFile = function (nId, title, accept) {
        const p = pickPayloads[pickSeen.length];
        pickSeen.push({ nId, title, accept });
        setTimeout(() => {
            if (!p) { m._SwinxFilePickDone(nId, 0, 0, 0, 0); return; }
            const nb = Buffer.from(p.name, 'utf8');
            const pName = m._malloc(nb.length + 1);
            m.HEAPU8.set(nb, pName);
            m.HEAPU8[pName + nb.length] = 0;
            const pData = m._malloc(p.bytes.length);
            m.HEAPU8.set(p.bytes, pData);
            m._SwinxFilePickDone(nId, 1, pName, pData, p.bytes.length);
            m._free(pName);
            m._free(pData);
        }, 0);
        return true;
    };

    /* 7a. SwinxFilterToAcceptA 是纯函数，逐字符钉住映射规则（描述段丢弃、".png" 去星号、
     * "*.*" 与空模式段按接受全部处理、重复模式去掉、空格清掉）。 */
    const acceptCases = [
        { parts: ['文本与图片', '*.txt;*.png'], want: '.txt,.png' },
        { parts: ['所有文件', '*.*'], want: '' },
        { parts: ['A', '*.png; *.PNG', 'B', '*.png'], want: '.png,.PNG' },
        { parts: ['图片', 'image/*'], want: 'image/*' },
        { parts: [], want: '' },
    ];
    let acceptBad = [];
    let probeBad = [];
    for (const c of acceptCases) {
        const f = writeFilter(m, c.parts);
        if (!f) { acceptBad.push('malloc 失败'); break; }
        const obuf = m._malloc(256);
        const n = m._SwinxFilterToAcceptA(f, obuf, 256);
        const got = n < 0 ? '<-1>' : readStr(m, obuf);
        if (n !== got.length || got !== c.want)
            acceptBad.push(JSON.stringify(c.parts.join('\\0')) + ' -> ' + JSON.stringify(got) +
                '（期望 ' + JSON.stringify(c.want) + '，返回 ' + n + '）');
        /* 探长度（cchOut=0 不写字节）与"缓冲不够必须显式失败"：静默截断会把一个不存在的
         * 路径或半个 accept 交出去，比报错更糟。 */
        const need = m._SwinxFilterToAcceptA(f, 0, 0);
        if (need !== c.want.length + 1) probeBad.push('need=' + need + ' 期望 ' + (c.want.length + 1));
        /* 短缓冲只在"要写的内容非空"时才可测：cchOut=0 本身就是探长度形态（契约里
         * 就是这么定的），拿它当"缓冲不够"用例会得到正数而不是 -1。 */
        if (c.want.length > 0) {
            const short = m._SwinxFilterToAcceptA(f, obuf, c.want.length);
            if (short !== -1) probeBad.push('短缓冲返回 ' + short + ' 期望 -1');
        }
        m._free(obuf);
        m._free(f);
    }
    check('筛选串换算成 accept 正确', acceptBad.length === 0, acceptBad.join('; '));
    check('accept 探长度正确、短缓冲显式拒绝', probeBad.length === 0, probeBad.join('; '));

    /* 7b. 第一轮：发起必须立刻返回，回调只能从 JS 那一侧回来。 */
    const pickBuf = m._malloc(32);
    if (!pickBuf) {
        check('自测：分配探针缓冲', false);
    } else {
        m._SwinxDemoFilePickSelfTest(0, pickBuf);
        let r = readInts(m, pickBuf, 8);
        check('文件选择：请求已受理', r[0] === 1, 'ret=' + r[0] + ' lastError=' + r[2]);
        check('文件选择：发起时没有内联回调（没把主线程挂在对话框上）', r[1] === 0,
            '发起后回调数=' + r[1]);
        check('文件选择：宿主钩子收到标题与 accept',
            pickSeen.length === 1 && pickSeen[0].title === '选择文件（swinx 异步文件选择）' &&
            pickSeen[0].accept === '.txt,.png',
            pickSeen.length ? 'title=' + pickSeen[0].title + ' accept=' + JSON.stringify(pickSeen[0].accept) : '没调用');

        await sleep(10);
        m._SwinxDemoFilePickSelfTest(1, pickBuf);
        r = readInts(m, pickBuf, 8);
        const path0 = readStr(m, m._SwinxDemoFilePickPath());
        check('文件选择：回调回到发起线程且只回一次', r[0] === 1 && r[4] === 1,
            'cb=' + r[0] + ' onUi=' + r[4]);
        check('文件选择：结果、字节数与首字节都对',
            r[1] === 1 && r[2] === 17 && r[3] === 83,
            'nResult=' + r[1] + ' bytes=' + r[2] + ' head=' + r[3]);
        const want0 = '/tmp/swinx-pick/' + pickSeen[0].nId + '-avatar.txt';
        check('文件选择：回投的字节落到可直接 fopen 的路径', path0 === want0 && r[5] === want0.length,
            'path=' + path0);

        /* 第二轮：名字里带 "../" 与空格，消毒后的路径必须还在选择目录里。 */
        m._SwinxDemoFilePickSelfTest(0, pickBuf);
        r = readInts(m, pickBuf, 8);
        await sleep(10);
        m._SwinxDemoFilePickSelfTest(1, pickBuf);
        r = readInts(m, pickBuf, 8);
        const path1 = readStr(m, m._SwinxDemoFilePickPath());
        const want1 = '/tmp/swinx-pick/' + pickSeen[1].nId + '-etcpass_wd.txt';
        check('文件选择：浏览器给的文件名被消毒，路径没跑出选择目录',
            r[0] === 2 && path1 === want1 && r[2] === 1 && r[3] === 88,
            'cb=' + r[0] + ' path=' + path1 + ' bytes=' + r[2]);

        /* 7c. 负路径：参数不合法与"wasm 没有 SAVE"都必须在发起前显式拒绝，且不产生回调。 */
        m._SwinxDemoFilePickSelfTest(2, pickBuf);
        r = readInts(m, pickBuf, 8);
        check('文件选择：非法参数与 SAVE 显式拒绝',
            r[0] === 0 && r[1] === 0 && r[2] === 0 && r[3] === 1 && r[4] === 0 && r[5] === 1,
            'null=' + r[0] + ' 无缓冲=' + r[1] + ' 多选=' + r[2] + '/' + r[3] +
            ' SAVE=' + r[4] + '/' + r[5]);
        check('文件选择：过期 id 的回投被丢掉，不会回调第二次', r[6] === 2, 'cb=' + r[6]);

        m._free(pickBuf);
    }

    /* 8. 全程无致命错误 */
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
