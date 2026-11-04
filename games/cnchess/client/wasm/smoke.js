/*
 * smoke.js -- 中国象棋 wasm 客户端的无头端到端烟测（node，不需要浏览器）
 *
 * 它验证的是这次移植里最容易坏、也最没法靠人工点几下确认的三件事：
 *
 *   1. 网络真的走浏览器原生 WebSocket
 *      脚本在加载模块前把 global.WebSocket 换成带探针的实现（node-websocket.js），
 *      然后断言构造函数收到的 URL 是 ws://127.0.0.1:<port>/cnchess/?、只开了一条连接、
 *      收到的帧全是二进制、字节数和服务器发出去的主题包对得上。桌面版那条 lws
 *      路径在 wasm 构建里根本没被链接进来（-lwebsocket.js 提供的是 JS 实现）。
 *   2. 主题包的下载 + 落盘 + 解压 + 加载全部完成
 *      服务器在 pc_theme.zip 缺失/不一致时会整包下发（4MB 左右），客户端收齐后
 *      校验 MD5、写进 MEMFS 缓存目录，再交给 ITaskLoop 解压。断言 stat 4（主题已加载）
 *      和 stat 5（chessboard 被 InsertGame 填满）都成立，等于整条链都跑过一遍。
 *   3. wasm 版 TaskLoop 按构建能力跑在正确的后端上
 *      不带 -pthread 的产物：解压只可能由 STaskLoopPump 从主循环里跑掉；带 -pthread 的
 *      产物（build-cnchess.sh 的默认）：start() 建一条 worker 线程，队列归它消费。
 *      断言分三条：累计执行数 >0、结束时排队数为 0、临时队列（解压那条）已释放，
 *      再加一条按后端分流的 worker 计数 —— stat 19（在 worker 上执行掉的任务数）等于
 *      stat 6（总执行数）才是"整条解压离开了 UI 线程"，泵后端下它必须恒为 0。这条差值
 *      只有真实线程给得出，是这次改动唯一的直接证据。
      结束时 stat 8 不再是 0：收包链自己长驻一条投递队列（stat 21/22，见下面第 7 条），
      所以那里断言的是"只剩那一条"，解压队列没漏。
 *      整段解压发生在一次 CnchessPump 里时，JS 侧采不到"运行中"的中间态；worker 后端
 *      下它跨帧了，所以 live/pending/workers 的峰值两边都会打一行观测，泵后端那条
 *      峰值为 0 正是"全都堵在主线程"的表现。
 *
 * 外加三条"只有页面能提供的能力"在 wasm 侧的落点（页面那半段仍要人工过）：
 *
 *   4. 音效：主题包里解出来的 wav 能被 swinx 读出来并交给 Module.swinxAudioPlay。
 *      这里把钩子换成记录用的假实现（node 没有 WebAudio），断言它收到的路径正是
 *      /tmp/theme_cache/def_theme/Sound/effect/go.wav（wasm 的主题落地目录，见 MainDlg
 *      的 __EMSCRIPTEN__ 分支）、字节数与主题里那份 wav 一致、且头
 *      四个字节是 RIFF/8-12 字节是 WAVE —— 证明读的是真文件，不是空缓冲。
 *   5. 输入法与粘贴：CnchessText 的 WM_IME_CHAR 和 SwinxClipboardSetBrowserText +
 *      Ctrl+V 都能把文本落进登录框的编辑框，按精确字符串断言。
 *   6. 插入符：焦点进编辑框后 GetCaretPos 有值，且坐标落在该控件的矩形里——页面
 *      靠它把候选词窗钉到正在输入的那一行。
 *   7. 收包交付跑在后台线程上（与桌面版的 lws 工作线程同口径）
 *      ws 的 wasm 实现给每个连接开一条 ITaskLoop 投递队列：主线程的 JS 回调只做一次
 *      拷贝 + 入队，拆包和 IConnListener 的四个回调都在队列的执行线程上发生。浏览器
 *      工具链还不允许把 socket 对象建在 worker 上（libwebsocket.js 里
 *      createOnMainThread==false 仍是 TODO），所以能搬、也该搬的边界就是回调之后那段。
 *      stat 21 说有几帧交到了监听器手里，stat 22 说其中几帧不在主线程：worker 档两者
 *      相等，泵档 22 恒为 0 —— 与 6/19 那对计数同一个口径。
 *
 * 它不能证明的东西：画面是否好看、棋盘交互是否正确、真人对局流程、滚轮是否真的把
 * 大厅列表滚动（node 里没有可滚动的量：烟测只建 6 张桌，tileview 内容不足一屏，
 * OnMouseWheel 无从移动）、浏览器里输入法候选窗的位置与鼠标的细节、WebAudio 是否
 * 真的出声（这里只证明字节交到了钩子上）、wss/跨域策略。这些仍然要在 Chromium 里
 * 手动过一遍（见 readme.md 的验证清单）。
 *
 * 环境前提：node 没有 DOM，而 canvas2d 后端在建第一条 SConnection（HDC）时就要
 * document.createElement('canvas')，缺它整个模块在启动的第一步就 TypeError —— 与
 * -pthread 无关，泵后端同样。所以脚本自带一份最小假 DOM，口径与
 * demos/wasm-demo/smoke.js 一致（见下面的「假 DOM」一节）。
 *
 * 用法：
 *   source <emsdk>/emsdk_env.sh
 *   bash games/cnchess/client/wasm/build-cnchess.sh            # 默认 worker 后端 -> wasm/out_pt
 *   node games/cnchess/client/wasm/smoke.js                     # 自动挑 out_pt，没有再退 out
 *   node games/cnchess/client/wasm/smoke.js games/cnchess/client/wasm/out/cnchess-wasm.js
 *                                                               # 显式指定产物（--no-pthreads 那份）
 *
 * 不带参数时按 out_pt -> out 的顺序取第一个存在的产物，并打印实际用的是哪份；两条后端
 * 各测一次时才需要显式给路径。烟测本身不要求 crossOriginIsolated —— 那是浏览器里的前提，
 * node 里 SharedArrayBuffer 直接可用，worker 后端能真起线程。
 *
 * 依赖本机的 native 服务器二进制（默认找 build/bin/cnchess_server），因为浏览器版
 * 客户端要连一个真的 cnchess websocket 服务器才有主题包可下。
 *
 * 环境变量：
 *   CNCHESS_SERVER     native 服务器可执行文件路径
 *   CNCHESS_THEME_ZIP  直接用现成的 pc_theme.zip（默认用 zip 命令打包 client/pc_theme）
 *   CNCHESS_KEEP=1     保留临时目录（里面有服务器日志、下发/解压的主题）
 *   CNCHESS_TIMEOUT    等待"主题就绪"的毫秒数，默认 30000
 *
 * 成功退出码 0，任何一条断言不成立退出码 1 并把失败项打到 stderr。
 */
'use strict';

const path = require('path');
const fs = require('fs');
const os = require('os');
const net = require('net');
const { spawn, execFileSync } = require('child_process');

const NodeWebSocket = require('./node-websocket');

/* ------------------------------------------------------------------ 参数与环境 */

/* 产物路径：显式给的参数（或 CNCHESS_JS）优先；否则按 worker 后端 -> 泵后端的顺序取
 * 第一个存在的。build-cnchess.sh 默认产 out_pt，所以不带参数时测的就是默认后端。 */
function pickArtifact() {
    const given = process.argv[2] || process.env.CNCHESS_JS;
    if (given) return path.resolve(given);
    const cands = ['out_pt', 'out', 'release_pt', 'release']
        .map((d) => path.join(__dirname, d, 'cnchess-wasm.js'));
    return cands.find((p) => fs.existsSync(p)) || cands[0];
}

const jsPath = pickArtifact();
if (!fs.existsSync(jsPath)) {
    console.error('没有 ' + jsPath + ' —— 先运行 build-cnchess.sh');
    process.exit(1);
}
const outDir = path.dirname(jsPath);
const repo = path.resolve(__dirname, '../../../..');
console.log('产物：' + jsPath);

const TIMEOUT_MS = Number(process.env.CNCHESS_TIMEOUT || 30000);
const KEEP = process.env.CNCHESS_KEEP === '1';

function findServer() {
    if (process.env.CNCHESS_SERVER) return process.env.CNCHESS_SERVER;
    const cands = [
        path.join(repo, 'build', 'bin', 'cnchess_server'),
        path.join(repo, 'build', 'Release', 'cnchess_server'),
        path.join(repo, 'build_rel', 'bin', 'Release', 'cnchess_server'),
    ];
    return cands.find((p) => fs.existsSync(p));
}

const serverBin = findServer();
if (!serverBin) {
    console.error('找不到本机的 cnchess_server —— 先用本地编译器构建它（games/cnchess/server），' +
        '或用 CNCHESS_SERVER 指定路径');
    process.exit(1);
}

/* ------------------------------------------------------------------ 假 DOM */

/* node 里没有 DOM，而 canvas2d 后端在 SConnection 构造里就建 HDC ——
 * document.createElement('canvas').getContext('2d')，缺 document 会在模块启动的
 * 第一步抛 TypeError（实测：泵后端与 worker 后端都一样，与 -pthread 无关）。这里用
 * 一个最小但够用的假 canvas / 2D context 顶上，与 demos/wasm-demo/smoke.js 同一套
 * 口径：绘制方法全 no-op，只有下面三处必须给真实形状，否则布局与读像素会崩或越界。
 *   - measureText：布局按 fontBoundingBoxAscent/Descent 算行高，返回 0 会让整页内容
 *     逐行上移。
 *   - getImageData / createImageData：回读到 wasm 堆的 .data 长度必须 = w*h*4。
 *   - ctx.canvas：drawImage(src.canvas, ...) 要用，必须回指。
 */
function makeGradient() {
    // 渐变对象只被拿来 addColorStop，画不出真像素不影响断言；返回 undefined 会让
    // canvas2d 后端在下一行 .addColorStop 上抛（皮肤 SVG 里有渐变填充，实测踩过）。
    return { addColorStop() {} };
}

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
        createLinearGradient() { return makeGradient(); },
        createRadialGradient() { return makeGradient(); },
        createConicGradient() { return makeGradient(); },
        createPattern() { return null; },
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
    Object.defineProperty(canvas, '_ctx', { value: makeCtx(canvas), writable: true, configurable: true });
    return canvas;
}

if (typeof ImageData === 'undefined') {
    global.ImageData = class { constructor(data, w, h) { this.data = data; this.width = w; this.height = h; } };
}
if (typeof requestAnimationFrame === 'undefined') {
    // emscripten_set_main_loop 在 node 里靠 rAF/setTimeout 起循环；这里给个 no-op
    // 防止它因 rAF 缺失而抛（循环本就由烟测手动驱动 CnchessPump）。
    global.requestAnimationFrame = () => 0;
}
/* node 也没有 location，而外壳里读到它的地方（?dpr= 之类的查询参数）用的是 eval，
 * 找不到 location 直接 ReferenceError、整个模块加载失败。给一个空查询串的对象，
 * 等价于浏览器地址栏没带参数。host/hostname/protocol 也要有：MainDlg 的默认服务器
 * 地址读 location.host，缺了就拼成 ws://undefined 写进登录框（虽然烟测随后就用
 * CnchessSetText 覆盖了，但断言"默认值跟随页面域名"这条时不能被假值糊过去）。 */
if (typeof location === 'undefined') {
    global.location = {
        search: '', protocol: 'http:', host: '127.0.0.1:8080', hostname: '127.0.0.1',
        pathname: '/cnchess-wasm.html', href: 'file:///cnchess-wasm.html',
    };
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

function freePort() {
    return new Promise((resolve, reject) => {
        const srv = net.createServer();
        srv.unref();
        srv.on('error', reject);
        srv.listen(0, '127.0.0.1', () => {
            const p = srv.address().port;
            srv.close(() => resolve(p));
        });
    });
}

function waitPort(port, deadlineMs) {
    const t0 = Date.now();
    return new Promise((resolve) => {
        const attempt = () => {
            const s = net.connect(port, '127.0.0.1');
            s.on('connect', () => { s.destroy(); resolve(true); });
            s.on('error', () => {
                s.destroy();
                if (Date.now() - t0 > deadlineMs) resolve(false);
                else setTimeout(attempt, 100);
            });
        };
        attempt();
    });
}

/* ------------------------------------------------------------------ 服务器临时目录 */

async function stageServer() {
    const stage = fs.mkdtempSync(path.join(os.tmpdir(), 'cnchess-wasm-smoke-'));
    const port = await freePort();

    fs.copyFileSync(serverBin, path.join(stage, 'cnchess_server'));
    fs.chmodSync(path.join(stage, 'cnchess_server'), 0o755);

    // 服务器从 exe 同级目录读 pc_theme.zip（OS_ID_WINDOWS 那一包）
    const themeZip = path.join(stage, 'pc_theme.zip');
    if (process.env.CNCHESS_THEME_ZIP) {
        fs.copyFileSync(process.env.CNCHESS_THEME_ZIP, themeZip);
    } else {
        try {
            execFileSync('zip', ['-r', '-q', themeZip, '.'],
                { cwd: path.join(repo, 'games', 'cnchess', 'client', 'pc_theme') });
        } catch (e) {
            console.error('打包 pc_theme 失败：' + e.message +
                '\n请安装 zip，或用 CNCHESS_THEME_ZIP 指定现成的 pc_theme.zip');
            process.exit(1);
        }
    }

    // 服务器按 argv[1] / ./config/config.xml 读配置，残局表则固定看 exe 同级 config/
    const cfgDir = path.join(stage, 'config');
    fs.mkdirSync(cfgDir);
    const cfgSrc = path.join(repo, 'games', 'cnchess', 'server', 'config');
    const cfgText = fs.readFileSync(path.join(cfgSrc, 'config.xml'), 'utf8')
        .replace(/port="\d+"/, 'port="' + port + '"');
    fs.writeFileSync(path.join(cfgDir, 'config.xml'), cfgText);
    fs.copyFileSync(path.join(cfgSrc, 'endgames.json'), path.join(cfgDir, 'endgames.json'));

    return { stage, port, themeSize: fs.statSync(themeZip).size };
}

/* ------------------------------------------------------------------ 主流程 */

(async () => {
    const { stage, port, themeSize } = await stageServer();
    // 动态库在同级 build/bin 里；LD_LIBRARY_PATH 只影响这台机器的 native 服务器
    // Linux 入口 main() 在 argc==1 时给 run() 传 NULL，服务器就会退回内置默认端口，
    // 所以配置路径必须显式作为 argv[1] 传进去（Windows 的 _tmain 默认值才指向 config/）。
    const srv = spawn(path.join(stage, 'cnchess_server'), [path.join('config', 'config.xml')], {
        cwd: stage,
        env: { ...process.env, LD_LIBRARY_PATH: path.dirname(serverBin) },
        stdio: ['ignore', 'pipe', 'pipe'],
    });
    let srvLog = '';
    srv.stdout.on('data', (d) => { srvLog += d; });
    srv.stderr.on('data', (d) => { srvLog += d; });
    const stopServer = () => { try { srv.kill('SIGTERM'); } catch (e) { /* 已经退出 */ } };

    process.on('exit', () => {
        stopServer();
        if (!KEEP) fs.rmSync(stage, { recursive: true, force: true });
        else console.log('临时目录保留在 ' + stage);
    });

    if (!await waitPort(port, 20000)) {
        console.error('服务器没能在 20s 内监听 ' + port + '：\n' + srvLog);
        process.exit(1);
    }

    /* WebSocket 探针：emscripten 的 libwebsocket.js 是直接给 socket.onopen/onmessage
     * 赋值的，所以必须挂属性 setter，直接赋值会被它盖掉。send() 走方法覆盖。 */
    const wsLog = { ctor: [], open: 0, close: 0, recv: 0, recvBytes: 0, text: 0, sent: 0 };
    global.WebSocket = class ProbeWebSocket extends NodeWebSocket {
        constructor(url, protocols) {
            super(url, protocols);
            wsLog.ctor.push(url);
            let hOpen = null, hMsg = null, hClose = null;
            Object.defineProperty(this, 'onopen', {
                get: () => hOpen,
                set: (f) => { hOpen = f ? (e) => { wsLog.open++; f(e); } : null; },
            });
            Object.defineProperty(this, 'onmessage', {
                get: () => hMsg,
                set: (f) => {
                    hMsg = f ? (e) => {
                        wsLog.recv++;
                        if (typeof e.data === 'string') wsLog.text++;
                        else wsLog.recvBytes += e.data.byteLength;
                        f(e);
                    } : null;
                },
            });
            Object.defineProperty(this, 'onclose', {
                get: () => hClose,
                set: (f) => { hClose = f ? (e) => { wsLog.close++; f(e); } : null; },
            });
        }
        send(d) {
            wsLog.sent += (typeof d === 'string' ? d.length : d.byteLength);
            return super.send(d);
        }
    };

    /* 音效探针：node 里没有 WebAudio，浏览器页面用 Module.swinxAudioPlay 解码播放，
     * 这里换成一份只记录的假实现，用来取证"sounds.xml 查名 -> 拼前缀 -> swinx 读出
     * wav 字节 -> 交给页面"这条链。装了这个钩子，PlaySound 才可能返回 TRUE；不装就
     * 是音频后端文档里写的"宿主没装钩子就显式返回 FALSE"。 */
    const audioLog = [];
    let fatal = null;
    const createCnchess = require(jsPath);
    const m = await createCnchess({
        noExitRuntime: true,
        locateFile: (f) => path.join(outDir, f),
        print: () => {},
        printErr: (s) => { if (!/readlink|fopen/.test(s)) console.error('[wasm!] ' + s); },
        cnchessFatal: (w) => { fatal = w; console.error('[FATAL] ' + w); },
        swinxAudioPlay: (ptr, len, flags, pathPtr) => {
            if (ptr === 0) { audioLog.push({ stop: true, flags }); return true; }
            // 字节要在本次同步调用里拷走：audio.cpp 返回后就 free 了。
            const base = ptr >>> 0;
            audioLog.push({
                path: heapCString(pathPtr >>> 0),
                len: len >>> 0,
                flags: flags >>> 0,
                head: Array.from(m.HEAPU8.subarray(base, base + 12)),
            });
            return true;
        },
    });

    function heapCString(start) {
        // ALLOW_MEMORY_GROWTH 扩容后视图会失效，所以每次现取。
        const heap = m.HEAPU8;
        let s = '';
        for (let i = start; heap[i] !== 0; ++i) s += String.fromCharCode(heap[i]);
        return s;
    }

    /* CnchessStat 下标，与 cnchess_main.cpp 的表保持一致 */
    const ST = { pump: 0, present: 1, ready: 2, font: 3, theme: 4, boardKids: 5,
                 taskRun: 6, taskPending: 7, taskLive: 8, loginVisible: 9,
                 rootW: 10, rootH: 11, truncated: 12, tick: 13, statusVisible: 14,
                 loginExists: 15, rootKids: 16, dlg: 17,
                 taskWorkers: 18, taskWorkerRun: 19, builtWithPthreads: 20,
                 wsFrames: 21, wsFramesOffUi: 22 };
    const st = (i) => m._CnchessStat(i);
    const cstr = (s) => {
        const bytes = Buffer.from(s + '\0', 'utf8');
        const p = m._malloc(bytes.length);
        m.HEAPU8.set(bytes, p);
        return p;
    };
    // 导出符号少一项这里就是 TypeError，把"二进制没导出"和"导出了但返回值不对"分开
    const NEED = ['_CnchessStat', '_CnchessPump', '_CnchessMouse', '_CnchessWheel', '_CnchessKey',
                  '_CnchessText', '_CnchessCaretPos', '_CnchessPlayEffectSound', '_CnchessClick',
                  '_CnchessChildRect', '_CnchessChildText', '_CnchessChildVisible',
                  '_CnchessChildCount', '_CnchessSetText', '_SwinxClipboardSetBrowserText',
                  '_malloc', '_free'];
    const noExport = NEED.filter((f) => typeof m[f] !== 'function');
    check('导出符号齐全', noExport.length === 0, noExport.join(','));

    const withCstr = (fn, ...args) => {
        const ptrs = args.map((a) => cstr(a));
        try { return fn(...ptrs); } finally { ptrs.forEach((p) => m._free(p)); }
    };
    const setText = (name, value) => {
        const pn = cstr(name), pv = cstr(value);
        const r = m._CnchessSetText(pn, pv);
        m._free(pn); m._free(pv);
        return r;
    };
    const pump = (n) => { for (let i = 0; i < (n || 1); ++i) m._CnchessPump(); };
    /* 弹窗的开合走 SOUI 动画，动画只由 kPulseTimer 的 WM_TIMER 推进，而 WM_TIMER 按**真实
     * 时间**到期：连续空泵 CnchessPump 一帧动画都推不动（实测 32 次泵只花 1ms，插入符/
     * 命中测试都没落点）。所以要等动画收敛就必须在两次泵之间让出事件循环。到上限仍不满足
     * 条件就返回 false，由调用方的断言把失败显式化，不在这里静默兜住。 */
    const settle = async (pred, maxMs) => {
        const t0 = Date.now();
        for (;;) {
            if (pred()) return true;
            if (Date.now() - t0 > (maxMs || 1500)) return false;
            pump(1);
            await sleep(20);
        }
    };
    const WM_KEYDOWN = 0x0100, WM_KEYUP = 0x0101;
    const VK_CONTROL = 0x11, VK_V = 0x56;
    const key = (msg, vk) => { m._CnchessKey(msg, vk, 0); pump(1); };
    const childText = (name) => {
        const cap = 256;
        const pn = cstr(name), pb = m._malloc(cap);
        try {
            const n = m._CnchessChildText(pn, pb, cap);
            return Buffer.from(m.HEAPU8.slice(pb, pb + n)).toString('utf8');
        } finally { m._free(pn); m._free(pb); }
    };
    const childRect = (name) => {
        const pn = cstr(name), pr = m._malloc(16);
        try {
            if (!m._CnchessChildRect(pn, pr)) return null;
            const v = (i) => m.HEAP32[(pr >> 2) + i];
            return [v(0), v(1), v(2), v(3)];
        } finally { m._free(pn); m._free(pr); }
    };
    const caretPos = () => {
        const p = m._malloc(8);
        try {
            if (!m._CnchessCaretPos(p)) return null;
            return [m.HEAP32[p >> 2], m.HEAP32[(p >> 2) + 1]];
        } finally { m._free(p); }
    };

    pump(5);

    /* ---- 1. 启动：宿主窗口 + 系统资源 + 登录弹窗 ---- */
    check('宿主窗口就绪', st(ST.ready) === 1, 'ready=' + st(ST.ready));
    check('字体可用(canvas2d浏览器字体)', st(ST.font) === 1);
    check('DoConfig 成功（uires/系统资源都加载）', st(ST.dlg) === 1 && st(ST.rootKids) >= 3,
        'dlg=' + st(ST.dlg) + ' rootKids=' + st(ST.rootKids));
    check('登录弹窗可见', st(ST.loginVisible) === 1 && st(ST.loginExists) === 1);
    check('棋盘页来自桌面布局（只有 btn_test 一个子节点）', st(ST.boardKids) === 1,
        'boardKids=' + st(ST.boardKids));
    check('登录前没有发起任何 WebSocket', wsLog.ctor.length === 0, 'ctor=' + wsLog.ctor.length);
    check('窗口尺寸与帧缓冲一致', st(ST.rootW) === m._SwinxFbWidth() && st(ST.rootH) === m._SwinxFbHeight(),
        st(ST.rootW) + 'x' + st(ST.rootH));

    /* ---- 1b. 输入：插入符 / 输入法 / 粘贴（登录弹窗还在的时候做）---- */
    // 这三条对应页面上的三处接线：候选词窗跟随（syncProxyPos 读 CnchessCaretPos）、
    // compositionend 提交（CnchessText -> WM_IME_CHAR）、paste 桥
    // （SwinxClipboardSetBrowserText + Ctrl+V）。断言都落在编辑框的最终文本上，
    // 不是"函数调用了"，所以接线断了这里就会红。
    check('点昵称框后插入符有坐标', (() => {
        withCstr(m._CnchessClick, 'edt_name');
        pump(2);
        return caretPos() !== null;
    })(), 'caret=' + JSON.stringify(caretPos()));
    const nameRect = childRect('edt_name');
    const caret = caretPos();
    check('插入符落在昵称框矩形内（候选窗才会跟到这一行）',
        !!(nameRect && caret && caret[0] >= nameRect[0] && caret[0] <= nameRect[2] &&
           caret[1] >= nameRect[1] - 2 && caret[1] <= nameRect[3]),
        'caret=' + JSON.stringify(caret) + ' rect=' + JSON.stringify(nameRect));

    setText('edt_name', '');
    pump(1);
    withCstr(m._CnchessText, '王一键');
    pump(2);
    check('输入法提交的中文进登录框', childText('edt_name') === '王一键',
        'got=' + childText('edt_name'));

    setText('edt_name', '');
    pump(1);
    const pClip = cstr('粘贴Paste');
    m._SwinxClipboardSetBrowserText(pClip);
    m._free(pClip);
    key(WM_KEYDOWN, VK_CONTROL);
    key(WM_KEYDOWN, VK_V);
    key(WM_KEYUP, VK_V);
    key(WM_KEYUP, VK_CONTROL);
    pump(2);
    check('Ctrl+V 把浏览器剪贴板文本粘进登录框', childText('edt_name') === '粘贴Paste',
        'got=' + childText('edt_name'));

    /* ---- 2. 登录：走 SModalRoot 的登录弹窗，不产生嵌套消息循环 ---- */
    setText('edt_svr', '127.0.0.1:' + port);
    setText('edt_name', 'wasm-smoke');
    check('点击登录按钮', m._CnchessClick(cstr('btn_login')) === 1);
    pump(2);

    /* ---- 3. 等主题包下载 / 解压 / 加载，期间记录 TaskLoop 的活动 ---- */
    const expectUrl = 'ws://127.0.0.1:' + port + '/cnchess/?';
    let maxTaskLive = 0, maxTaskPending = 0, maxTaskWorkers = 0;
    const t0 = Date.now();
    while (Date.now() - t0 < TIMEOUT_MS) {
        await sleep(50);
        pump(2);
        maxTaskLive = Math.max(maxTaskLive, st(ST.taskLive));
        maxTaskPending = Math.max(maxTaskPending, st(ST.taskPending));
        maxTaskWorkers = Math.max(maxTaskWorkers, st(ST.taskWorkers));
        if (st(ST.theme) === 1) break;
    }
    const elapsed = Date.now() - t0;
    pump(10);

    /* ---- 4. 网络：用的是浏览器原生 WebSocket ---- */
    check('只建了一条 WebSocket 连接', wsLog.ctor.length === 1, 'ctor=' + wsLog.ctor.length);
    check('连接 URL 由 host:port + /cnchess/ 拼出', wsLog.ctor[0] === expectUrl,
        'got ' + wsLog.ctor[0] + ' want ' + expectUrl);
    check('连接已建立', wsLog.open === 1 && wsLog.close === 0,
        'open=' + wsLog.open + ' close=' + wsLog.close);
    check('协议是二进制帧（没有文本帧）', wsLog.text === 0 && wsLog.recv > 0,
        'text=' + wsLog.text + ' recv=' + wsLog.recv);
    // 服务器发的是主题包 + 房间信息 + 残局列表，总量不小于主题包本身
    check('收到的字节数覆盖整个主题包', wsLog.recvBytes >= themeSize,
        wsLog.recvBytes + ' >= ' + themeSize);
    check('客户端发过请求（登录）', wsLog.sent > 0, 'sent=' + wsLog.sent);

    /* ---- 5. 主题：下载 -> 落盘 -> TaskLoop 解压 -> 加载 ---- */
    check('主题加载完成', st(ST.theme) === 1, elapsed + 'ms');
    check('InitGame 把棋盘插满了', st(ST.boardKids) >= 93, 'boardKids=' + st(ST.boardKids));
    check('大厅列表收到房间信息', withCstr(m._CnchessChildCount, 'room_container') > 0,
        'roomKids=' + withCstr(m._CnchessChildCount, 'room_container'));
    check('残局列表收到服务器数据', withCstr(m._CnchessChildCount, 'endgame_container') > 0,
        'endgameKids=' + withCstr(m._CnchessChildCount, 'endgame_container'));
    // 关闭弹窗走退场动画，要泵 + 让出事件循环等它收敛（见 settle 上的说明）。
    check('进度弹窗已关闭', await settle(() => st(ST.statusVisible) === 0),
        'statusVisible=' + st(ST.statusVisible));
    check('登录弹窗已销毁', await settle(() => st(ST.loginVisible) === 0 && st(ST.loginExists) === 0),
        'loginVisible=' + st(ST.loginVisible) + ' loginExists=' + st(ST.loginExists));

    /* ---- 6. TaskLoop：解压跑在哪条后端上，队列跑空、实例释放 ---- */
    const nRun = st(ST.taskRun), nPending = st(ST.taskPending), nLive = st(ST.taskLive);
    const nWorkers = st(ST.taskWorkers), nWorkerRun = st(ST.taskWorkerRun);
    const nPt = st(ST.builtWithPthreads);
    console.log('  [taskloop] pt=' + nPt + ' run=' + nRun + ' offUi=' + nWorkerRun +
        ' workers=' + nWorkers + ' workersPeak=' + maxTaskWorkers +
        ' pending=' + nPending + ' live=' + nLive);
    check('TaskLoop 执行过任务', nRun >= 1, 'run=' + nRun);
    check('TaskLoop 队列已跑空', nPending === 0, 'pending=' + nPending);
    /* 实例数不再归零：每个连接都长驻一条投递队列（与桌面版每连接一条 lws 工作线程
     * 同形态），烟测全程只有一条连接，所以收尾时应当恰好剩它一条。这条断言真正的
     * 对象是解压那条临时队列 —— 它漏了就会是 2。 */
    check('TaskLoop 实例：只剩收包投递那一条（解压队列已释放）', nLive === 1, 'live=' + nLive);
    check('构建能力探针可读（stat 20 = 0 或 1）', nPt === 0 || nPt === 1, 'pt=' + nPt);
    if (nPt === 1) {
        /* 带 -pthread 的产物。node 里 SharedArrayBuffer 直接可用（不需要 COOP/COEP，那
         * 是浏览器侧的前提），所以这里 worker 必须真的起来并把任务全部吃掉：offUi==run
         * 才是"整条解压离开了 UI 线程"。这份产物在浏览器里缺隔离头时 workers 会是 0 ——
         * 那条降级路径由页面状态行人工过（readme 清单）。 */
        check('worker 后端：起过 worker 线程', maxTaskWorkers >= 1, 'workersPeak=' + maxTaskWorkers);
        check('worker 后端：解压任务全部跑在非 UI 线程上', nWorkerRun === nRun && nRun >= 1,
            'offUi=' + nWorkerRun + '/' + nRun);
        /* 泵后端把整段解压塞进一次 CnchessPump，50ms 的轮询永远落在它前后，采不到"活的
         * loop"；worker 后端跨帧，所以采得到。这是上面两条之外的第三方信号。 */
        check('worker 后端：解压跨帧（轮询采到活的实例）', maxTaskLive >= 1,
            'livePeak=' + maxTaskLive);
    } else {
        check('泵后端：没起 worker，任务全在 UI 线程上执行', nWorkers === 0 && nWorkerRun === 0,
            'workers=' + nWorkers + ' offUi=' + nWorkerRun);
    }
    console.log('（观测）采样到的最大 live/pending/workers: ' +
        maxTaskLive + '/' + maxTaskPending + '/' + maxTaskWorkers);

    /* ---- 6c. 收包交付：帧交到 IConnListener 手里时在哪条线程上 ----
     * ws 的 wasm 实现把边界放在 JS 回调之后：主线程只拷一次 + 入队，拆包和回调
     * 由这条连接自己的投递队列做。下面先证明"每一帧都真的交出去了"（和 node 侧
     * 收到的帧数对齐，漏一帧就对不上），再按后端证明"交出去时在哪条线程"。 */
    const nWsFrames = st(ST.wsFrames), nWsFramesOffUi = st(ST.wsFramesOffUi);
    console.log('  [ws-delivery] frames=' + nWsFrames + ' offUi=' + nWsFramesOffUi +
        ' jsRecv=' + wsLog.recv);
    check('收到的每一帧都交给了监听器', nWsFrames >= 1 && nWsFrames === wsLog.recv,
        'frames=' + nWsFrames + ' jsRecv=' + wsLog.recv);
    if (nPt === 1) {
        check('worker 后端：交付全部发生在非主线程', nWsFramesOffUi === nWsFrames,
            'offUi=' + nWsFramesOffUi + '/' + nWsFrames);
    } else {
        check('泵后端：交付全部发生在主线程', nWsFramesOffUi === 0,
            'offUi=' + nWsFramesOffUi + '/' + nWsFrames);
    }

    /* ---- 6b. 音效：主题包里的 wav 能被 swinx 读出来交给页面 ---- */
    // CChessGame::PlayEffectSound 的三步（sounds.xml 查名 -> 拼文件前缀 -> SPlaySound）
    // 由 CnchessPlayEffectSound 走同一条路，但要等到落子才触发的部分留给浏览器人工验。
    // 返回值口径见 cnchess_main.cpp：2=PlaySound TRUE，3=FALSE，1=名字不在 sounds.xml。
    const goWav = path.join(repo, 'games', 'cnchess', 'client', 'pc_theme', 'Sound', 'effect', 'go.wav');
    // wasm 的主题落地目录：MainDlg 的 __EMSCRIPTEN__ 分支把缓存目录定在 /tmp/theme_cache
    //（-pthread 产物必须开 -sWASMFS=1，而 WASMFS 里 --preload-file 铺出的 /opt/soui
    //  整棵树只读，建不了目录），所以交给页面的绝对路径是这个前缀开头。
    const wantSoundPath = '/tmp/theme_cache/def_theme/Sound/effect/go.wav';
    // 默认用 zip 现打包（包的就是仓库里那份 pc_theme），此时可以按真实文件长度断言；
    // 用 CNCHESS_THEME_ZIP 换成别的包时长度未知，只断言魔数与非空。
    const wantLen = process.env.CNCHESS_THEME_ZIP ? 0 : fs.statSync(goWav).size;
    const audioBefore = audioLog.length;
    const rcSound = withCstr(m._CnchessPlayEffectSound, '走子');
    pump(1);
    const rec = audioLog[audioBefore];
    check('走子音效走通到页面钩子（返回 2）', rcSound === 2,
        'rc=' + rcSound + '（3=页面没装 swinxAudioPlay 钩子或文件读不到）');
    check('音效名解析成主题目录里的 wav 绝对路径', !!rec && rec.path === wantSoundPath,
        rec ? rec.path + ' want ' + wantSoundPath : '没有调用页面钩子');
    check('交给页面的是 RIFF/WAVE 头', !!rec &&
        rec.head.slice(0, 4).join() === Buffer.from('RIFF').join() &&
        rec.head.slice(8, 12).join() === Buffer.from('WAVE').join(),
        rec ? rec.head.join(',') : '-');
    if (wantLen)
        check('字节数与主题里那份 go.wav 一致', !!rec && rec.len === wantLen,
            rec ? rec.len + ' want ' + wantLen : '-');
    else
        check('读到了非空的 wav 字节', !!rec && rec.len > 44, rec ? String(rec.len) : '-');
    // CMainDlg::PlayWave 传的是 SND_ASYNC|SND_NOSTOP|SND_FILENAME = 0x20011；flags 错了
    // 页面侧的 SND_NOSTOP 判重与循环就不对。
    check('flags 原样带到页面（SND_ASYNC|SND_NOSTOP|SND_FILENAME）', !!rec && rec.flags === 0x20011,
        rec ? '0x' + rec.flags.toString(16) : '-');
    check('名字不在 sounds.xml 里时不发声', withCstr(m._CnchessPlayEffectSound, '没有这个音效') === 1);
    check('音效请求没有多出别的调用', audioLog.length === audioBefore + 1,
        'delta=' + (audioLog.length - audioBefore));

    /* ---- 7. 帧循环 ---- */
    check('主循环在出画', st(ST.present) > 0, 'presents=' + st(ST.present));
    check('每帧都在预算内返回', st(ST.truncated) === 0);
    check('没有 fatal 回调', fatal === null, fatal || '');

    stopServer();
    console.log(failures.length
        ? '\n' + failures.length + ' 项失败：' + failures.join(' / ')
        : '\n全部通过');
    process.exit(failures.length ? 1 : 0);
})().catch((e) => {
    console.error(e);
    process.exit(1);
});
